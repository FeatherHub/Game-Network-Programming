#pragma once

#include <chrono>

namespace NLogicLib
{
	enum class GameState 
	{
		NONE,
		STARTING, //��û�ϰ� ���� �غ� ��
		PLAYING,
		END //���â �����ִ� ���
	};

	class Game
	{
	public:
		Game();
		virtual ~Game(); //Unit test�� ����

		void Clear();

		GameState GetState() { return m_state; }

		void SetState(const GameState state) { m_state = state; }

		bool CheckSelectTime();
	private:
		GameState m_state = GameState::NONE;

		//��밡 ���� ���ŵȴ�
		//�� �� �ð��� ������ ���º�
		//�ð��� ������ ���� �Ѿ��
		std::chrono::system_clock::time_point m_selectTime 
			= std::chrono::system_clock::now();

		int m_select1; //�� 0 �� 1 �� 2 �� 3
		int m_select2;
	};
}