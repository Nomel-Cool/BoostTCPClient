// lexcast.cpp -- simple cast from float to string

#include <iostream>
#include <string>
#include <bitset>

//library for test
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "tcpLink.h"
using namespace std;

//// support method , to be used inside QML control
//void print_struct(std::map<std::string, std::vector<std::bitset<8> > > source)
//{
//	size_t len = 0;
//	std::string s = "";
//	for (const auto& piece : source)
//	{
//		for (int i = 0; i < piece.second.size(); i++)
//		{
//			for (int j = piece.second[i].size() - 1; j >= 0; j--)
//			{
//				s += piece.second[i][j] == 1 ? "1" : "0";
//			}
//		}
//		std::cout << piece.first << " : " << s << std::endl;
//		s.clear();
//	}
//}

int main()
{
	std::map<std::string, int> factor{ {"1000_header",1}, {"1001_length_flag",1}, {"1002_param_type_flag",1}, {"1003_package_type_flag",1}, {"1004_package_ID",1} , {"1005_sequence_flag",4}, {"1006_data_flag",0}, {"1007_verify_flag",1} };
	uint8_t headerFlag = 0xFA;
	char recvBuffer[1024];
	TcpClient tcp_client;
	tcp_client.SetClientParameter(1024, 1000, 1024, 1000);
	tcp_client.SetAccordParameter(headerFlag, 8, factor);
	tcp_client.SetNoBlock(true);
	//tcp_client.ConnectServer((char*)"192.168.129.1", 23, 1000);
	tcp_client.ConnectServer((char*)"192.168.0.88", 23, 1000);

	int ret = -1;
	// 试读，然后马上处理，必须
	while(ret == -1)
	{
		ret = tcp_client.AttempRead(recvBuffer, 1024);
	} 
	//std::string data = tcp_client.GetReservedData();
	tcp_client.parsing_thread();

	while (true)
	{
		int recvLen = tcp_client.RecvData(recvBuffer, 1024);
		tcp_client.parsing_thread();
	}

	//boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
	//my_function();
	//boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
	//boost::posix_time::time_duration duration = end - start;
	//std::cout << "函数执行时间: " << duration.total_microseconds() << " 微秒" << std::endl;

	system("pause");
	return 0;
}

 