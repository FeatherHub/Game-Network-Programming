#include <WinSock2.h>
#include <WS2tcpip.h>

#include <Windows.h>
#include <Process.h>

#include <stdio.h>

const int READ = 3;
const int WRITE = 5;

const int BUFF_SIZE = 100;

const USHORT PORT_NUM = 23452;

struct SOCK_INFO
{
	SOCKET sock;
	SOCKADDR_IN addr;
};

struct SOCK_DATA
{
public:
	SOCK_DATA()
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsaBuf.len = BUFF_SIZE;
		wsaBuf.buf = buff;
		rwMode = READ;
	}

	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buff[BUFF_SIZE];
	int rwMode;
};

UINT __stdcall EchoThread(LPVOID pComPort);

int main()
{
	int res = 0;

	WSADATA wsaData;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	if (res != 0) 
		printf("WSAStartup err \n");

	
	HANDLE hComPort = CreateIoCompletionPort //1. IOCP ����
	( 
		INVALID_HANDLE_VALUE, //IOCP�� ������ �ڵ�
							  //���� �� INVALID_HANDLE_VALUE
		NULL, //IOCP �ڵ�
			  //���� �� NULL

		0, //IO �Ϸ�� �Ѿ ��
		   //����ڰ� �����Ѵ�

		0 //�� ���� ������ �� �ִ� �ִ� ������ ����
		  //0 �����ϸ� ���μ��� ���ڷ� �ڵ� �����ȴ�.
	);
	
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	DWORD procNum = sysInfo.dwNumberOfProcessors;
	for(DWORD i = 0; 
		i < procNum * 2 + 1; //�۾� ó�� ���� blocking �Լ��� ������ �Ǿ�
						     //�����尡 Paused Thread List�� ���� ��Ȳ���� 
							 //�ٷ� �ٸ� �۾��� ó���� �� �ְ� 
							 //������ ������ CPU ���� ���� ũ�� ��´�.
		i++)
	{
		//IO�۾��� �Ϸ�� ���� ó���� ������ ������ Ǯ�� �����Ѵ�
		_beginthreadex(nullptr, 0,
						EchoThread, (LPVOID)hComPort,
						0, nullptr);
	}

	SOCKET hListenSock = WSASocket(
						AF_INET, SOCK_STREAM, 0,
						nullptr, 0,
						WSA_FLAG_OVERLAPPED);
	
	if (hListenSock == INVALID_SOCKET)
		printf("WSASocket err \n");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	res = bind(hListenSock, 
				(SOCKADDR*)&serverAddr,
				sizeof(serverAddr));

	if (res == SOCKET_ERROR) 
		printf("bind err \n");

	res = listen(hListenSock, SOMAXCONN);

	if (res == SOCKET_ERROR)
		printf("listen err \n");

	printf("Server run... \n");

	while (true)
	{
		SOCKET hClientSock;
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		hClientSock = accept(hListenSock,
							(SOCKADDR*)&clientAddr, 
							&addrLen);

		if (hClientSock == INVALID_SOCKET)
			printf("accept err \n");
		
		SOCK_INFO* pSockInfo = new SOCK_INFO();
		pSockInfo->sock = hClientSock;
		CopyMemory(&pSockInfo->addr, &clientAddr, addrLen);
		//sockInfo->addr = clientAddr;

		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1); 
		printf("%s has connected \n", clientIP);

		//2. IO��ġ�� IOCP����
		CreateIoCompletionPort(
			(HANDLE)hClientSock, //IOCP�� ������ �ڵ�

			hComPort, //IOCP �ڵ�

			(ULONG_PTR)pSockInfo, //����� ���� Ű

			0 //Released Thread List�� ��� �� �� �ִ� �ִ� ������ ����
			  //��, ���ÿ� �۾��� ������ �������� ����
			  //������ �� CPU �ϳ��� ���� �̻����� �����ս��� ���� ������
			  //CPU�� ������ ���� �� ���� �����Ѵ�.
		);
	
		SOCK_DATA* pSockData = new SOCK_DATA();

		DWORD recvNum = 0;
		DWORD flag = 0;
		res = WSARecv(pSockInfo->sock,
				&pSockData->wsaBuf,
				1, //number of buff
				&recvNum,
				&flag,
				&pSockData->overlapped,
				nullptr);

		if (res == 0)
			printf("client left \n");
		else if (res == SOCKET_ERROR)
		{
			res = WSAGetLastError();
			if (res == WSA_IO_PENDING)
				printf("WSARecv WSA IO PENDING \n");
			else if (res == WSAEWOULDBLOCK)
				printf("WSARecv WSAEWOULDBLOCK \n");
			else
				printf("WSARecv err \n");
		}
	}

	return 0;
}

UINT __stdcall EchoThread(LPVOID pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;

	static int echoThreadRunNum = 0;
	printf("Thread %d run \n", echoThreadRunNum++);

	while (true)
	{
		DWORD recvNum = 0;
		SOCK_INFO* pSockInfo = nullptr;
		SOCK_DATA* pSockData = nullptr;

		int ret = GetQueuedCompletionStatus	 //�����尡 GQCS ȣ���ϸ�
		(									 //Blocking�ǰ�
											 //Waiting Thread Queue��
											 //������ ID�� ���� 
			hComPort, //handle of an iocp object								 
			&recvNum, //trans bytes					 
			(LPDWORD)&pSockInfo, //user defined key
			(LPOVERLAPPED*)&pSockData, //overlapped
			INFINITE //time out
		); 

//		if(ret == 0 && WSAGetLastError() == WAIT_TIMEOUT)
//			continue;

		SOCKET sock = pSockInfo->sock;

		if (pSockData->rwMode == READ)
		{
			if (recvNum == 0) 
			{
				char clientIP[32] = { 0, };
				inet_ntop(AF_INET, &(pSockInfo->addr.sin_addr), clientIP, 32 - 1);
				printf("%s has left \n", clientIP);
			
				closesocket(pSockInfo->sock);
				delete pSockInfo;
				delete pSockData;

				continue;
			}

			printf("Msg recved : %s \n", pSockData->buff);

			//echoing
			ZeroMemory(&pSockData->overlapped,
				sizeof(pSockData->overlapped));
			pSockData->wsaBuf.len = recvNum;
			pSockData->rwMode = WRITE;

			WSASend(sock,
				&pSockData->wsaBuf,
				1, //number of buff
				nullptr, //sent bytes buff
				0, //flag
				&pSockData->overlapped,
				nullptr);

			pSockData = new SOCK_DATA();
			DWORD flag = 0;
			WSARecv(sock,
				&pSockData->wsaBuf,
				1,
				nullptr,
				&flag,
				&pSockData->overlapped,
				nullptr);
		}
	}

	return 0;
}