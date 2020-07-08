#include "NetworkManager2.h"
#include <zlib/zlib.h>
#include "../Utilities/Utils.h"

namespace Minecraft::Server {
	NetworkManager::NetworkManager(ServerSocket* socket)
	{
		m_Socket = socket;
	}
	void NetworkManager::AddPacket(PacketOut* p)
	{
		packetQueue.push(p);
	}
	void NetworkManager::ClearPacketQueue()
	{
		utilityPrint("Clearing Packet Queue", Utilities::LOGGER_LEVEL_DEBUG);
		for (int i = 0; i < packetQueue.size(); i++) {
			delete packetQueue.front();
			packetQueue.pop();
		}
	}
	void NetworkManager::SendPackets()
	{
		Utilities::detail::core_Logger->log("Sending Network Packet Queue");


		int len = packetQueue.size();
		for (int i = 0; i < len; i++) {
			uint64_t packetLength;

			packetLength = packetQueue.front()->buffer->GetUsedSpace() + 1;
			
			ByteBuffer* bbuf = new ByteBuffer(packetLength + 5); //512 KB

			if (compression) {

				if (packetLength > 256) { //If length > threshold
					bbuf->WriteVarInt32(packetLength);


					ByteBuffer* bbuf2 = new ByteBuffer(packetLength + 5);
					bbuf2->WriteBEUInt8(packetQueue.front()->ID);
					for (int i = 0; i < packetQueue.front()->buffer->GetUsedSpace(); i++) {
						uint8_t temp;
						packetQueue.front()->buffer->ReadBEUInt8(temp);
						bbuf2->WriteBEUInt8(temp);
					}

					//COMPRESS DATA!
					char byteBuffer[200 KiB];

					uLongf csize = compressBound(packetLength);
					if (csize > 200 KiB) {
						Utilities::detail::core_Logger->log("FATAL! COMPRESSED SIZE TOO LARGE", LOGGER_LEVEL_ERROR);
						throw std::runtime_error("FATAL! COMPRESSED SIZE TOO LARGE: " + std::to_string(csize));
					}

					int32_t stat = compress2((Bytef*)byteBuffer, &csize, (const Bytef*)bbuf2->m_Buffer, bbuf2->GetUsedSpace(), Z_DEFAULT_COMPRESSION);
					delete bbuf2;

					if (stat != Z_OK) {
						Utilities::detail::core_Logger->log("FATAL! FAILED TO COMPRESS", LOGGER_LEVEL_ERROR);
						throw std::runtime_error("FAILED TO COMPRESS: " + std::to_string(stat));
					}

					for (int i = 0; i < csize; i++) {
						bbuf->WriteBEUInt8(byteBuffer[i]);
					}

				}
				else {
					bbuf->WriteVarInt32(0);
					bbuf->WriteBEUInt8(packetQueue.front()->ID);

					packetQueue.front()->buffer->ResetRead();
					//Add body
					for (int i = 0; i < packetQueue.front()->buffer->GetUsedSpace(); i++) {
						uint8_t temp;
						packetQueue.front()->buffer->ReadBEUInt8(temp);
						bbuf->WriteBEUInt8(temp);
					}
				}

				ByteBuffer* bbuf2 = new ByteBuffer(bbuf->GetUsedSpace() + 5);
				bbuf2->WriteVarInt32(bbuf->GetUsedSpace());

				for (int i = 0; i < bbuf->GetUsedSpace(); i++) {
					uint8_t temp;
					bbuf->ReadBEUInt8(temp);
					bbuf2->WriteBEUInt8(temp);
				}

				Utilities::detail::core_Logger->log("Sending packet with ID: " + std::to_string(packetQueue.front()->ID), Utilities::LOGGER_LEVEL_DEBUG);
				//Send over socket
#if CURRENT_PLATFORM == PLATFORM_PSP
				sceKernelDcacheWritebackInvalidateAll();
#endif
				m_Socket->Send(bbuf2->GetUsedSpace(), bbuf2->m_Buffer);

				delete bbuf2;
			}
			else {
				//Header
				bbuf->WriteVarInt32(packetLength);

				bbuf->WriteBEUInt8(packetQueue.front()->ID);

				packetQueue.front()->buffer->ResetRead();
				//Add body
				for (int i = 0; i < packetQueue.front()->buffer->GetUsedSpace(); i++) {
					uint8_t temp;
					packetQueue.front()->buffer->ReadBEUInt8(temp);
					bbuf->WriteBEUInt8(temp);
				}

				Utilities::detail::core_Logger->log("Sending packet with ID: " + std::to_string(packetQueue.front()->ID), Utilities::LOGGER_LEVEL_DEBUG);
				//Send over socket
#if CURRENT_PLATFORM == PLATFORM_PSP
				sceKernelDcacheWritebackInvalidateAll();
#endif
				m_Socket->Send(bbuf->GetUsedSpace(), bbuf->m_Buffer);
			}

			delete bbuf;
			delete packetQueue.front()->buffer;
			delete packetQueue.front();
			packetQueue.pop();
		}
	}
	bool NetworkManager::ReceivePacket()
	{
		PacketIn* p = m_Socket->Recv(false);

		if (p != NULL && compression) {
			//Correct this - packet ID is currently 0, and is next on read iterator
			uint8_t r = 0;
			p->buffer->ReadBEUInt8(r);
			p->ID = r;
		}

		if (p != NULL) {
			unhandledPackets.push(p);
		}


		return (p != NULL);
	}
	void NetworkManager::HandlePackets()
	{

		int len = unhandledPackets.size();
		for (int i = 0; i < len; i++) {
			PacketIn* p = unhandledPackets.front();

			if (packetHandlers.find(p->ID) != packetHandlers.end()) {
				packetHandlers[p->ID](p);
			
				delete p->buffer;
				delete p;
				unhandledPackets.pop();
			}
		}
	}
	void NetworkManager::AddPacketHandler(int id, PacketHandler h)
	{
		packetHandlers.emplace(id, h);
	}
	void NetworkManager::ClearPacketHandlers()
	{
		packetHandlers.clear();
	}

