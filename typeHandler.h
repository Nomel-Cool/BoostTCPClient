#pragma once
#ifndef TYPE_HANDLER_H
#define TYPE_HANDLER_H

#include <vector>
#include <boost/container/vector.hpp>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <map>
#include <boost/bimap/bimap.hpp>
#include <bitset>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <functional>
#include <boost/function.hpp>

#include "TcpPackageDef.h"

// 以下的类都是为了服务于一个抽象的通讯协议

class type_castor
{
public:
	type_castor(const int& meta_seg);
	~type_castor();
public:
	bool segment_length_check(const int& data_len); // 检查data的长度是否是meta_segment的倍数
	std::vector<std::bitset<8> > chars2Bits(int len, const char* uData); // 把字符串转换成 8N长的比特流 

	void initAccord(std::map<std::string, int> factor); // （按顺序）批量记录协议中 <字段名,字段长>，若该字段为不定长，则字段长为0。而且必须存在一个且只有一个名为header的字段 （Map容器保证了这一点）
	void initAccord(std::string segNmae, int segLen); // 重载函数，用于协议补充

	std::map<std::string, std::vector<std::bitset<8> > > buildStruct(int len, const char* data); // 若用户已经正确注册协议，调用该函数将生成一个键值对组成map，用于描述具体的每个字段的值

	// 判断该包是否符合协议
	int isLegal(std::string strData, const unsigned int& uiHeaderFlag);

	// 不定长字段的推断函数
	bool UnknownLengthSegmentDeduction(std::map<std::string, int>::iterator& it_cur_header, std::string origin_data, std::map<std::string, std::vector<std::bitset<8> > >& ret_struct, int& offset, size_t total_length, size_t fix_length);

	// 读取指定字段的多个字节
	bool readSpecificBytes(specific_package package);

private:
	int meta_segment; // 单位元数据长度，一般为一个字节
	int total_fix_length; // 所有定长字段的长度之和
	int total_unfix_length; // 所有不定长字段的长度之和
	int prefix_segment; // 字段前缀，用于map的迭代
	std::map<std::string, int> accordMap; // 一个对象维护一个协议
};

#endif // !TYPE_HANDLER_H
