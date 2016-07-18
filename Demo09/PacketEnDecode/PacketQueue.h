#pragma once

//���� ������Ÿ�Կ� ���� ��Ī�� ����ϱ� ���� �����Ѵ�
#include <Windows.h>

#define		PACKETBUFFERSIZE	8192
#define		PACKETHEADERSIZE	4

//���� �ľ��� Packet Ŭ������ �뵵
//1. ������ ũ�⸦ ����(�ִ� 8192) ��Ŷ�� �ϳ� �����Ѵ�
//2. ��Ŷ�� PacketHeader + DataField�� �����ȴ�.
//3. ������ ���������� ����Ʈȭ�Ͽ� ���ۿ� �����Ѵ�
//	void*(��� ���� ���밡��)�� ����ؼ� �����ϴ�.
//	Offset�� 1�� ������ ��ġ��
class PacketQueue
{
public:
	PacketQueue();
	PacketQueue(unsigned short idValue); //Packet ID�� �ʱ�ȭ
	PacketQueue(const PacketQueue& source); //���� ������
	virtual ~PacketQueue(); 

	//���� ũ�� �ʰ�/�̴��� �˻�
	bool			isValidHeader();
	bool			isValidPacket();

	//��Ŷ���̵� ����
	PacketQueue&			id(unsigned short ID);
	//��Ŷ�Ƶ��̸� get
	unsigned short	id();

	//��Ŷ�����͸� ����
	void			clear();

	//PacketHeader->dataSize
	unsigned short	getDataFieldSize();
	//dataSize + headerSize
	int				getPacketSize() { return (getDataFieldSize() + PACKETHEADERSIZE); }
	
	int				getReceivedSize() { return receivedSize; }
	char*			getPacketBuffer() { return packetBuffer; }

	//[buffer, buffer+size] -> member buffer
	void			copyToBuffer(char* buff, int size);

	//[buffer, buffer+size] <-> member buffer
	void			readData(void* buffer, int size);
	void			writeData(void* buffer, int size);

	// Packet�� ������ ���� �����Ѵ�
	PacketQueue&	operator = (PacketQueue& packet);

	//symmetric pairs of read and write
	//bool
	//short, int, long, DWORD, __int64
	//char*
	//float, double
	//a struct, Packet

	//Write Data to Packet
	PacketQueue&	operator << (bool arg);
	PacketQueue&	operator << (int arg);
	PacketQueue&	operator << (long arg);
	PacketQueue&	operator << (DWORD arg);
	PacketQueue& operator << (__int64 arg);
	PacketQueue&	operator << (LPTSTR arg);
	PacketQueue& operator << (PacketQueue& arg);

	//Read data From Packet
	PacketQueue&	operator >> (bool& arg);
	PacketQueue&	operator >> (int& arg);
	PacketQueue&	operator >> (long& arg);
	PacketQueue&	operator >> (DWORD& arg);
	PacketQueue& operator >> (__int64& arg);
	PacketQueue& operator >> (LPTSTR arg);
	PacketQueue& operator >> (PacketQueue& arg);

protected:
	//Packet ID & Data size
	typedef struct
	{
		unsigned short*		dataSize;
		unsigned short*		protocolID;
	} HEADER;

	HEADER		packetHeader;

private:
	//Receive & Write�� ���� (�ӽ�) �۾� ����
	char		packetBuffer[PACKETBUFFERSIZE];

	//data ��ġ
	char*		dataField;

	//readPos, writePos
	char*		readPosition, *writePosition;
	
	//dataField + dataSize
	char*		endOfDataField;

	int			receivedSize;
};
