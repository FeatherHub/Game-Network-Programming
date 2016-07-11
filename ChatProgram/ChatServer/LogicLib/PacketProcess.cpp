#include "../ServerNetLib/ILog.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "ConnectedUserManager.h"
#include "User.h"
#include "UserManager.h"
#include "Room.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "PacketProcess.h"

using LOG_TYPE = NServerNetLib::LOG_TYPE;

namespace NLogicLib
{	
	PacketProcess::PacketProcess() {}
	PacketProcess::~PacketProcess() {}

	void PacketProcess::Init(TcpNet* pNetwork, UserManager* pUserMgr, LobbyManager* pLobbyMgr, ILog* pLogger)
	{
		m_pRefLogger = pLogger;
		m_pRefNetwork = pNetwork;
		m_pRefUserMgr = pUserMgr;
		m_pRefLobbyMgr = pLobbyMgr;

		//ConnectedUserManager�� �����Ѵ�
		m_pConnectedUserManager = std::make_unique<ConnectedUserManager>();
		//ConnectedUserManager�� �ʱ�ȭ�Ѵ�.
		m_pConnectedUserManager->Init(pNetwork->ClientSessionPoolSize(), pNetwork, pLogger);

		//server packet id �� common packet id�� �����ϱ� ���� ������ ���δ�.
		using netLibPacketId = NServerNetLib::PACKET_ID;
		using commonPacketId = NCommon::PACKET_ID;

		for (int i = 0; i < (int)commonPacketId::MAX; ++i)
		{
			PacketFuncArray[i] = nullptr;
		}

		//Make connection with Packet Id and Function address
		PacketFuncArray[(int)netLibPacketId::NTF_SYS_CONNECT_SESSION] = &PacketProcess::NtfSysConnctSession;
		PacketFuncArray[(int)netLibPacketId::NTF_SYS_CLOSE_SESSION] = &PacketProcess::NtfSysCloseSession;
		PacketFuncArray[(int)commonPacketId::LOGIN_IN_REQ] = &PacketProcess::Login;
		PacketFuncArray[(int)commonPacketId::LOBBY_LIST_REQ] = &PacketProcess::LobbyList;
		PacketFuncArray[(int)commonPacketId::LOBBY_ENTER_REQ] = &PacketProcess::LobbyEnter;
		PacketFuncArray[(int)commonPacketId::LOBBY_ENTER_ROOM_LIST_REQ] = &PacketProcess::LobbyRoomList;
		PacketFuncArray[(int)commonPacketId::LOBBY_ENTER_USER_LIST_REQ] = &PacketProcess::LobbyUserList;
		PacketFuncArray[(int)commonPacketId::LOBBY_LEAVE_REQ] = &PacketProcess::LobbyLeave;
		PacketFuncArray[(int)commonPacketId::LOBBY_CHAT_REQ] = &PacketProcess::LobbyChat;
		PacketFuncArray[(int)commonPacketId::LOBBY_WHISPER_REQ] = &PacketProcess::LobbyWhisper;
		PacketFuncArray[(int)commonPacketId::ROOM_ENTER_REQ] = &PacketProcess::RoomEnter;
		PacketFuncArray[(int)commonPacketId::ROOM_LEAVE_REQ] = &PacketProcess::RoomLeave;
		PacketFuncArray[(int)commonPacketId::ROOM_CHAT_REQ] = &PacketProcess::RoomChat;
	}
	
	void PacketProcess::Process(PacketInfo packetInfo)
	{
		//PacketInfo�κ��� Packet Id�� �����´�
		//�ڴ��� PacketInfo�� Packet Id�� �ִٴ� ���� �ȴ�
		//PacketInfo�� Packet Id�� �����ٴ� ���� ǥ���Ѵ�
		//�ڴ��� PacketInfo�� Packet Id�� �����ٴ� ǥ�ø� �ߴ�
		//�ڴ��� (�ڵ�)���� PacketInfo�� ǥ�ñ�� ������ 
		//�ڴ��� PacketInfo�� Packet Id�� �ִٴ� ���� �ȴ�. 
		auto packetId = packetInfo.PacketId;
		
		if (PacketFuncArray[packetId] == nullptr)
		{
			//TODO: �α� �����
			return;
		}

		(this->*PacketFuncArray[packetId])(packetInfo);
	}

	void PacketProcess::StateCheck()
	{
		//���� �����͵��� �α��� ���¿� �����Ͽ� �˻��Ѵ�.
		m_pConnectedUserManager->LoginCheck();
	}

	ERROR_CODE PacketProcess::NtfSysConnctSession(PacketInfo packetInfo)
	{
		m_pConnectedUserManager->SetConnectSession(packetInfo.SessionIndex);
		
		return ERROR_CODE::NONE;
	}

	//���������� ����ü�� ��� ���δ�
	ERROR_CODE PacketProcess::NtfSysCloseSession(PacketInfo packetInfo)
	{
		auto pUser = std::get<1>(m_pRefUserMgr->GetUser(packetInfo.SessionIndex));

		if (pUser) 
		{
			auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
			if (pLobby)
			{
				auto pRoom = pLobby->GetRoom(pUser->GetRoomIndex());

				if (pRoom)
				{
					pRoom->LeaveUser(pUser->GetIndex());
					pRoom->NotifyLeaveUserInfo(pUser->GetID().c_str());
					pLobby->NotifyChangedRoomInfo(pRoom->GetIndex());

					m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d). Room Out", __FUNCTION__, packetInfo.SessionIndex);
				}

				pLobby->LeaveUser(pUser->GetIndex());

				if (pRoom == nullptr) {
					pLobby->NotifyLobbyLeaveUserInfo(pUser);
				}

				m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d). Lobby Out", __FUNCTION__, packetInfo.SessionIndex);
			}
			
			m_pRefUserMgr->RemoveUser(packetInfo.SessionIndex);		
		}
		
		m_pConnectedUserManager->SetDisConnectSession(packetInfo.SessionIndex);

		m_pRefLogger->Write(LOG_TYPE::L_INFO, "%s | NtfSysCloseSesson. sessionIndex(%d)", __FUNCTION__, packetInfo.SessionIndex);
		return ERROR_CODE::NONE;
	}
}