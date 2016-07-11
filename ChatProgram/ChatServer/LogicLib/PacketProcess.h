#pragma once

#include <memory>
//Packet ����ü ������ PacketId �������� ���ԵǾ� �ִ�
#include "../../Common/Packet.h"

//�������� ����ϴ� ������ ���ԵǾ� �ִ�
//Constants, ClientSession, 
//RecvPacketInfo, Packet Id, Packet header size
#include "../ServerNetLib/Define.h"

//������ Ŭ���̾�Ʈ�� �����ϴ� �����ڵ尡 �ִ�.
//�����ڵ�� ������Ȳ�� ��Ī�� �ڵ��̴�.
#include "../../Common/ErrorCode.h"

using ERROR_CODE = NCommon::ERROR_CODE;

namespace NServerNetLib
{
	class ITcpNetwork;
	class ILog;
}

namespace NLogicLib
{	
	class ConnectedUserManager;
	class LobbyManager;
	class UserManager;

	#define CHECK_START  ERROR_CODE __result=ERROR_CODE::NONE;
	#define CHECK_ERROR(f) __result=f; goto PROCESS_ERROR;

	class PacketProcess
	{
		using TcpNet = NServerNetLib::ITcpNetwork;
		using ILog = NServerNetLib::ILog;

		//���Ϲ�ȣ, ��Ŷ�ڵ�, ��Ŷũ��, �����͸� �����Ѵ�.
		using PacketInfo = NServerNetLib::RecvPacketInfo;

		//����� �����ͷ� �ٲ� ��Ŷ�� ���޹޴´�
		//� ������������ �迭�� ���� ����Ǿ��ִ�. 
		typedef ERROR_CODE(PacketProcess::*PacketFunc)(PacketInfo);
		PacketFunc PacketFuncArray[(int)NCommon::PACKET_ID::MAX];

	public:
		PacketProcess();
		~PacketProcess();

		//����� �����ϰ�
		//Ư�� Packetó���Լ� �迭�� ��Ī��Ų��.
		void Init(TcpNet* pNetwork, UserManager* pUserMgr, LobbyManager* pLobbyMgr, ILog* pLogger);

		//Packetó���Լ� �迭�� Packet id ��ġ�� ȣ���Ѵ�.
		void Process(PacketInfo packetInfo);

		//���� �����͵��� ���¸� �˻��Ѵ�.
		void StateCheck();
	
	private:
		ILog* m_pRefLogger;
		TcpNet* m_pRefNetwork;
		UserManager* m_pRefUserMgr;
		LobbyManager* m_pRefLobbyMgr;
		std::unique_ptr<ConnectedUserManager> m_pConnectedUserManager;
						
	private:
		ERROR_CODE NtfSysConnctSession(PacketInfo packetInfo);
		
		ERROR_CODE NtfSysCloseSession(PacketInfo packetInfo);
		
		ERROR_CODE Login(PacketInfo packetInfo);
		
		ERROR_CODE LobbyList(PacketInfo packetInfo);

		ERROR_CODE LobbyEnter(PacketInfo packetInfo);

		ERROR_CODE LobbyRoomList(PacketInfo packetInfo);

		ERROR_CODE LobbyUserList(PacketInfo packetInfo);

		ERROR_CODE LobbyLeave(PacketInfo packetInfo);

		ERROR_CODE LobbyChat(PacketInfo packetInfo);

		ERROR_CODE LobbyWhisper(PacketInfo packetInfo);

		ERROR_CODE RoomEnter(PacketInfo packetInfo);

		ERROR_CODE RoomLeave(PacketInfo packetInfo);

		ERROR_CODE RoomChat(PacketInfo packetInfo);
	};
}