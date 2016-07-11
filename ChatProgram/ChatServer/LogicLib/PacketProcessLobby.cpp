#include "../../Common/Packet.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "../../Common/ErrorCode.h"
#include "User.h"
#include "UserManager.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "PacketProcess.h"

using PACKET_ID = NCommon::PACKET_ID;

//�κ� ���� -> �ش� ȣ���� �� ����Ʈ & ���� ����Ʈ
//�κ� ä�� & �ӼӸ�
//�κ� ����

namespace NLogicLib
{
	//QSF
	ERROR_CODE PacketProcess::LobbyEnter(PacketInfo packetInfo)
	{
	CHECK_START
		//��Ŷ�� �д´�.
		auto reqPkt = (NCommon::PktLobbyEnterReq*)packetInfo.pRefData;

		//������ & ���� ��ȿ�� �˻�
		//���� ��ġ�� �α����� �ƴϸ� ��ȿ���� �ʴ�
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLogIn() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_ENTER_INVALID_DOMAIN);
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(reqPkt->LobbyId);
		if (pLobby == nullptr) {
			CHECK_ERROR(ERROR_CODE::LOBBY_ENTER_INVALID_LOBBY_INDEX);
		}
		//�˻� ��

		//��ûŬ���� ��û�� ó���Ѵ�.
		auto enterRet = pLobby->EnterUser(pUser);
		if (enterRet != ERROR_CODE::NONE) {
			CHECK_ERROR(enterRet);
		}
		
		//�ٸ�Ŭ�󿡰� ����� �����Ѵ�.
		pLobby->NotifyLobbyEnterUserInfo(pUser);

		//��ûŬ�󿡰� ����� �����Ѵ�.
		NCommon::PktLobbyEnterRes resPkt;
		resPkt.MaxUserCount = pLobby->MaxUserCount();
		resPkt.MaxRoomCount = pLobby->MaxRoomCount();
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
		return ERROR_CODE::NONE;