	void NetworkManager::setConnectionStatus(uint8_t status)
	{
		utilityPrint("Setting Connection Status to: " + std::to_string((int)status), LOGGER_LEVEL_INFO);
		connectionStatus = status;

		g_NetMan->ClearPacketHandlers();

		switch (connectionStatus) {
		case CONNECTION_STATE_HANDSHAKE: {
			g_NetMan->compression = false;
			g_NetMan->AddPacketHandler(Protocol::Handshake::HANDSHAKE, Protocol::Handshake::handshake_packet_handler);
			break;
		}

		case CONNECTION_STATE_STATUS: {
			g_NetMan->AddPacketHandler(Protocol::Status::REQUEST, Protocol::Status::request_packet_handler);
			g_NetMan->AddPacketHandler(Protocol::Status::PING, Protocol::Status::ping_packet_handler);
			break;
		}

		case CONNECTION_STATE_LOGIN: {
			g_NetMan->AddPacketHandler(Protocol::Login::LOGIN_START, Protocol::Login::login_start_packet_handler);
			break;
		}

		case CONNECTION_STATE_PLAY: {
			g_NetMan->AddPacketHandler(Protocol::Play::TELEPORT_CONFIRM, Protocol::Play::teleport_confirm_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::QUERY_BLOCK_NBT, Protocol::Play::query_block_nbt_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CHAT_MESSAGE, Protocol::Play::chat_message_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLIENT_STATUS, Protocol::Play::client_status_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLIENT_SETTINGS, Protocol::Play::client_settings_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::TAB_COMPLETE, Protocol::Play::tab_complete_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CONFIRM_TRANSACTION, Protocol::Play::confirm_transaction_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ENCHANT_ITEM, Protocol::Play::enchant_item_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLICK_WINDOW, Protocol::Play::click_window_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CLOSE_WINDOW, Protocol::Play::close_window_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLUGIN_MESSAGE, Protocol::Play::plugin_message_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::EDIT_BOOK, Protocol::Play::edit_book_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::QUERY_ENTITY_NBT, Protocol::Play::query_entity_nbt_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::USE_ENTITY, Protocol::Play::use_entity_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::KEEP_ALIVE, Protocol::Play::keep_alive_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER, Protocol::Play::player_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_POSITION, Protocol::Play::player_position_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_POSITION_AND_LOOK, Protocol::Play::player_position_and_look_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_LOOK, Protocol::Play::player_look_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::VEHICLE_MOVE, Protocol::Play::vehicle_move_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::STEER_BOAT, Protocol::Play::steer_boat_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PICK_ITEM, Protocol::Play::pick_item_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CRAFT_RECIPE_REQUEST, Protocol::Play::craft_recipe_request_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_ABILITIES, Protocol::Play::player_abilities_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_DIGGING, Protocol::Play::player_digging_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ENTITY_ACTION, Protocol::Play::entity_action_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::STEER_VEHICLE, Protocol::Play::steer_vehicle_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::RECIPE_BOOK_DATA, Protocol::Play::recipe_book_data_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::NAME_ITEM, Protocol::Play::name_item_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::RESOURCE_PACK_STATUS, Protocol::Play::resource_pack_status_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ADVANCEMENT_TAB, Protocol::Play::advancement_tab_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::SELECT_TRADE, Protocol::Play::select_trade_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::SET_BEACON_EFFECT, Protocol::Play::set_beacon_effect_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::HELD_ITEM_CHANGE, Protocol::Play::held_item_change_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::UPDATE_COMMAND_BLOCK, Protocol::Play::update_command_block_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::UPDATE_COMMAND_BLOCK_MINECART, Protocol::Play::update_command_block_minecart_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::CREATIVE_INVENTORY_ACTION, Protocol::Play::creative_inventory_action_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::UPDATE_STRUCTURE_BLOCK, Protocol::Play::update_structure_block_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::UPDATE_SIGN, Protocol::Play::update_sign_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::ANIMATION, Protocol::Play::animation_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::SPECTATE, Protocol::Play::spectate_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::PLAYER_BLOCK_PLACEMENT, Protocol::Play::player_block_placement_handler);
			g_NetMan->AddPacketHandler(Protocol::Play::USE_ITEM, Protocol::Play::use_item_handler);
			break;
		}
		}
	}

	NetworkManager* g_NetMan = nullptr;
}