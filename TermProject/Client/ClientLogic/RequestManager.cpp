#include "RequestManager.h"
#include "..\Network\Network.h"

#include "..\..\Common\Packet.h"
#include "..\..\Common\PacketId.h"

RequestManager* RequestManager::m_instance = nullptr;

RequestManager* RequestManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new RequestManager();
	}

	return m_instance;
}

RequestManager::RequestManager()
{
	m_refNetwork = Network::GetInstance();
}

ERRORCODE RequestManager::RequestConnect(const std::string& port, const std::string& ip)
{
	unsigned short portNumber = atoi(port.c_str());

	auto res = m_refNetwork->ConnectTo(ip.c_str(), portNumber);

	if (res == true)
	{
		return ERRORCODE::CONNECT_REQ_OK;
	}
	else
	{
		return ERRORCODE::CONNECT_REQ_FAIL;
	}
}

ERRORCODE RequestManager::RequestLogin(const std::string& id, const std::string& pw)
{
	if (id.length() == 0 || pw.length() == 0)
	{
		return ERRORCODE::LOGIN_REQ_ID_OR_PW_EMPTY;
	}

	LoginReqPkt reqPkt;

	/* ���Ŀ� ����ȭ�ϱ�
	/* SendPacket�� �������ڷ� �������� ��ġ�� �޾Ƽ�
	/* �����۾��� SendPacket���� �ϰ������� ó���Ѵ�
	*/

	CopyMemory(&reqPkt.id, &id, id.length());
	reqPkt.id[id.length()] = '\0';

	CopyMemory(&reqPkt.pw, &pw, pw.length());
	reqPkt.id[pw.length()] = '\0';

	m_refNetwork->SendPacket(PacketId::LoginReq, (char*)&reqPkt);
	
	return ERRORCODE::LOGIN_REQ_OK;
}