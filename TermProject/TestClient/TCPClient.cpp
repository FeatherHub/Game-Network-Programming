#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\Common\Packet.h"
#include "..\Common\PacketId.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 23452
#define BUFSIZE    512

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

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) // success
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	inet_pton(AF_INET, SERVERIP, (void *)&serveraddr.sin_addr.s_addr);

	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) {
		err_quit("connect()");
	}

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE+1];
	int len;

	//����
	TestReqPkt reqPkt;
	reqPkt.num = 10;

	//����
	Packet pkt;
	pkt.id = TestReq;
	pkt.bodySize = sizeof(TestReqPkt);
	pkt.data = (char*)&reqPkt;

	send(sock, (char*)&pkt, PACKET_HEADER_SIZE + pkt.bodySize, 0);

	char recvBuf[512] = { 0, };
	int readPos = 0;
	int recvNum = 0;

	while (1)
	{
		//�ޱ�
		int recvedNum = recv(sock, recvBuf + readPos, 512 - recvNum, 0);

		recvNum += recvedNum;

		while (recvNum < PACKET_HEADER_SIZE)
		{
			//��Ŷȭ
			Packet pkt;
			CopyMemory(&pkt, recvBuf + readPos, sizeof(pkt));

			if (recvNum < PACKET_HEADER_SIZE + pkt.bodySize)
			{
				break;
			}

			//���� ���� ����
			readPos += PACKET_HEADER_SIZE + pkt.bodySize;
			recvNum -= PACKET_HEADER_SIZE + pkt.bodySize;

			//������ ����
			TestResPkt* resPkt = (TestResPkt*)pkt.data;

			//���
			printf("%d \n", resPkt->num);

			if (resPkt->num == 0)
			{
				printf("test success \n");
				break;
			}

			//���� �����ʹ� ��������� ���� ����
			CopyMemory(recvBuf, recvBuf + readPos, recvNum);
			readPos = 0;
		}
	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}