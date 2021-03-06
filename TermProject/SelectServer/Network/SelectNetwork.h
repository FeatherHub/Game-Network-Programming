#pragma once

#pragma comment(lib, "ws2_32.lib")

#define FD_SETSIZE 64
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <queue>

#include "Client.h"
#include "NetPacket.h"

class BodySizeManager;

namespace NNetworkLib
{
	enum NETCODE : int;

	class Client;

	//Subject : Server
	//Transport : TCP
	//Socket I/O Model : Select

	class SelectNetwork
	{
	public:
		SelectNetwork() = default;
		~SelectNetwork();
		bool Init(unsigned short port, const char* ip);
		bool Run();

		NETCODE SendPacket(int receiverId, unsigned short pktId, char* dataPos);
		RecvPacket GetPacket();
		bool PacketQueueEmpty();

		void ForceCloseClient(int clientIdx);
		std::queue<int>& GetClosedClients() { return m_clientToClosePool; }
	private:
		//INIT//
		void InitClientStuff();

		//RUN//
		bool ProcessSelect();

		NETCODE ProcessAccept();
		void AddClient(SOCKET s, SOCKADDR_IN& addr);

		NETCODE ProcessClient();
		NETCODE Recv(int id);
		void RecvBuffProc1(int id);
		void RecvBuffProc2(int clientId);
		NETCODE Send(int id);
		bool SendBuffProc(int id);

		void AddToRecvPktQueue(RecvPacket packet);

		void CloseClient(int id);

		void DisplayErrorMsg(const char* msg);
	private:
		Client m_clientPool[FD_SETSIZE];
		std::queue<int> m_clientIndexPool;
		std::queue<int> m_clientToClosePool;
		int m_clientNum = 0;

		fd_set m_fds;
		fd_set m_readFds;
		fd_set m_writeFds;

		std::queue<RecvPacket> m_recvPktQueue;

		SOCKET m_listenSock;

		BodySizeManager* m_bodySizeMgr;
	private:
		enum 
		{ 
			MAX_ACCEPT_AT_ONCE = 30,
			MAX_CLIENT_NUM = FD_SETSIZE
		};
	};
}