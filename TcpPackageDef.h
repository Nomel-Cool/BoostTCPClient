#pragma once
#ifndef TCPPACKAGEDEF_H
#define TCPPACKAGEDEF_H
#include <bitset>
#include <vector>
#include <string>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>


struct specific_package 
{
	std::map<std::string, std::vector<std::bitset<8> > > source; // 封装好的信息结构
	std::string package_type_info; // 包类型key值
	std::string package_data_info; // 指定读取的段key值
	boost::posix_time::ptime time_stamp; // 当前信息的时间戳
};

enum PACKAGE_TYPE
{
	GENERIC_COMMAND_RESPONSE_PACKAGE = (uint8_t)0x80,
	POWER_UP_HANDSHAKE_REQUEST_DATA_PACKAGE,
	MODULE_INFORMATION_RESPONSE_DATA_PACKAGE,
	MODULE_STATUS_ANSWER_DATA_PACKAGE,
	HEART_BEAT_PACKAGE = (uint8_t)0x90,
	BREATH_PACKAGE,
	ECG_PACKAGE,
	CARDIAC_CHANNEL_OVERLOAD_PACKAGE,
	HEART_RATE_PACKAGE,
	ARRHYTHMIA_ANALYSIS_PACKAGE,
	ARRHYTHMIA_ANALYSIS_RESULT_PACKAGE,
	ARRHYTHMIA_ANALYSIS_STATE_PACKAGE,
	CARDIAC_ST_VALUE_PACKAGE,
	CARDIAC_ST_TEMPLATE_PACKAGE,
	RESPIRATORY_ASPHYXIA_PACKAGE,
	BREATH_CVA_FLAG_PACKAGE,
	PVC_STATISTIC_PACKAGE,
	TEMPERATURE_PACKAGE = (uint8_t)0xB0
};

enum VERIFY_ERROR_TYPE
{
	VERIFY_CORRECT = 0,
	VERIFY_OUT_OF_RANGE,
	VERIFY_LESS_THAN_RANGE,
	VERIFY_HEADER_MISMATCH,
	VERIFY_SUM_MISMATCH
};

extern std::vector< std::vector<int> > vec_package_eigen;

#endif