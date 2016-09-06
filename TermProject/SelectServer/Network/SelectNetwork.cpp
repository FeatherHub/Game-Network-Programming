#include "SelectNetwork.h"
#include "NetCode.h"

#include "..\..\Common\Util\BodySizeMananger.h"
#include "..\..\Common\Util\Logger.h"

namespace NNetworkLib
{
	bool SelectNetwork::Init(unsigned short port, const char* ip)
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			return false;

		m_listenSock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_listenSock == INVALID_SOCKET)
			return false;

		SOCKADDR_IN serverAddr;
		ZeroMemory(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		inet_pton(AF_INET, ip, (void*)serverAddr.sin_addr.s_addr);

		if (bind(m_listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			return false;

		unsigned long mode = 1;
		if (ioctlsocket(m_listenSock, FIONBIO, &mode) == SOCKET_ERROR)
			return false;

		InitClientStuff();

		if (listen(m_listenSock, SOMAXCONN) == SOCKET_ERROR)
			return false;

		FD_ZERO(&m_fds);
		FD_SET(m_listenSock, &m_fds);

		m_bodySizeMgr = new BodySizeManager();

		Logger::Write(Logger::INFO, "Network Init Success");

		return true;
	}

	bool SelectNetwork::Run()
	{
		auto res = ProcessSelect();
		if (res == false)
		{
			DisplayErrorMsg("select");
		}

		ProcessAccept();

		ProcessClient();

		return true;
	}

	bool SelectNetwork::ProcessSelect()
	{
		FD_ZERO(&m_readFds);
		m_readFds = m_fds;

		FD_ZERO(&m_writeFds);
		for (int id = 0; id < MAX_CLIENT_NUM; id++)
		{
			if (m_clientPool[id].IsConnected() == false)
			{
				continue;
			}

			if (m_clientPool[id].sendSize > 0)
			{
				FD_SET(m_clientPool[id].s, &m_writeFds);
			}
		}

		auto res = select(0, &m_readFds, &m_writeFds, nullptr, nullptr);

		if (res == SOCKET_ERROR)
		{
			return false;
		}

		return true;
	}

	NETCODE SelectNetwork::ProcessAccept()
	{
		if (m_clientNum >= FD_SETSIZE)
		{
			Logger::Write(Logger::WARN, "Client pool is full. Cannot accept.");
			return NETCODE::INFO_CLIENT_POOL_FULL;
		}

		if (FD_ISSET(m_listenSock, &m_readFds))
		{
			SOCKADDR_IN clientAddr;
			int addrLen = sizeof(clientAddr);
			SOCKET clientSock;
			int acceptCnt = 0;

			do
			{
				clientSock = accept(m_listenSock, (SOCKADDR*)&clientAddr, &addrLen);
				if (clientSock == INVALID_SOCKET)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						return NETCODE::INFO_ACCEPT_NONBLOCKED;
					}

					return NETCODE::ERROR_ACCEPT_SOCKET;
				}

				AddClient(clientSock, clientAddr);
			} while (++acceptCnt < MAX_ACCEPT_AT_ONCE &&
					m_clientNum < FD_SETSIZE);
		}

		return NETCODE::NONE;
	}

	NETCODE SelectNetwork::ProcessClient()
	{
		for (int id = 0; id < FD_SETSIZE; id++)
		{
			Client& client = m_clientPool[id];

			if (client.IsConnected())
			{
				if (FD_ISSET(client.s, &m_readFds))
				{
					RecvBuffProc1(id);

					auto res = Recv(id);
					if (res == NETCODE::INFO_CLIENT_LEFT ||
						res == NETCODE::ERROR_RECV_SOCKET)
					{
						CloseClient(id);

						continue;
					}

					RecvBuffProc2(id);
				}

				if (FD_ISSET(client.s, &m_writeFds))
				{
					auto res = Send(id);
					if (res == NETCODE::ERROR_SEND_SOCKET)
					{
						CloseClient(id);

						continue;
					}

					SendBuffProc(id);
				}
			}
		}

		return NETCODE::NONE;
	}

	NETCODE SelectNetwork::Recv(int id)
	{
		Client& c = m_clientPool[id];

		int res = recv(c.s, c.recvBuff + c.recvSize,
			Client::MAX_BUFF_SIZE - c.recvSize, 0);

		if (res == 0)
		{
			return NETCODE::INFO_CLIENT_LEFT;
		}
		else if (res < 0)
		{
			return NETCODE::ERROR_RECV_SOCKET;
		}
		else
		{
			c.recvSize += res;
		}

		return NETCODE::NONE;
	}

	void SelectNetwork::RecvBuffProc1(int id)
	{
		Client& c = m_clientPool[id];

		CopyMemory(c.recvBuff, c.recvBuff + c.readPos, c.recvSize);
		c.readPos = 0;
	}

	void SelectNetwork::RecvBuffProc2(int clientId)
	{
		Client& c = m_clientPool[clientId];

		while (true)
		{
			if (c.recvSize < PACKET_HEADER_SIZE)
			{
				break;
			}

			PktHeader* pktH = (PktHeader*)(c.recvBuff + c.readPos);

			int bodySize = m_bodySizeMgr->Get(pktH->id);

			if (c.recvSize < PACKET_HEADER_SIZE + bodySize)
			{
				break;
			}
			
			char* dataPos = c.recvBuff + c.readPos + PACKET_HEADER_SIZE;

			AddToRecvPktQueue(RecvPacket{ pktH->id, dataPos, clientId });

			c.readPos += PACKET_HEADER_SIZE + bodySize;
			c.recvSize -= PACKET_HEADER_SIZE + bodySize;
		}
	}

	NETCODE SelectNetwork::Send(int id)
	{
		Client& c = m_clientPool[id];

		int sentSize = 0;
		sentSize = send(c.s, c.sendBuff, c.sendSize, 0);
		if (sentSize < 0)
		{
			return NETCODE::ERROR_SEND_SOCKET;
		}
		else
		{
			c.sentSize = sentSize;
		}

		Logger::Write(Logger::INFO, "Network send");

		return NETCODE::NONE;
	}

	bool SelectNetwork::SendBuffProc(int id)
	{
		Client& c = m_clientPool[id];

		CopyMemory(c.sendBuff, c.sendBuff + c.sentSize, c.sendSize - c.sentSize);

		c.sendSize -= c.sentSize;
		c.sentSize = 0;

		return true;
	}

	RecvPacket SelectNetwork::GetPacket()
	{
		RecvPacket pkt = m_recvPktQueue.front();
		m_recvPktQueue.pop();

		return pkt;
	}

	void SelectNetwork::AddToRecvPktQueue(RecvPacket packet)
	{
		m_recvPktQueue.push(packet);
	}

	NNetworkLib::NETCODE SelectNetwork::SendPacket(int receiverId, unsigned short pktId, char* dataPos)
	{
		Client& c = m_clientPool[receiverId];
		int bodySize = m_bodySizeMgr->Get(pktId);

		if (c.sendSize + PACKET_HEADER_SIZE + bodySize > Client::MAX_BUFF_SIZE)
		{
			Logger::Write(Logger::WARN, "Send Buffer Full");
			return NETCODE::ERROR_SENDBUFFER_FULL;
		}

		PktHeader pktHeader{pktId};

		//header
		CopyMemory(c.sendBuff + c.sendSize, &pktHeader, PACKET_HEADER_SIZE);

		//body data
		CopyMemory(c.sendBuff + c.sendSize + PACKET_HEADER_SIZE, dataPos, bodySize);

		c.sendSize += PACKET_HEADER_SIZE + bodySize;

		return NETCODE::NONE;
	}

	void SelectNetwork::AddClient(SOCKET s, SOCKADDR_IN& addr)
	{
		int id = m_clientIndexPool.front();
		m_clientIndexPool.pop();

		m_clientPool[id].s = s;
		m_clientPool[id].isConnected = true;
		inet_ntop(AF_INET, &(addr.sin_addr.s_addr), m_clientPool[id].IP, INET_ADDRSTRLEN);

		FD_SET(s, &m_fds);

		m_clientNum++;
	
		Logger::Write(Logger::INFO, "Client %s connected", m_clientPool[id].IP);
	}

	void SelectNetwork::CloseClient(int id)
	{
		auto& target = m_clientPool[id];

		//http://stackoverflow.com/questions/15504016/c-winsock-socket-error-10038-wsaenotsock	
		//https://msdn.microsoft.com/en-us/library/windows/desktop/ms740481(v=vs.85).aspx

		auto res = shutdown(target.s, 2);
		if (res == SOCKET_ERROR)
		{
			DisplayErrorMsg("shutdown");
			return;
		}

		target.isConnected = false;

		m_clientToClosePool.push(id);

		Logger::Write(Logger::INFO, "Client %s has left", target.IP);
	}

	void SelectNetwork::ForceCloseClient(int clientIdx)
	{
		auto& target = m_clientPool[clientIdx];
/*
		auto res = shutdown(target.s, 2);
		if (res == SOCKET_ERROR)
		{
			DisplayErrorMsg("shutdown");
			return;
		}
*/
		auto& s = m_clientPool[clientIdx].s;
		FD_CLR(s, &m_fds);
		FD_CLR(s, &m_writeFds);
		FD_CLR(s, &m_readFds);

		auto res = closesocket(s);
		if (res == SOCKET_ERROR)
		{
			DisplayErrorMsg("closesocket");
			return;
		}

		target.isConnected = false;
		target.recvSize = 0;
		target.sendSize = 0;

		m_clientIndexPool.push(clientIdx);
		m_clientNum--;

		Logger::Write(Logger::INFO, "Force close client %s", target.IP);
	}

	void SelectNetwork::InitClientStuff()
	{
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			m_clientIndexPool.push(i);
		}

		m_clientNum = 0;
	}

	bool SelectNetwork::PacketQueueEmpty()
	{
		return m_recvPktQueue.empty();
	}

	SelectNetwork::~SelectNetwork()
	{
		WSACleanup();
	}

	void SelectNetwork::DisplayErrorMsg(const char* msg)
	{
		LPVOID lpMsgBuf;

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&lpMsgBuf,
			0,
			NULL);

		MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
		LocalFree(lpMsgBuf);
	}

	/*
	void SelectNetwork::ProcessClient2()
	{
		while (m_clientToClosePool.empty() == false)
		{
			auto& clientIdx = m_clientToClosePool.front();
			m_clientToClosePool.pop();

			auto& s = m_clientPool[clientIdx].s;
			FD_CLR(s, &m_fds);
			FD_CLR(s, &m_writeFds);
			FD_CLR(s, &m_readFds);
			closesocket(s);
		}
	}
	*/
}