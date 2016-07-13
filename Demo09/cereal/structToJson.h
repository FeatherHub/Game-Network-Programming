#pragma once
// http://qiita.com/Ushio@github/items/827cf026dcf74328efb7 , http://qiita.com/usagi/items/8e00f23b2508c98947b3 ���� �߽��ϴ�.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace Json
{
	struct NPCInfo
	{
		std::string Name = "test_npc";
		int HP = 102;

		template<class Archive>
		void serialize(Archive & archive)
		{
			archive(Name, HP);
		}
	};

	struct StructToJson1
	{
		NPCInfo TestNPC;

		void EncodingDecoding()
		{
			std::cout << "StructToJson1 - Encoding Start" << std::endl;

			// std::stringstream�� ����Ͽ� .str()�� ���ڿ��� ��� ���� �� cereal::JSONOutputArchive�� �ı� �Ǿ�� �Ѵ�
			// cereal �ø��������� RAII �̵���� ���ʷ� �ϰ� �־ �ı� �� ������ ����� �����ϰ� ���� ���� ���ɼ��� �ִ�
			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(TestNPC);
			}
			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson1 - Encoding End" << std::endl;
			std::cout << std::endl;

			std::cout << "StructToJson1 - Decoding Start" << std::endl;
			NPCInfo testNPC;
			cereal::JSONInputArchive i_archive(ss);
			i_archive(testNPC);

			std::cout << testNPC.Name << std::endl;
			std::cout << testNPC.HP << std::endl;
			std::cout << "StructToJson1 - Decoding End" << std::endl;
			std::cout << std::endl;
		}

	};


	struct StructToJson2
	{
		NPCInfo TestNPC;
		std::string EncodingData;

		void Encoding()
		{
			std::cout << "StructToJson2 - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(TestNPC);
			}
			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson2 - Encoding End" << std::endl;

			EncodingData = ss.str();

			std::cout << std::endl;
		}

		void Decoding()
		{
			std::cout << "StructToJson2 - Decoding Start" << std::endl;

			std::istringstream is(EncodingData);

			NPCInfo testNPC;
			cereal::JSONInputArchive i_archive(is);
			i_archive(testNPC);

			std::cout << testNPC.Name << std::endl;
			std::cout << testNPC.HP << std::endl;
			std::cout << "StructToJson2 - Decoding End" << std::endl;

			std::cout << std::endl;
		}

	};


	//  �̸� ���̱�
	struct NPCInfo2
	{
		std::string Name = "test_npc2";
		int HP = 202;

		template<class Archive>
		void serialize(Archive & archive)
		{
			archive(CEREAL_NVP(Name), CEREAL_NVP(HP));
		}
	};

	struct StructToJson3
	{
		NPCInfo2 TestNPC;

		void EncodingDecoding()
		{
			std::cout << "StructToJson3. key�� �̸� ���̱� - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(cereal::make_nvp("root", TestNPC));
			}
			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson3 - Encoding End" << std::endl;
			std::cout << std::endl;

			std::cout << "StructToJson3 - Decoding Start" << std::endl;
			NPCInfo2 testNPC;
			cereal::JSONInputArchive i_archive(ss);
			i_archive(cereal::make_nvp("root", testNPC));

			std::cout << testNPC.Name << std::endl;
			std::cout << testNPC.HP << std::endl;
			std::cout << "StructToJson3 - Decoding End" << std::endl;
			std::cout << std::endl;
		}
	};


	// ��ħ����. ���� �ҽ��� ���� ���� ���� 
	struct Vector2 {
		Vector2(float x, float y) { this->x = x; this->y = y; }

		float x = 0.0f;
		float y = 0.0f;
	};

	// �Ķ���� Ÿ���� �� �����̾�� �Ѵ�
	template<class Archive>
	void serialize(Archive& archive, Vector2& vec)
	{
		archive(cereal::make_nvp("x", vec.x), cereal::make_nvp("y", vec.y));
	}

	struct NPCInfo3
	{
		std::string Name = "test_npc3";
		int HP = 302;
		Vector2 pos = { 5.0f, 12.40f };

		template<class Archive>
		void serialize(Archive & archive)
		{
			archive(CEREAL_NVP(Name), CEREAL_NVP(HP), CEREAL_NVP(pos));
		}
	};

	struct StructToJson4
	{
		NPCInfo3 TestNPC;

		void EncodingDecoding()
		{
			std::cout << "StructToJson4. ��ħ���� - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(cereal::make_nvp("root", TestNPC));
			}
			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson4 - Encoding End" << std::endl;
			std::cout << std::endl;

			std::cout << "StructToJson4 - Decoding Start" << std::endl;
			NPCInfo3 testNPC;
			cereal::JSONInputArchive i_archive(ss);
			i_archive(cereal::make_nvp("root", testNPC));

			std::cout << testNPC.Name << std::endl;
			std::cout << testNPC.HP << std::endl;
			std::cout << "StructToJson4 - Decoding End" << std::endl;
			std::cout << std::endl;
		}
	};


	// STL �����̳� ��� 
	struct NPCInfo4
	{
		std::string Name = "test_npc3";
		int HP = 302;
		std::vector<int> Numbers{ 10, 11, 12 };

		template<class Archive>
		void serialize(Archive & archive)
		{
			archive(CEREAL_NVP(Name), CEREAL_NVP(HP), CEREAL_NVP(Numbers));
		}
	};

	struct StructToJson5
	{
		NPCInfo4 TestNPC;

		void EncodingDecoding()
		{
			std::cout << "StructToJson5. std::vector ��� - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(cereal::make_nvp("root", TestNPC));
			}
			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson5 - Encoding End" << std::endl;
			std::cout << std::endl;

			std::cout << "StructToJson5 - Decoding Start" << std::endl;
			NPCInfo4 testNPC;
			cereal::JSONInputArchive i_archive(ss);
			i_archive(cereal::make_nvp("root", testNPC));

			std::cout << testNPC.Name << std::endl;
			std::cout << testNPC.HP << std::endl;
			std::cout << "StructToJson5 - Decoding End" << std::endl;
			std::cout << std::endl;
		}
	};


	// UINT64
	struct StructToJson6
	{
		static void Encoding()
		{
			std::cout << "StructToJson6. UINT64 ��� - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(cereal::make_nvp("key", std::numeric_limits< std::uint64_t >::max()));
			}

			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson6 - Encoding End" << std::endl;
			std::cout << std::endl;
		}
	};


	// enum class
	struct StructToJson7
	{
		enum class NPCType : std::uint8_t
		{
			NONE
			, TYPE1
			, TYPE2
		};

		static void Encoding()
		{
			std::cout << "StructToJson7. enum class ��� - Encoding Start" << std::endl;

			std::stringstream ss;
			{
				cereal::JSONOutputArchive o_archive(ss);
				o_archive(cereal::make_nvp("NPCType", NPCType::TYPE1));
			}

			std::cout << ss.str() << std::endl;
			std::cout << "StructToJson7 - Encoding End" << std::endl;
			std::cout << std::endl;
		}
	};
}