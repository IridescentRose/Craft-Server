#include "NetworkManager2.h"

#include "NetworkManager2.h"

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
		std::vector<byte> endByteBuffer;
		int len = packetQueue.size();
		for (int i = 0; i < len; i++) {
			endByteBuffer.clear();

			int packetLength = packetQueue.front()->bytes.size() + 1;

			//Header
			encodeVarInt(packetLength, endByteBuffer);
			encodeByte(packetQueue.front()->ID, endByteBuffer);

			//Add body
			for (int x = 0; x < packetQueue.front()->bytes.size(); x++) {
				endByteBuffer.push_back(packetQueue.front()->bytes[x]);
			}

			//Send over socket
			m_Socket->Send(endByteBuffer.size(), endByteBuffer.data());

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