	PROCESS_ERROR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}

	//�ش� �κ��� �븮��Ʈ�� �� ���� ������ ��� ������.(��Ŷ���̾�׷� ����)
	ERROR_CODE PacketProcess::LobbyRoomList(PacketInfo packetInfo)
	{
	CHECK_START
		//�����Ϳ� ���� ��ȿ�� �˻�
		//���� ��ġ�� �κ� �ƴϸ� ��ȿ���� �ʴ�.
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_ROOM_LIST_INVALID_DOMAIN);
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) {
			CHECK_ERROR(ERROR_CODE::LOBBY_ROOM_LIST_INVALID_LOBBY_INDEX);
		}
		//�˻� ��

		//��ûŬ���� ��û�� ó���Ѵ�.
		auto reqPkt = (NCommon::PktLobbyRoomListReq*)packetInfo.pRefData;
		//��ûŬ�� ��ġ�� �κ��� �� ����Ʈ�� �����Ѵ�.
		pLobby->SendRoomList(pUser->GetSessioIndex(), reqPkt->StartRoomIndex);
		//											(�κ񿡼� ó�� ��û�� ���� 0)
		return ERROR_CODE::NONE;

	PROCESS_ERROR :
		NCommon::PktLobbyRoomListRes resPkt;
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_RES, sizeof(NCommon::PktBase), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}

	//QS
	ERROR_CODE PacketProcess::LobbyUserList(PacketInfo packetInfo)
	{
	CHECK_START
		//�ʿ��� ������ �ε� & ������/������ ��ȿ�� �˻�
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
		//�˻� ��

		//�ش�Ŭ�󿡰� ��������Ʈ�� �����Ѵ�
		auto reqPkt = (NCommon::PktLobbyUserListReq*)packetInfo.pRefData;
		pLobby->SendUserList(pUser->GetSessioIndex(), reqPkt->StartUserIndex);

		return ERROR_CODE::NONE;
	PROCESS_ERROR:
		NCommon::PktLobbyUserListRes resPkt;
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_USER_LIST_RES, sizeof(NCommon::PktBase), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}
	
	//QSF
	ERROR_CODE PacketProcess::LobbyLeave(PacketInfo packetInfo)
	{
	CHECK_START
		//�˻�
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_LEAVE_INVALID_DOMAIN);
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) {
			CHECK_ERROR(ERROR_CODE::LOBBY_LEAVE_INVALID_LOBBY_INDEX);
		}
		//�˻� ��

		//��ûŬ���� �κ񳪰��� ��û�� ó���Ѵ�.
		auto enterRet = pLobby->LeaveUser(pUser->GetIndex());
		if (enterRet != ERROR_CODE::NONE) {
			CHECK_ERROR(enterRet);
		}

		//(��ûŬ��� ����ó����)

		//�ٸ�Ŭ��鿡�� ���ŵ� �κ������� �����Ѵ�.
		pLobby->NotifyLobbyLeaveUserInfo(pUser);

		//��ûŬ�󿡰� ����� �����Ѵ�.
		NCommon::PktLobbyLeaveRes resPkt;
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);

		return ERROR_CODE::NONE;
	PROCESS_ERROR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}

	//QSF
	ERROR_CODE PacketProcess::LobbyChat(PacketInfo packetInfo)
	{
	CHECK_START
		//������ �ʿ��� ������ �ε�
		//������ �������� ��ȿ�� �˻�
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_CHAT_INVALID_DOMAIN);
		}

		auto lobbyIndex = pUser->GetLobbyIndex();
		auto pLobby = m_pRefLobbyMgr->GetLobby(lobbyIndex);
		if (pLobby == nullptr) {
			CHECK_ERROR(ERROR_CODE::LOBBY_CHAT_INVALID_LOBBY_INDEX);
		}
		//�ε�&�˻� ��

		//��ûŬ�� ��û�� ���񽺸� ó���Ѵ�.

		//���Ŭ�󿡰� ���񽺸� �����Ѵ�.
		//���Ŭ�󿡰� ��ûŬ���� ���̵�� ������ �����Ѵ�.
		auto reqPkt = (NCommon::PktLobbyChatReq*)packetInfo.pRefData;
		pLobby->NotifyChat(pUser->GetSessioIndex(), pUser->GetID().c_str(), reqPkt->Msg);
		
		//��ûŬ�󿡰� ����/���а���� �����Ѵ�.
		NCommon::PktLobbyChatRes resPkt;
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
		
		return ERROR_CODE::NONE;

	PROCESS_ERROR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}

	//QSF
	//��û�� ������ == ���� ������ 
	ERROR_CODE PacketProcess::LobbyWhisper(PacketInfo packetInfo)
	{
	CHECK_START
		//������ �ʿ��� �����͸� �ε��Ѵ�
		//������ �������� ��ȿ���� �˻��Ѵ�
		auto pUserRet = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		auto errorCode = std::get<0>(pUserRet);

		if (errorCode != ERROR_CODE::NONE) {
			CHECK_ERROR(errorCode);
		}

		auto pUser = std::get<1>(pUserRet);

		if (pUser->IsCurDomainInLobby() == false) {
			CHECK_ERROR(ERROR_CODE::LOBBY_CHAT_INVALID_DOMAIN);
		}

		auto reqPkt = (NCommon::PktLobbyWhisperReq*)packetInfo.pRefData;
	
		auto pTargetRef = m_pRefUserMgr->GetUser(reqPkt->UserID);

		auto pTargetUser = std::get<1>(pTargetRef);

		auto& pTargetId = pTargetUser->GetID();
		//�ε�&�˻� ��

		//Notify to target user
		m_pRefNetwork->SendData(pTargetUser->GetSessioIndex(), (short)packetInfo.PacketId, sizeof(reqPkt), (char*)&reqPkt);

		//Send result to requesting user
		m_pRefNetwork->SendData(pUser->GetSessioIndex(), (short)packetInfo.PacketId, sizeof(reqPkt), (char*)&reqPkt);
		
		NCommon::PktLobbyChatRes resPkt;
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
		return ERROR_CODE::NONE;

	PROCESS_ERROR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_CHAT_RES, sizeof(resPkt), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}
}