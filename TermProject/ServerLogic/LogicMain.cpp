#include "LogicMain.h"
#include "Network.h"
#include "PacketManager.h"

void LogicMain::Init()
{
	m_isRun = true;

	//���⼭ ��Ʈ��ũ�� ����.
	m_network = new Network();
	m_network->Init(23452, "127.0.0.1");

	m_pktMgr = new PacketManager();
	m_pktMgr->Init();
}

void LogicMain::Run()
{
	while (m_isRun)
	{
		m_network->Run();

		while (m_network->PacketQueueEmpty() == false)
		{
			auto pkt = m_network->GetPacket();

			m_pktMgr->ProcessPacket(pkt);
		}
	}
}
