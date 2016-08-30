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

ERRORCODE RequestManager::RequestLogin(const std::string& name, const std::string& pw)
{
	if (name.length() == 0 || pw.length() == 0)
	{
		return ERRORCODE::LOGIN_REQ_ID_OR_PW_EMPTY;
	}

	if (name.length() > MAX_USER_NAME_LEN || pw.length() > MAX_USER_PW_LEN)
	{
		return ERRORCODE::LOGIN_REQ_ID_OR_PW_OVERFLOW;
	}

	/* ���Ŀ� ����ȭ�ϱ�
	/* SendPacket�� �������ڷ� �������� ��ġ�� �޾Ƽ�
	/* �����۾��� SendPacket���� �ϰ������� ó���Ѵ�
	*/

	LoginReqPkt reqPkt;

	CopyMemory(reqPkt.name, name.c_str(), name.length());
	reqPkt.name[name.length()] = '\0';

	CopyMemory(reqPkt.pw, pw.c_str(), pw.length());
	reqPkt.pw[pw.length()] = '\0';

	m_refNetwork->SendPacket(PacketId::LoginReq, (char*)&reqPkt);
	
	return ERRORCODE::LOGIN_REQ_OK;
}

ERRORCODE RequestManager::RequestUserNameList()
{
	LobbyUserNameListReqPkt reqPkt;
	
	m_refNetwork->SendPacket(PacketId::LobbyUserNameListReq, (char*)&reqPkt);
	
	return ERRORCODE::LOBBY_USER_ID_LIST_REQ_OK;
}
