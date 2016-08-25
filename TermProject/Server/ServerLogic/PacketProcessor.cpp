#include "PktProcHeaders.h"

void PacketProcessor::Init(Network* network, UserManager* userMgr)
{
	m_pRefNetwork = network;
	m_pRefUserManager = userMgr;
}

ERRORCODE PacketProcessor::TestReq(char* pData, int clientIdx)
{
	//������ ����
	TestReqPkt* reqPkt = (TestReqPkt*)pData;
	int cnt = reqPkt->num;

	//������ ����
	TestResPkt resPkt;
	resPkt.num = cnt;

	//������ ����
	m_pRefNetwork->SendPacket(clientIdx, (unsigned short)PacketId::TestRes, (char*)&resPkt);

	return ERRORCODE::NONE;
}
