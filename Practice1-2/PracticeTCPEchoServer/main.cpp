#include <winSock2.h>
#include <ws2tcpip.h>

#include <stdio.h>

void DisplayError(char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

void PrintErrAndExit(const char* errType, bool exitProg = true)
{
	printf("Error : %s \n", errType);
	
	if (exitProg) exit(-1);
}

void PrintIPAndPort(SOCKADDR_IN* sockAddr)
{
	if (sockAddr == nullptr) return;

	USHORT port = ntohs(sockAddr->sin_port);
	char IP[32];
	inet_ntop(AF_INET, &(sockAddr->sin_addr), IP, sizeof(IP));

	printf("IP: %s Port: %d \n", IP, port);
}

int main()
{
	int ret = 0;

	WSADATA wsaData;

	//���μ����� Winsock dll�� ����� �� �ֵ��� �ʱ�ȭ�Ѵ�.
	//������ 0�� �����Ѵ�. wsaData�� ����� �ʱ�ȭ�ȴ�.
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) PrintErrAndExit("WSAStartup");
	
	//printf("Max socket num : %d \n", wsaData.iMaxSockets);

	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
	/* SOCK_STREAM
	: provides sequenced, reliable, two-way, 
	connection-based byte streams with an OOB data. 
	: This socket type uses the Transmission Control Protocol (TCP) 
	for the Internet address family (AF_INET or AF_INET6).
	*/

	//You can get specific error code by WSAGetLastError(); 
	if (listenSock == INVALID_SOCKET) PrintErrAndExit("Socket");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(23452);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	//������� ���� ������ �ʿ�� �Ѵ�
	ret = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (ret == SOCKET_ERROR) PrintErrAndExit("bind");

	PrintIPAndPort(&serverAddr);

	/* listen 
		- �������� ��� 0�� �����Ѵ�.
		- �������� ��� SOCKET_ERROR�� �����ϰ� error code�� �����Ѵ�

		- ������ TCP ��Ʈ ���¸� LISTENING���� �ٲ۴�
		- Ŭ���̾�Ʈ ������ �޾Ƶ��� �� �ִٴ� �ǹ̴�
		- backlog: Ŭ���̾�Ʈ ���� ť�� ����
	*/
	ret = listen(listenSock, SOMAXCONN);
	if (ret == SOCKET_ERROR) PrintErrAndExit("listen");

	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	
	//initially contains the amount of space pointed to by addr.
	//On return it will contain the actual length in bytes of the address returned.
	int clientAddrLen = sizeof(clientAddr);
	char buf[512] = { 0, };
	int recvNum = 0;
	int sendNum = 0;
	while (true)
	{
		/* accept
			- ������ Ŭ���̾�Ʈ�� ������ �����Ѵ�
			- �Ű������� �ѱ� �ּҺ����� Ŭ���̾�Ʈ�� ������ ä���ش�

			- ������ Ŭ���̾�Ʈ�� ���� ���
			- WAIT_STATE, SUSPENDED_STATE�̴�
		*/
		clientSock = accept(listenSock, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (clientSock == INVALID_SOCKET) 
		{
			if (WSAEWOULDBLOCK == WSAGetLastError())
			{//listen socket is marked as nonblocking 
			//and no connections are present to be accepted
				continue;
			}
			else
			{
				PrintErrAndExit("accept", false);
				//closesocket(clientSock);
				continue;
			}
		}
		
		PrintIPAndPort(&clientAddr);

		while (true)
		{
			//recv : connected sock, buffer pointer, buffer size, option
			//If no incoming data is available at the socket, 
			//the recv call blocks and waits for data to arrive according to the blocking rules 
			ret = recv(clientSock, buf, sizeof(buf), 0);
			if (ret == SOCKET_ERROR) { DisplayError("recv"); break; }
			else if (ret == 0) { printf("Gracefully closed \n"); break; }

			//recv buffer process
			recvNum = ret;
			buf[recvNum++] = '\0';

			printf("recv %d : %s \n", recvNum, buf);

			//send
			ret = send(clientSock, buf, recvNum, 0);
			if (ret == SOCKET_ERROR) PrintErrAndExit("send");
			printf("send %d : %s \n", recvNum, buf);

			sendNum = ret;

			printf("\n");
		}

		printf("close connection: ");
		PrintIPAndPort(&clientAddr);
		closesocket(clientSock);
	}

	closesocket(listenSock);

	WSACleanup();

	return 0;
}