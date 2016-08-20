#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <stdlib.h>
#include <stdio.h>

const int SERVER_PORT = 23452;
const int BUFFER_SIZE = 512;

struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFFER_SIZE + 1];
	int recvBytes;
	int sendBytes;
};

int nTotalSockets = 0;
SOCKETINFO* socketInfoArray[FD_SETSIZE];

bool AddSocketInfo(SOCKET s);
void RemoveSocketInfoAt(int idx);

int main()
{
	//wsa
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//socket
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	//socket address
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind
	bind(listen_sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	//listen
	listen(listen_sock, SOMAXCONN);

	//non-blocking socket
	u_long on = 1;
	ioctlsocket(listen_sock, FIONBIO, &on);

	//data for network
	FD_SET read_set, write_set;
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	int i;

	while (true)
	{
		//�ʱⰪ���� ����
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_SET(listen_sock, &read_set);

		//���� �� ������ ���¿� ���� read_set�� write_set ����
		for (i = 0; i < nTotalSockets; i++)
		{
			//���� ������ �� > �۽� ������ ��
			if (socketInfoArray[i]->recvBytes > socketInfoArray[i]->sendBytes)
			{
				FD_SET(socketInfoArray[i]->sock, &write_set);
			}
			//�۽� ������ �� > ���� ������ ��
			else
			{
				FD_SET(socketInfoArray[i]->sock, &read_set);
			}
		}

		//���Ҹ� ����
		select(0, &read_set, &write_set, NULL, NULL);

		//�о���� ���Ұ� ������
		if (FD_ISSET(listen_sock, &read_set))
		{
			//Ŭ���̾�Ʈ ���� ����
			client_sock = accept(listen_sock, (SOCKADDR*)&clientAddr, &addrLen);

			AddSocketInfo(client_sock);
		}

		//��� �������տ� ����
		for (i = 0; i < nTotalSockets; i++)
		{
			auto sockInfo = socketInfoArray[i];
			
			//���Ŵ���� �� ó��
			if (FD_ISSET(sockInfo->sock, &read_set))
			{
				int recvLen = recv(sockInfo->sock, sockInfo->buf, BUFFER_SIZE, 0);

				if (recvLen == 0)
				{
					RemoveSocketInfoAt(i);
					continue;
				}

				sockInfo->recvBytes = recvLen;
				sockInfo->buf[recvLen] = '\0';

				printf("%d \n", sockInfo->buf);
			}

			//�۽Ŵ���� �� ó��
			if (FD_ISSET(sockInfo->sock, &write_set))
			{
				send(sockInfo->sock, sockInfo->buf, sockInfo->sendBytes, 0);
			}
		}
	}

	//free memories
	closesocket(listen_sock);

	WSACleanup();

	return 0;
}

bool AddSocketInfo(SOCKET s)
{
	auto socketInfo = new SOCKETINFO;

	socketInfo->sock = s;
	socketInfo->sendBytes = 0;
	socketInfo->recvBytes = 0;
	
	socketInfoArray[nTotalSockets++] = socketInfo;

	return true;
}

void RemoveSocketInfoAt(int idx)
{
	auto sockInfo = socketInfoArray[idx];

	closesocket(sockInfo->sock);
	delete sockInfo;

	if (idx != (nTotalSockets - 1))
	{
		socketInfoArray[idx] = socketInfoArray[nTotalSockets - 1];
	}

	nTotalSockets--;
}
