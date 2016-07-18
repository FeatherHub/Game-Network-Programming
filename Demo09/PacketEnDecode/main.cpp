#include <iostream>
using namespace std;

#include "PacketQueue.h"

#include <memory.h>

const unsigned short PACKET_ID_LOGIN = 11;

//����:
//1. ��Ŷť�� ��Ŷ���� �״´�
//2. ��Ŷ�� ��� ó���Ѵ�

//��Ŷ �б�/���� �׽�Ʈ �Լ�
//������ ��Ŷ�� Receive ������ ����
void ReceivePacket(PacketQueue& packet);

void Print(int* a, int* a1, int* a2)
{
	cout << a << endl;
	cout << a1 << endl;
	cout << a2 << endl;
}


int main()
{	
	PacketQueue packet1(PACKET_ID_LOGIN);
	packet1 << L"jacking75";
	packet1 << L"123qwe";
	packet1 << 25;

	ReceivePacket(packet1);

	//int* a = new int[3];
	//cout << a << endl << endl;
	//cout << a << endl << a + 1 << endl << a + 2 << endl << endl;
	//cout << a << endl << a++ << endl << a++ << endl << endl;
	//cout << a++ << endl << a++ << endl << a << endl << endl;
	//Print(a, a++, a++);
	//cout << endl;
	//cout << a << endl;

	return 0;
}


void ReceivePacket(PacketQueue& packet)
{
	switch (packet.id())
	{
		case PACKET_ID_LOGIN:
			//1.
			//Packet ID�� Login�̸�
			//���� ID�� PW�� �ǹ��ϴ�
			//wchar_t[32] Ÿ��/ũ����
			//������ �� ����
			//Version�� �ǹ��ϴ�
			//int Ÿ��/ũ����
			//�����Ͱ� �ִ�
			//2.
			//��ӵ� ������� �д´�
		{
			wchar_t UserID[32] = { 0, };
			wchar_t UserPW[32] = { 0, };
			int version = 0;

			//��Ŷ ����
			packet >> UserID;
			packet >> UserPW;
			packet >> version;
			
			//��� ���
			printf("UserID:%S, UserPW:%S, Version:%d\n", UserID, UserPW, version);
		}
		break;
	}
}