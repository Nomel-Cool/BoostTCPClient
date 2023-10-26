#include "typeHandler.h"			


std::vector<std::vector<int>> vec_package_eigen;
int heart_beat_count = 0; // 临时变量，测试心跳

type_castor::type_castor(const int& meta_seg)
{
	meta_segment = meta_seg;
	prefix_segment = 1000;

	vec_package_eigen.resize(49, { 0 });
	vec_package_eigen[16] = { 0 };
	vec_package_eigen[vec_package_eigen.size() - 1] = { 2,3 };
}

type_castor::~type_castor()
{

}

bool type_castor::segment_length_check(const int & data_len)
{
	return data_len % meta_segment ? true : false;
}

std::vector<std::bitset<8> > type_castor::chars2Bits(int len, const char* uData)
{
	std::vector<std::bitset<8>> bits_ret;
	for (int i = 0; i < len; i++)
	{
		//if (uData[i] == '\0')
		//{
		//	std::cout << "ZERO" << std::endl;
		//}
		std::bitset<8> bin(uData[i]);
		bits_ret.push_back(std::move(bin));
	}
	return bits_ret;
}

void type_castor::initAccord(std::map<std::string, int> fator)
{
	accordMap.clear();
	accordMap = fator;
	auto it_accord_header = accordMap.begin();
	while (it_accord_header != accordMap.end())
	{
		if (it_accord_header->second != 0)
		{
			total_fix_length += it_accord_header->second;
		}
		it_accord_header++;
	}
	prefix_segment += fator.size() - 1; // start from 1000
}

void type_castor::initAccord(std::string segName, int segLen)
{
	std::pair<std::string, int> p(std::to_string(prefix_segment) + "_" + segName, segLen);
	prefix_segment++;
	accordMap.insert(p);
	if (segLen != 0)
	{
		total_fix_length += segLen;
	}
}

//不同的协议应当重载该函数（通过继承m_pTc的类来实现），从而更改判别包是否合法
int type_castor::isLegal(std::string strData, const unsigned int& uiHeaderFlag)
{
	//auto d = (uint8_t)strData[0];
	if (((uint8_t)strData[0] ^ uiHeaderFlag) != 0)
		return VERIFY_HEADER_MISMATCH;
	if ((int)strData[1] < strData.size())
		return VERIFY_LESS_THAN_RANGE;
	if ((int)strData[1] > strData.size())
		return VERIFY_OUT_OF_RANGE;
	uint8_t verify_sum = 0x00;
	for (int i = 1; i < strData.size() - 1; i++)
	{
		verify_sum += (int)strData[i];
	}
	auto test = (int)strData[strData.size() - 1];
	if ((verify_sum ^ (uint8_t)strData[strData.size() - 1]) != 0)
		return VERIFY_SUM_MISMATCH;
	return VERIFY_CORRECT;
}

bool type_castor::UnknownLengthSegmentDeduction(std::map<std::string, int>::iterator& it_cur_header, std::string origin_data, std::map<std::string, std::vector<std::bitset<8> > >& ret_struct, int& offset, size_t total_length, size_t fix_length)
{
	size_t total_unfix_length = total_length - fix_length;
	std::string strTemp = origin_data.substr(offset, total_unfix_length);
	std::vector<std::bitset<8> > v_Temp = chars2Bits(strTemp.size(), strTemp.data());
	ret_struct.insert(std::pair<std::string, std::vector<std::bitset<8> > >(it_cur_header->first, std::move(v_Temp)));
	offset += total_unfix_length;
	it_cur_header++;
	return true;
}

// support method , to be used inside QML control
void print_struct(std::map<std::string, std::vector<std::bitset<8> > > source)
{
	size_t len = 0;
	std::string s = "";
	for (const auto& piece : source)
	{
		for (int i = 0; i < piece.second.size(); i++)
		{
			for (int j = piece.second[i].size() - 1; j >= 0; j--)
			{
				s += piece.second[i][j] == 1 ? "1" : "0";
			}
		}
		std::cout << piece.first << " : " << s << std::endl;
		s.clear();
	}
}

