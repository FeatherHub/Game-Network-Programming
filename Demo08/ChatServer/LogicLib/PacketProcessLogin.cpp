
#include "../ServerNetLib/TcpNetwork.h"
#include "../../Common/Packet.h"
#include "../../Common/ErrorCode.h"

#include "User.h"
#include "UserManager.h"
#include "LobbyManager.h"
#include "PacketProcess.h"

using PACKET_ID = NCommon::PACKET_ID;

namespace NLogicLib
{
	ERROR_CODE PacketProcess::Login(PacketInfo packetInfo)
	{
	CHECK_START
		//TO-DO
		//1. �α��� ��Ŷ ũ�� �˻�
		//2. ���̵�-��й�ȣ ��ġ �˻�
		
		NCommon::PktLogInRes resPkt;

		auto loginPkt = (NCommon::PktLoginReq*)packetInfo.pRefData;
		
		auto ret = m_pRefUserMgr->AddUser(packetInfo.SessionIndex, loginPkt->szID);

		if (ret != ERROR_CODE::NONE) CHECK_ERROR(ret);

		resPkt.ErrorCode = (short)ret;

		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);

		return ERROR_CODE::NONE;

	PROCESS_ERROR:
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOGIN_IN_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}

	ERROR_CODE PacketProcess::LobbyList(PacketInfo packetInfo)
	{
	CHECK_START
		//TO-DO: ������ �α��� ȭ�鿡 �ִ��� �˻�

		auto pUserInfo = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		auto errorCode = std::get<0>(pUserInfo);
		if (errorCode != ERROR_CODE::NONE) CHECK_ERROR(errorCode);
	
		auto pUser = std::get<1>(pUserInfo);
		if (pUser->IsCurDomainInLogIn() == false) CHECK_ERROR(ERROR_CODE::LOBBY_LIST_INVALID_DOMAIN);
		
		m_pRefLobbyMgr->SendLobbyListInfo(packetInfo.SessionIndex);
		
		return ERROR_CODE::NONE;

	PROCESS_ERROR:
		NCommon::PktLobbyListRes resPkt;
		resPkt.SetError(__result);
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LIST_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);
		return (ERROR_CODE)__result;
	}
}