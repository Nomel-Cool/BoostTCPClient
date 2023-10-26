#pragma once
#ifndef TCPLINK_H
#define TCPLINK_H

#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include "typeHandler.h"

using namespace boost::asio;

#define			TCP_RECV_DATA_PACKAGE_MAX_LENGTH				1024

//接收数据类型
enum RecvDataType
{
	RecvDataType_NoKnown = 0x00,			//未知
	RecvDataType_Head = 0x01,			//头数据
	RecvDataType_Body = 0x02,			//体数据
	RecvDataType_Some = 0x03,			//部分数据
	RecvDataType_OnePackage = 0x04,			//整包数据
};

//发送数据回调函数
typedef void (CALLBACK *TcpSendDataCallback)(const boost::system::error_code& error, std::size_t bytes_transferred, DWORD dwUserData1, DWORD dwUserData2);

//接收数据回调函数
typedef void (CALLBACK *TcpRecvDataCallback)(const boost::system::error_code& error, char *pData, int nDataSize, DWORD dwUserData1, DWORD dwUserData2);

class TcpClient
{
public:
	TcpClient(void);
	virtual ~TcpClient(void);

	//设置客户端参数
	void SetClientParameter(unsigned int uiSendBufferSize, unsigned int uiSendTimeout, unsigned int uiRecvBufferSize, unsigned int uiRecvTimeout);

	//设置协议识别参数
	void SetAccordParameter(unsigned int uiHeaderFlag, const int& meta_segment, std::map<std::string, int> factor);

	//设置阻塞与非阻塞
	void SetNoBlock(bool bNoBlock);

	//连接服务器(同步)
	int	ConnectServer(char *pIp, unsigned short usPort, unsigned int uiConnectTimeout);

	//连接服务器(异步)
	int	ConnectServerByAynsc(char *pIp, unsigned short usPort, unsigned int uiConnectTimeout, unsigned int uiReconnectInteralTime);

	//关闭连接
	void CloseConnect();

	//发送数据
	int	SendData(const char *pBuffer, int nBufferSize);

	//接收数据
	int	RecvData(char *pBuffer, int nBufferSize);

	//接收数据(阻塞)
	int	RecvDataByBlock(char *pBuffer, int nBufferSize);

	//发送数据(异步)
	int	SendDataByAynsc(char *pBuffer, int nBufferSize, TcpSendDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2);

	//接收数据(异步)
	int	RecvDataByAynsc(TcpRecvDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2);

	// 获取当前堆积信息
	std::string GetReservedData();

	//试读，用于消除第一次读入时剔除非包头开始部分
	int AttempRead(char* pBuffer, int nBufferSize);

	// 拆包
	void parsing_thread();

	// 时间戳管理
	boost::posix_time::ptime time_detect(size_t pos);

protected:
	void connect_handler(const boost::system::error_code& ec);
	void async_read_handler(const boost::system::error_code& ec, size_t bytes_transferred, TcpRecvDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2);
	void read_hander(char *pBuffer, size_t bytes_transferred, const boost::system::error_code& err);
	void write_handler(const boost::system::error_code& error, size_t bytes_transferred, TcpSendDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2);

	void RecvDataTimeoutProcess();

	io_service m_io;
	ip::tcp::endpoint * m_pEndPoint;
	ip::tcp::socket * m_pSocket;
	boost::array<char, TCP_RECV_DATA_PACKAGE_MAX_LENGTH> m_rbTempRecvBuffer;		//临时接收数据缓冲区

	int m_nSyncRecviceDataSize;				//同步接收数据大小

	unsigned int m_uiSendBufferSize;
	unsigned int m_uiSendTimeout;
	unsigned int m_uiRecvBufferSize;
	unsigned int m_uiRecvTimeout;

	std::string m_strReserved; // 当前滞留的所有数据

	size_t m_total_recieve_bytes; // 当前全部收到的字节

	size_t m_total_consume_bytes; // 当前已经处理的字节

	unsigned int m_uiHeaderFlag; // 包头标志，需要继承来修改该截取多少位用于识别

	// 协议识别属性
	type_castor * m_pTc;

	//时间戳管理
	std::map<size_t, boost::posix_time::ptime> m_recv_time_recorder;

	deadline_timer * m_pTimer;
};

#endif