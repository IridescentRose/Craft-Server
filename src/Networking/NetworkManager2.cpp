#include "NetworkManager2.h"
#include <zlib/zlib.h>

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
		PacketIn* p = m_Socket->Recv();
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

	NetworkManager* g_NetMan = nullptr;
}