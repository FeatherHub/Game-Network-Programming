#include "PktProcHeaders.h"

ERRORCODE PacketProcessor::LobbyUserListReq(char* pData, int clientIdx)
{
//	auto reqPkt = (LobbyUserNameListReqPkt*)pData; //dummy packet
	pData;
	
	//ó��
	LobbyUserNameListResPkt resPkt;

	auto* userList = m_pRefUserManager->GetUserList();
	int cnt = 0;
	for (int i = 0; i < MAX_LOBBY_USER_NUM; i++)
	{
		if (userList[i].Connected() == true)
		{
			CopyMemory(resPkt.userNameList[cnt], userList[i].name, MAX_USER_NAME_LEN + 1);
			resPkt.userClientIdxList[cnt] = userList[i].clientIdx;
			cnt++;
		}
	}
	resPkt.userNum = cnt;

	m_pRefNetwork->SendPacket(clientIdx, (unsigned short)PacketId::LobbyUserNameListRes, (char*)&resPkt);

	return ERRORCODE::NONE;
}

ERRORCODE PacketProcessor::LobbyChatReq(char* pData, int clientIdx)
{
	auto* reqPkt = (LobbyChatReqPkt*)pData;
	
	//������ �������� �� ��ȿ�� �˻�
	const auto* senderName = m_pRefUserManager->FindUserName(clientIdx);
	if (senderName == nullptr)
	{
		return ERRORCODE::LOBBY_CHAT_SENDER_NAME_NULL;
	}

	//ó��
	LobbyChatResPkt resPkt;
	resPkt.permitted = true;
	
	m_pRefNetwork->SendPacket(clientIdx, PacketId::LobbyChatRes, (char*)&resPkt);

	//����
	m_pRefUserManager->NotifyLobbyChatMsg(clientIdx, senderName, reqPkt->msg);

	return ERRORCODE::NONE;
}