#include "../../Common/Packet.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "../../Common/ErrorCode.h"
#include "User.h"
#include "UserManager.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "PacketProcess.h"

using PACKET_ID = NCommon::PACKET_ID;

/*�帧*/
//���޹��� ��Ŷ �����ϱ�

//������ ��Ŷ���� ���� ���
//�Ŵ����� ���� ���� ���
//��ȣ�� ���� ���

//��ȿ�� �������� �˻��ϱ�

//�������� �۾� ó���ϱ�

//�۾���� �����ϱ�

namespace NLogicLib
{
	ERROR_CODE PacketProcess::LobbyEnter(PacketInfo packetInfo)
	{
	CHECK_START
		// ���� ��ġ ���´� �α����� �³�?
		// �κ� ����.
		// ���� �κ� �ִ� ������� �� ����� ���Դٰ� �˷��ش�

		//���޹��� ��Ŷ�� �κ�����Ŷ���� �����Ѵ�
		auto reqPkt = (NCommon::PktLobbyEnterReq*)packetInfo.pRefData;

		//�κ��Ϳ����� ����� ������ ������ �����Ѵ�
		NCommon::PktLobbyEnterRes resPkt;

		//���޹��� ��Ŷ���� ���ǹ�ȣ ������ ���� �����Ŵ����� ���� ���� ������ ��´� 
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		//���� �������� ��� ���� ������ ��´�
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) 
		{
			CHECK_ERROR(errorCode);
		}

		//���� �������� ���� ���� ������ ��´�
		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLogIn() == false) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_ENTER_INVALID_DOMAIN);
		}

		//������ ��Ŷ���� �κ� ���̵� ��´�.
		//�κ�Ŵ����� �κ� ������ ��´�.
		auto pLobby = m_pRefLobbyMgr->GetLobby(reqPkt->LobbyId);
		if (pLobby == nullptr) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_ENTER_INVALID_LOBBY_INDEX);
		}

		//�κ� ������ ���� ������ �����Ͽ� ���������� ȣ���Ѵ�
		auto enterRet = pLobby->EnterUser(pUser);
		if (enterRet != ERROR_CODE::NONE) 
		{
			CHECK_ERROR(enterRet);
		}
		
		//�κ� ������ ���� ������ �����Ͽ� ��Ƽ�κ��������� ȣ���Ѵ�
		pLobby->NotifyLobbyEnterUserInfo(pUser);

		resPkt.MaxUserCount = pLobby->MaxUserCount();
		resPkt.MaxRoomCount = pLobby->MaxRoomCount();

		m_pRefNetwork->SendData(packetInfo.SessionIndex, 
								(short)PACKET_ID::LOBBY_ENTER_RES, 
								sizeof(NCommon::PktLobbyEnterRes), 
								(char*)&resPkt);
		
		return ERROR_CODE::NONE;

	CHECK_ERR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, 
								(short)PACKET_ID::LOBBY_ENTER_RES, 
								sizeof(NCommon::PktLobbyEnterRes), 
								(char*)&resPkt);

		return (ERROR_CODE)__result;
	}



	ERROR_CODE PacketProcess::LobbyRoomList(PacketInfo packetInfo)
	{
	CHECK_START
		// ���� �κ� �ִ��� �����Ѵ�.
		// �� ����Ʈ�� �����ش�.
		
		//���޹��� ��Ŷ���� ���ǹ�ȣ�� ��´�
		//�����Ŵ����� ���� ���ǹ�ȣ�� ���������� ��´�
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) 
		{
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_ROOM_LIST_INVALID_DOMAIN);
		}

		//������������ �κ��ȣ�� ��´�
		//�κ�Ŵ����� ���� �κ��ȣ�� �κ������� ��´�
		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_ROOM_LIST_INVALID_LOBBY_INDEX);
		}

		//���޹��� ��Ŷ�� �ٵ� �κ�븮��Ʈ ��Ŷ���� �����Ѵ�
		auto reqPkt = (NCommon::PktLobbyRoomListReq*)packetInfo.pRefData;

		//�κ�븮��Ʈ ��Ŷ���� ���۷��ȣ�� ��´�
		//������������ ���ǹ�ȣ�� ��´�
		//�κ������� ���� �븮��Ʈ������ ȣ���Ѵ�
		//���������� ���۷��ȣ�� �����Ѵ�.
		pLobby->SendRoomList(pUser->GetSessioIndex(), reqPkt->StartRoomIndex);

		return ERROR_CODE::NONE;

	CHECK_ERR :
		NCommon::PktLobbyRoomListRes resPkt;
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, 
								(short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_RES, 
								sizeof(NCommon::PktBase), 
								(char*)&resPkt);

		return (ERROR_CODE)__result;
	}



	ERROR_CODE PacketProcess::LobbyUserList(PacketInfo packetInfo)
	{
	CHECK_START
		// ���� �κ� �ִ��� �����Ѵ�.
		// ���� ����Ʈ�� �����ش�.

		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_USER_LIST_INVALID_DOMAIN);
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) {
			CHECK_ERROR(ERROR_CODE::LOBBY_USER_LIST_INVALID_LOBBY_INDEX);
		}

		auto reqPkt = (NCommon::PktLobbyUserListReq*)packetInfo.pRefData;

		pLobby->SendUserList(pUser->GetSessioIndex(), reqPkt->StartUserIndex);

		return ERROR_CODE::NONE;

	CHECK_ERR:
		NCommon::PktLobbyUserListRes resPkt;
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_USER_LIST_RES, sizeof(NCommon::PktBase), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}
	


	ERROR_CODE PacketProcess::LobbyLeave(PacketInfo packetInfo)
	{
	CHECK_START
		// ���� �κ� �ִ��� �����Ѵ�.
		// �κ񿡼� ������
		// ���� �κ� �ִ� ������� ������ ����� �ִٰ� �˷��ش�.

		//�κ񸮺극������ ����� ��� ��Ŷ�� �����Ѵ�
		NCommon::PktLobbyLeaveRes resPkt;

		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);

		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) 
		{
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_LEAVE_INVALID_DOMAIN);
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) 
		{
			CHECK_ERROR(ERROR_CODE::LOBBY_LEAVE_INVALID_LOBBY_INDEX);
		}

		auto enterRet = pLobby->LeaveUser(pUser->GetIndex());
		if (enterRet != ERROR_CODE::NONE) {
			CHECK_ERROR(enterRet);
		}

		pLobby->NotifyLobbyLeaveUserInfo(pUser);
				
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);

		return ERROR_CODE::NONE;

	CHECK_ERR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}
}