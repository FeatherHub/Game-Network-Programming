#pragma once
#include <memory>

//��Ŷ ������ ���� ����ü
//��Ŷ�� ��ɿ� ���� �����͸� ������.
#include "../../Common/Packet.h"
//��� ���� ������ �����س��� ������
//������ ��Ȳ�� ���� �ڵ带 ������.
#include "../../Common/ErrorCode.h"

//���ӽ����̽� ����
using ERROR_CODE = NCommon::ERROR_CODE;

//��� ���� ���� ����
namespace NServerNetLib
{
	struct ServerConfig;
	class ILog;
	class ITcpNetwork;
}

namespace NLogicLib
{
	//���� ���� ���� ����
	class UserManager;
	class LobbyManager;
	class PacketProcess;

	class Main
	{
	public:
		//��� ���� �� �ǰ� ���� ����� �ڷᱸ���� �ִ� ���
		//�����ڿ� �Ҹ��ڴ� �ҽ����Ͽ��� �����Ǿ�� �Ѵ�
		//�����Ϸ��� ����� ���ؼ� ����,�Ҹ� �ڵ带 �ֱ� ������
		Main();
		~Main();

		//ȣ��ο��� init -> run loop -> stop ������ �����Ѵ�.
		ERROR_CODE Init();
		void Run();
		void Stop();

	private:
		//init sub
		ERROR_CODE LoadConfig();
		//~main sub
		void Release();

	private:
		//stop sub
		bool m_IsRun = false;

		std::unique_ptr<NServerNetLib::ITcpNetwork> m_pNetwork;
		std::unique_ptr<NServerNetLib::ServerConfig> m_pServerConfig;
		std::unique_ptr<NServerNetLib::ILog> m_pLogger;

		std::unique_ptr<PacketProcess> m_pPacketProc;
		std::unique_ptr<UserManager> m_pUserMgr;
		std::unique_ptr<LobbyManager> m_pLobbyMgr;
	};
}
