#include <thread>
#include <chrono>

//���� ���� ����(User���� �� �����ڵ�)�� �����س��� ������
#include "../ServerNetLib/ServerNetErrorCode.h"
//�������� ����ϴ� ����ü, �������� ��Ƴ��� ����
//ServerConfig, ClientSession, RecvPacket, PacketHeader
//Packet Id, Socket close case (User���� ���� ���� �����ڵ�)
#include "../ServerNetLib/Define.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "ConsoleLogger.h"
#include "LobbyManager.h"
#include "UserManager.h"
#include "PacketProcess.h"
#include "Main.h"

using LOG_TYPE = NServerNetLib::LOG_TYPE;
using NET_ERROR_CODE = NServerNetLib::NET_ERROR_CODE;

namespace NLogicLib
{
	Main::Main()
	{
	}

	Main::~Main()
	{
		Release();
	}

	ERROR_CODE Main::Init()
	{
		//Interface�� Implemented�� �ִ´�
		//Interface�� Mockup Implemented�� ���� Unit test�� �ϱ� ���ؼ���
		m_pLogger = std::make_unique<ConsoleLog>();

		//m_pServerConfig �ʱ�ȭ
		LoadConfig();

		//��Ʈ��ũ�� �����Ѵ�.
		m_pNetwork = std::make_unique<NServerNetLib::TcpNetwork>();
		//��Ʈ��ũ�� �ʱ�ȭȯ��.
		//���������� �ΰŸ� �����Ѵ�.
		auto result = m_pNetwork->Init(m_pServerConfig.get(), m_pLogger.get());
		
		//���������� ���ܳ� ������ �߻��� ��쿡 ���� ó���Ѵ�
		if (result != NET_ERROR_CODE::NONE)
		{
			m_pLogger->Write(LOG_TYPE::L_ERROR, "%s | Init Fail. NetErrorCode(%s)", __FUNCTION__, (short)result);
			return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
		}
		
		//�����Ŵ����� �����Ѵ�.
		m_pUserMgr = std::make_unique<UserManager>();
		//�����Ŵ����� �ʱ�ȭ�Ѵ�.
		//�����������κ��� ������ ��´�.
		m_pUserMgr->Init(m_pServerConfig->MaxClientCount);

		//�κ�Ŵ����� �����Ѵ�.
		m_pLobbyMgr = std::make_unique<LobbyManager>();
		//�κ�Ŵ����� �ʱ�ȭ�Ѵ�.
		//�����������κ��� ������ ��´�.
		//��Ʈ��ũ�� �ΰŸ� �����Ѵ�.
		m_pLobbyMgr->Init({ m_pServerConfig->MaxLobbyCount, 
							m_pServerConfig->MaxLobbyUserCount,
							m_pServerConfig->MaxRoomCountByLobby, 
							m_pServerConfig->MaxRoomUserCount },
						m_pNetwork.get(), m_pLogger.get());

		//��Ŷ���μ����� �����Ѵ�
		m_pPacketProc = std::make_unique<PacketProcess>();
		//��Ŷ���μ����� �ʱ�ȭ�Ѵ�.
		//��� ������ �����Ѵ�.
		m_pPacketProc->Init(m_pNetwork.get(), 
							m_pUserMgr.get(), 
							m_pLobbyMgr.get(), 
							m_pLogger.get());

		//�۵� ���θ� �ʱ�ȭ�Ѵ�.
		m_IsRun = true;

		//Main-�ʱ�ȭ �Ϸ�
		m_pLogger->Write(LOG_TYPE::L_INFO, "%s | Version2.0 Init Complete. Server Run", __FUNCTION__);
		
		return ERROR_CODE::NONE;
	}

	void Main::Run()
	{
		//���� ���� ����
		while (m_IsRun)
		{
			//��Ʈ��ũ�� �����Ѵ�
			m_pNetwork->Run();

			//������ �ϸ� ���� ��Ŷ��
			//ó���� ���� �������� ��Ŷ��
			//��� ó���� ������
			//��Ŷ �б�-ó�� �۾��� �ݺ��Ѵ�
			while (true)
			{
				//ť���� ��Ŷ�� �ϳ� �о� �´�.
				auto packetInfo = m_pNetwork->GetPacketInfo();

				//ť�� ��Ŷ�� ��� ó���ߴ�
				if (packetInfo.PacketId == 0)
				{
					break;
				}
				else
				{
					//�о�� ��Ŷ�� ó���Ѵ�.
					//1. packet id -> packet function call
					//2. packet id -> packet data to functional data
					//3. process by function and send(copy data to a session's buffer)
					//4. return result
					m_pPacketProc->Process(packetInfo);
				}
			}

			//�����͵鿡 ���� ���¸� �˻��ϰ�
			//����� ���� �����͸� ó���Ѵ�.
			m_pPacketProc->StateCheck();
		}
	}

	void Main::Stop()
	{
		m_IsRun = false;
	}

	void Main::Release()
	{
		if (m_pNetwork)
			m_pNetwork->Release();
	}

	ERROR_CODE Main::LoadConfig()
	{
		m_pServerConfig = std::make_unique<NServerNetLib::ServerConfig>();
		
		wchar_t sPath[MAX_PATH] = { 0, };
		GetCurrentDirectory(MAX_PATH, sPath);

		wchar_t inipath[MAX_PATH] = { 0, };
		_snwprintf_s(inipath, _countof(inipath), _TRUNCATE, L"%s\\ServerConfig.ini", sPath);

		m_pServerConfig->Port = (unsigned short)GetPrivateProfileInt(L"Config", L"Port", 0, inipath);
		m_pServerConfig->BackLogCount = GetPrivateProfileInt(L"Config", L"BackLogCount", 0, inipath);
		m_pServerConfig->MaxClientCount = GetPrivateProfileInt(L"Config", L"MaxClientCount", 0, inipath);

		m_pServerConfig->MaxClientSockOptRecvBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSockOptRecvBufferSize", 0, inipath);
		m_pServerConfig->MaxClientSockOptSendBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSockOptSendBufferSize", 0, inipath);
		m_pServerConfig->MaxClientRecvBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientRecvBufferSize", 0, inipath);
		m_pServerConfig->MaxClientSendBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSendBufferSize", 0, inipath);

		m_pServerConfig->ExtraClientCount = GetPrivateProfileInt(L"Config", L"ExtraClientCount", 0, inipath);
		m_pServerConfig->MaxLobbyCount = GetPrivateProfileInt(L"Config", L"MaxLobbyCount", 0, inipath);
		m_pServerConfig->MaxLobbyUserCount = GetPrivateProfileInt(L"Config", L"MaxLobbyUserCount", 0, inipath);
		m_pServerConfig->MaxRoomCountByLobby = GetPrivateProfileInt(L"Config", L"MaxRoomCountByLobby", 0, inipath);
		m_pServerConfig->MaxRoomUserCount = GetPrivateProfileInt(L"Config", L"MaxRoomUserCount", 0, inipath);

		m_pLogger->Write(NServerNetLib::LOG_TYPE::L_INFO, "%s | Port(%d), Backlog(%d)", __FUNCTION__, m_pServerConfig->Port, m_pServerConfig->BackLogCount);
		
		return ERROR_CODE::NONE;
	}
}