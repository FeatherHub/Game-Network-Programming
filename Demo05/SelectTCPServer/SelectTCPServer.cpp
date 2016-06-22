#pragma comment(lib, "ws2_32")

#include <winsock2.h>
#include <Ws2tcpip.h>

#include <stdlib.h>
#include <stdio.h>

const int SERVERPORT = 23452;
const int BUFSIZE = 512;

struct SOCKETINFO
{
	SOCKET sock;		//socket
	char buf[BUFSIZE+1];//data
	int recvbytes;		//received 
	int sendbytes;		//sent
};

int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];

// ���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfoAt(int nIndex);

// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) 
		err_quit("socket()");

	//SOCKADDR_IN member
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// bind()
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) 
		err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) 
		err_quit("listen()");

	// �ͺ��ŷ �������� ��ȯ
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if(retval == SOCKET_ERROR) 
		err_display("ioctlsocket()");

	// ������ ��ſ� ����� ����
	FD_SET rset, wset;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen, i;

	while(1)
	{
		// ���� �� �ʱ�ȭ
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);

		for(i=0; i<nTotalSockets; i++)
		{
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes) 
			{
				FD_SET(SocketInfoArray[i]->sock, &wset);
			} 
			else 
			{
				FD_SET(SocketInfoArray[i]->sock, &rset);
			}
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);
		if (retval == SOCKET_ERROR) 
		{
			err_quit("select()");
		}

		// ���� �� �˻�(1): Ŭ���̾�Ʈ ���� ����
		if(FD_ISSET(listen_sock, &rset))
		{
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if(client_sock == INVALID_SOCKET)
			{
				err_display("accept()");
			}
			else
			{
				char clientIP[32] = { 0, };
				inet_ntop(AF_INET, &(clientaddr.sin_addr), clientIP, 32 - 1);
				printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", clientIP, ntohs(clientaddr.sin_port));
				
				AddSocketInfo(client_sock);
			}
		}

		// ���� �� �˻�(2): ������ ���
		for(i=0; i<nTotalSockets; i++)
		{
			SOCKETINFO *sockInfo = SocketInfoArray[i];
			
			if(FD_ISSET(sockInfo->sock, &rset))
			{
				// ������ �ޱ�
				retval = recv(sockInfo->sock, sockInfo->buf, BUFSIZE, 0);
				if(retval == SOCKET_ERROR) {
					err_display("recv()");
					RemoveSocketInfoAt(i);
					continue;
				}
				else if(retval == 0) {
					RemoveSocketInfoAt(i);
					continue;
				}

				sockInfo->recvbytes = retval;
				// ���� ������ ���				
				sockInfo->buf[retval] = '\0';

				addrlen = sizeof(clientaddr);
				getpeername(sockInfo->sock, (SOCKADDR *)&clientaddr, &addrlen);
				char clientIP[32] = { 0, };
				inet_ntop(AF_INET, &(clientaddr.sin_addr), clientIP, 32 - 1);

				printf("[TCP/%s:%d] %s\n", clientIP, ntohs(clientaddr.sin_port), sockInfo->buf);
			}

			if(FD_ISSET(sockInfo->sock, &wset))
			{
				// ������ ������
				retval = send(sockInfo->sock, sockInfo->buf + sockInfo->sendbytes, 
					sockInfo->recvbytes - sockInfo->sendbytes, 0);
				
				if(retval == SOCKET_ERROR) {
					err_display("send()");
					RemoveSocketInfoAt(i);
					continue;
				}
				
				sockInfo->sendbytes += retval;
				
				if(sockInfo->recvbytes == sockInfo->sendbytes){
					sockInfo->recvbytes = sockInfo->sendbytes = 0;
				}
			}
		}
	}

	// ���� ����
	WSACleanup();
	return 0;
}

// ���� ���� �߰�
BOOL AddSocketInfo(SOCKET sock)
{
	if(nTotalSockets >= FD_SETSIZE)
	{
		printf("[����] ���� ������ �� �̻� �߰��� �� �����ϴ�!\n");
		return FALSE;
	}

	SOCKETINFO *sockInfo = new SOCKETINFO;
	if(sockInfo == NULL)
	{
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return FALSE;
	}

	//initialize
	sockInfo->sock = sock;
	sockInfo->recvbytes = 0;
	sockInfo->sendbytes = 0;
	SocketInfoArray[nTotalSockets++] = sockInfo;

	return TRUE;
}

// ���� ���� ����
void RemoveSocketInfoAt(int nIndex)
{
	SOCKETINFO *sockInfo = SocketInfoArray[nIndex];

	// Ŭ���̾�Ʈ ���� ���
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	getpeername(sockInfo->sock, (SOCKADDR *)&clientaddr, &addrlen);
	
	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(clientaddr.sin_addr), clientIP, 32 - 1);

	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", clientIP, ntohs(clientaddr.sin_port));

	closesocket(sockInfo->sock);
	
	delete sockInfo;

	//������ �������� �����ϱ� ������ ������ �������
	if(nIndex != (nTotalSockets-1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets-1];

	--nTotalSockets;
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}