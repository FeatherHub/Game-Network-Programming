#pragma once


class IOCPNetwork;
class UserManager;

enum ERRORCODE : int;

class PacketProcessor
{
	friend class PacketProcArray;
	using Network = IOCPNetwork;
public:
	void Init(Network* network, UserManager* userMgr);
private:
	ERRORCODE TestReq(char* pData, int clientIdx);
	ERRORCODE LoginReq(char* pData, int clientIdx);
	ERRORCODE LobbyUserListReq(char* pData, int clientIdx);
	ERRORCODE LobbyChatReq(char* pData, int clientIdx);
private:
	Network* m_pRefNetwork;
	UserManager* m_pRefUserManager;
};