inline void temperature_handler(std::vector<int> eigens, std::vector<std::bitset<8> > package_data)
{
	// 保证数据包不为空才去读数组，否则会越界
	if (!package_data.empty())
	{
		std::string s_out = "";
		for (auto& eigen_pos : eigens)
		{
			s_out += package_data[eigen_pos].to_string();
		}
		std::cout << "Temperature_package  " << s_out << std::endl;
	}
}

inline void heartbeat_handler(std::vector<int> eigens, std::vector<std::bitset<8> > package_data, boost::posix_time::ptime package_time)
{
	// 保证数据包不为空才去读数组，否则会越界
	if (!package_data.empty())
	{
		std::string s_out = "";
		for (auto& eigen_pos : eigens)
		{
			s_out += package_data[eigen_pos].to_string();
		}
		if (s_out[3] == '1')
		{
			std::cout << "Heartbeat_package  " << s_out << "  " << ++heart_beat_count << "  " << package_time << std::endl;
		}
	}
}

bool type_castor::readSpecificBytes(specific_package package)
{
	try
	{
		// 先判断包的类型，再采用对应的函数来处理
		auto package_type = package.source[package.package_type_info];
		auto package_data = package.source[package.package_data_info];
		// 这里默认当前协议的包类型由一个字节指定

		//获取当前包的特征量
		auto test = package_type[0].to_ulong() % 0x80;
		if (test < 0 || test > vec_package_eigen.size() - 1)
		{
			print_struct(package.source);
		}
		auto eigens = vec_package_eigen[package_type[0].to_ulong() % 0x80];

		//// 调取对应的函数处理（这里先用输出替代）
		//std::string s_out = "TYPE: " + package_type[0].to_string() + "  EIGEN_DATA: ";
		//if (!package_data.empty()) //数据段可能为空
		//{
		//	for (auto& eigen_pos : eigens)
		//	{
		//		s_out += package_data[eigen_pos].to_string();
		//	}
		//}
		//std::cout << s_out << std::endl;

		auto temperature_type = std::bitset<8>(TEMPERATURE_PACKAGE);
		auto heartbeat_type = std::bitset<8>(HEART_BEAT_PACKAGE);
		// 保证数据包不为空才去读数组，否则会越界
		if ((package_type[0] ^ temperature_type) == 0)
		{
			temperature_handler(eigens, package_data);
		}
		if ((package_type[0] ^ heartbeat_type) == 0)
		{
			heartbeat_handler(eigens, package_data, package.time_stamp);
		}
		//s_out.clear();
		return true;
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return false;
	}
}

std::map<std::string, std::vector<std::bitset<8> > > type_castor::buildStruct(int len, const char* data)
{
	if (accordMap.size() == 0 || accordMap.find("1000_header") == accordMap.end())
	{
		// 用户没有正确初始化协议
		return std::map<std::string, std::vector<std::bitset<8> > >();
	}
	// 此处应该还要判断数据长度是否是meta_segment的倍数
	std::string strData(data,len);
	//std::cout << "LEN: " << strData.size() << std::endl;
	int offset = 0;
	auto it_accord_header = accordMap.begin();
	std::map<std::string, std::vector<std::bitset<8> > > ret_struct;
	while (it_accord_header != accordMap.end())
	{
		if (it_accord_header->second != 0) // 如果当前字段为定长字段，则直接转成比特流存起来
		{
			std::string strTemp = strData.substr(offset, it_accord_header->second);
			std::vector<std::bitset<8> > v_Temp = chars2Bits(strTemp.size(),strTemp.data());
			ret_struct.insert(std::pair<std::string, std::vector<std::bitset<8> > >(it_accord_header->first, std::move(v_Temp)));
			offset += it_accord_header->second;
			it_accord_header++;
		}
		else 
		{
			/*
			  * 若当前字段为不定长字段，需要进行推断，这里信息不够，可以通过继承这个类提供更多推断规则并重载这个函数以达到推断多个不定长字段的效果
			  * 此处应该抽象为一个可供重载的deduction函数，以供适用于各种协议的不定长字段的推导
			  * 全长 - 全定长 = 不定长，仅适用于单个不定长字段
			  */
			UnknownLengthSegmentDeduction(it_accord_header, strData, ret_struct, offset, len, total_fix_length);
		}
	}
	return ret_struct;
}