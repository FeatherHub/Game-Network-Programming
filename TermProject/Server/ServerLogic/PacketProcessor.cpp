#include "PktProcHeaders.h"

void PacketProcessor::Init(Network* network)
{
	m_pRefNetwork = network;

	m_pUserManager = new UserManager();
	m_pUserManager->Init(network);
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
	m_pRefNetwork->SendPacket(clientIdx, PacketId::TestRes, (char*)&resPkt);

	return ERRORCODE::NONE;
}
