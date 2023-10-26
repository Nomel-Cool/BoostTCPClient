#include "tcpLink.h"

int recv_msg_count = 0; // 临时变量，统计接收次数

TcpClient::TcpClient(void)
{
	m_uiSendBufferSize = 0;
	m_uiSendTimeout = 10000;
	m_uiRecvBufferSize = 0;
	m_uiRecvTimeout = 10000;
	m_pEndPoint = NULL;
	m_pSocket = NULL;
	m_nSyncRecviceDataSize = 0;
	m_pTimer = new deadline_timer(m_io);
	m_strReserved = "";
}

TcpClient::~TcpClient(void)
{
	delete m_pTc;
}

//设置参数
void TcpClient::SetClientParameter(unsigned int uiSendBufferSize, unsigned int uiSendTimeout, unsigned int uiRecvBufferSize, unsigned int uiRecvTimeout)
{
	m_uiSendBufferSize = uiSendBufferSize;
	m_uiSendTimeout = uiSendTimeout;
	m_uiRecvBufferSize = uiRecvBufferSize;
	m_uiRecvTimeout = uiRecvTimeout;
	if (m_uiRecvTimeout <= 0)
	{
		m_uiRecvTimeout = 10000;
	}
}

void TcpClient::SetAccordParameter(unsigned int uiHeaderFlag, const int& meta_segment, std::map<std::string, int>  factor)
{
	m_uiHeaderFlag = uiHeaderFlag;
	m_pTc = new type_castor(meta_segment);
	m_pTc->initAccord(factor);
}


//连接服务器(同步)
int	TcpClient::ConnectServer(char *pIp, unsigned short usPort, unsigned int uiConnectTimeout)
{
	if (pIp == NULL || usPort == 0)
		return -1;

	try
	{
		m_pEndPoint = new ip::tcp::endpoint(ip::address::from_string(pIp), usPort);
		m_pSocket = new ip::tcp::socket(m_io);
		m_pSocket->open(m_pEndPoint->protocol());
		//m_pSocket->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
		if (m_uiSendBufferSize != 0)
		{
			boost::asio::socket_base::send_buffer_size sendBufferSize(m_uiSendBufferSize);
			m_pSocket->set_option(sendBufferSize);
		}
		if (m_uiRecvBufferSize != 0)
		{
			boost::asio::socket_base::receive_buffer_size recvBufferSize(m_uiRecvBufferSize);
			m_pSocket->set_option(recvBufferSize);
		}
		//connect
		m_pSocket->connect(*m_pEndPoint);
		/*
		char str[1024];
		sock.read_some(buffer(str));
		std::cout << "receive from" << sock.remote_endpoint().address() << std::endl;;
		std::cout << str << std::endl;
		*/
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -2;
	}
	return 0;
}

//连接服务器(异步)
int	TcpClient::ConnectServerByAynsc(char *pIp, unsigned short usPort, unsigned int uiConnectTimeout, unsigned int uiReconnectInteralTime)
{
	if (pIp == NULL || usPort == 0)
		return -1;

	try
	{
		m_pEndPoint = new ip::tcp::endpoint(ip::address::from_string(pIp), usPort);
		m_pSocket = new ip::tcp::socket(m_io);
		m_pSocket->open(m_pEndPoint->protocol());
		//m_pSocket->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
		if (m_uiSendBufferSize != 0)
		{
			boost::asio::socket_base::send_buffer_size sendBufferSize(m_uiSendBufferSize);
			m_pSocket->set_option(sendBufferSize);
		}
		if (m_uiRecvBufferSize != 0)
		{
			boost::asio::socket_base::receive_buffer_size recvBufferSize(m_uiRecvBufferSize);
			m_pSocket->set_option(recvBufferSize);
		}
		//connect
		m_pSocket->async_connect(*m_pEndPoint, boost::bind(&TcpClient::connect_handler, this, boost::asio::placeholders::error));
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -2;
	}

	return 0;
}

void TcpClient::connect_handler(const boost::system::error_code& ec)
{
	if (ec)
	{
		return;
	}

	std::cout << "receive from:" << m_pSocket->remote_endpoint().address() << std::endl;
}

void TcpClient::async_read_handler(const boost::system::error_code& ec, size_t bytes_transferred, TcpRecvDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2)
{
	//回调数据
	if (fnCallback != NULL)
	{
		fnCallback(ec, m_rbTempRecvBuffer.data(), bytes_transferred, dwUserData1, dwUserData2);
	}

	if (ec == boost::asio::error::eof)
	{
		//对端方关闭连接
		if (m_pSocket->is_open())
		{
			m_pSocket->close();
		}
		//printf("close connect \n");
		return;
	}
	if (!ec)
	{
		//发送数据失败
		return;
	}

	//接收下一条数据
	m_pSocket->async_read_some(boost::asio::buffer(m_rbTempRecvBuffer),
		boost::bind(&TcpClient::async_read_handler, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			fnCallback, dwUserData1, dwUserData2));
}

//关闭连接
void TcpClient::CloseConnect()
{
	if (m_pSocket != NULL)
	{
		m_pSocket->close();
		m_pSocket = NULL;
	}
}


//发送数据
int	TcpClient::SendData(const char *pBuffer, int nBufferSize)
{
	int nRet = 0;
	if (m_pSocket != NULL)
	{
		nRet = m_pSocket->send(buffer(pBuffer, nBufferSize));
	}

	return nRet;
}

//接收数据
int	TcpClient::RecvData(char *pBuffer, int nBufferSize)
{
	int nRet = 0;
	if (m_pSocket != NULL)
	{
		m_nSyncRecviceDataSize = 0;
		boost::system::error_code ec;
		m_pSocket->async_read_some(buffer(pBuffer, nBufferSize), boost::bind(&TcpClient::read_hander, this, pBuffer, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error));
		m_pTimer->expires_from_now(boost::posix_time::seconds(m_uiRecvTimeout));
		m_pTimer->async_wait(boost::bind(&TcpClient::RecvDataTimeoutProcess, this));
		m_io.reset();
		m_io.run(ec);
		nRet = m_nSyncRecviceDataSize;
		m_strReserved += std::string(pBuffer, nRet);
	}

	return nRet;
}

void TcpClient::read_hander(char *pBuffer, size_t bytes_transferred, const boost::system::error_code& err)
{
	if (err)
	{
		return;
	}

	m_nSyncRecviceDataSize = bytes_transferred;
	m_total_recieve_bytes += bytes_transferred;
	m_pTimer->cancel();
	boost::posix_time::ptime timeStamp = boost::posix_time::microsec_clock::local_time();
	m_recv_time_recorder.insert(std::make_pair(m_total_recieve_bytes - 1 + m_nSyncRecviceDataSize, std::move(timeStamp)));
	//std::cout << "信息时间戳: " << timeStamp << "     第" << m_strReserved.size() - 1 + m_nSyncRecviceDataSize << "段" << std::endl;
}

void TcpClient::RecvDataTimeoutProcess()
{
	int n = 0;
}

int TcpClient::AttempRead(char * pBuffer, int nBufferSize)
{
	int nRet = -1;
	if (m_pSocket != NULL)
	{
		m_nSyncRecviceDataSize = 0;
		boost::system::error_code ec;
		m_pSocket->async_read_some(buffer(pBuffer, nBufferSize), boost::bind(&TcpClient::read_hander, this, pBuffer, boost::asio::placeholders::bytes_transferred, boost::asio::placeholders::error));
		m_pTimer->expires_from_now(boost::posix_time::seconds(m_uiRecvTimeout));
		m_pTimer->async_wait(boost::bind(&TcpClient::RecvDataTimeoutProcess, this));
		m_io.reset();
		m_io.run(ec);
		nRet = m_nSyncRecviceDataSize;
		std::string strData = std::string(pBuffer, nRet);

		/* Time Stamp Insert */


		if (((uint8_t)strData[0] ^ m_uiHeaderFlag) == 0)
		{
			// storage directly
			m_strReserved += strData;
		}
		else
		{
			// 如果读取的数据的开头并不是包头，那么就需要往后找到包头并截断了
			for (int cut_pos = 1; cut_pos < nRet; cut_pos++)
			{
				if (((uint8_t)strData[cut_pos] ^ m_uiHeaderFlag) == 0)
				{
					strData = strData.substr(cut_pos, strData.size() - cut_pos);
					// cut then storage
					m_strReserved += strData;
					break;
				}
			}
		}
		// 返回真实裁剪过后的长度
		nRet = m_strReserved.size();
		m_total_consume_bytes += m_nSyncRecviceDataSize - nRet;
	}
	return nRet;
}

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
//			s += " ";
//		}
//		std::cout << piece.first << " : " << s << std::endl;
//		s.clear();
//	}
//}

void TcpClient::parsing_thread()
{
	bool first_read_flag = false; // 如果没有找到下一个包头或者该包不合法，就不让m_strReserved变化
	// 在这里已经保证了strData的开头一定是FA了,所以从第二个字节开始找
	int pre_pos = 0, cut_pos;
	// 解析data...，把一个块按headerFlag切割成更小的包，然后传给typeHandler封装,只有当遇到下一个FA时才认为当前包读取完毕了
	for (cut_pos = pre_pos + 1; cut_pos < m_strReserved.size(); cut_pos++)
	{
		if (((uint8_t)m_strReserved[cut_pos] ^ m_uiHeaderFlag) == 0)
		{
			std::string segment_data = m_strReserved.substr(pre_pos, cut_pos - pre_pos);
			// you should check if the package is legal.Otherwise,the user should not build the struct.
			int verify_package = m_pTc->isLegal(segment_data, m_uiHeaderFlag);
			if (verify_package == VERIFY_CORRECT)
			{
				// cut then process handling
				pre_pos = cut_pos;
				specific_package package;
				auto source = m_pTc->buildStruct(segment_data.size(), segment_data.data());
				//print_struct(source);
				package.source = source;
				package.package_type_info = "1004_package_ID";
				package.package_data_info = "1006_data_flag";
				package.time_stamp = time_detect(m_total_consume_bytes); 
				m_pTc->readSpecificBytes(package);
				first_read_flag = true;
			}
			else
			{
				switch (verify_package)
				{
				case VERIFY_HEADER_MISMATCH:
				{
					std::cout << "找不到包头" << std::endl;
					break;
				}
				case VERIFY_LESS_THAN_RANGE:
				{
					std::cout << "超出范围，应该请求下位机重发" << std::endl;
					break;
				}
				case VERIFY_OUT_OF_RANGE:
				{
					//std::cout << "提前截取" << std::endl;
					break;
				}
				case VERIFY_SUM_MISMATCH:
				{
					std::cout << "校验和不匹配" << std::endl;
					break;
				}
				default:
					break;
				};
			}
		}
	}
	// reserved_data从pre_pos 处开始截断,也保证下次第一个是FA
	if (first_read_flag)
	{
		m_strReserved = m_strReserved.substr(pre_pos, m_strReserved.size() - pre_pos);
		m_total_consume_bytes += pre_pos; // pre_pos已经是裁剪后的相对值了
		first_read_flag = false; //正确解包后，把标志置为重新读，由于校验错误不会损耗数据，所以会
	}
}

boost::posix_time::ptime TcpClient::time_detect(size_t key)
{
	auto it_up = m_recv_time_recorder.upper_bound(key);
	if (it_up != m_recv_time_recorder.end()) {
		return it_up->second;
	}
	else {
		std::cout << "No element found" << std::endl;
		return boost::posix_time::not_a_date_time;
	}
}


//接收数据(阻塞)
int	TcpClient::RecvDataByBlock(char *pBuffer, int nBufferSize)
{
	int nRet = 0;
	if (m_pSocket != NULL)
	{
		m_nSyncRecviceDataSize = 0;
		boost::system::error_code ec;
		m_pSocket->receive(buffer(pBuffer, nBufferSize));
	}

	return nRet;
}

//发送数据(异步)
int		TcpClient::SendDataByAynsc(char *pBuffer, int nBufferSize, TcpSendDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2)
{
	if (pBuffer == NULL || nBufferSize == 0)
		return -1;

	if (m_pSocket == NULL || !m_pSocket->is_open())
		return -1;

	boost::asio::async_write(
		*m_pSocket,
		boost::asio::buffer(pBuffer, nBufferSize),
		boost::bind(&TcpClient::write_handler, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			fnCallback, dwUserData1, dwUserData2));

	return 0;
}

//接收数据(异步)
int	TcpClient::RecvDataByAynsc(TcpRecvDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2)
{
	if (m_pSocket == NULL || !m_pSocket->is_open())
		return -1;

	m_pSocket->async_read_some(buffer(m_rbTempRecvBuffer), boost::bind(&TcpClient::async_read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, fnCallback, dwUserData1, dwUserData2));

	return 0;
}

//返回当前堆积数据
std::string TcpClient::GetReservedData()
{
	return m_strReserved;
}

//设置阻塞与非阻塞
void TcpClient::SetNoBlock(bool bNoBlock)
{
	if (m_pSocket == NULL)
		return;

	if (bNoBlock)
	{
		//boost::asio::ip::tcp::socket::non_blocking_io io_option(true);
		//m_pSocket->io_control(io_option);

		// 版本问题
		m_pSocket->non_blocking(true);
	}
	else
	{
		//阻塞
		//boost::asio::ip::tcp::socket::non_blocking_io io_option(false);
		//m_pSocket->io_control(io_option);
		m_pSocket->non_blocking(false);
	}
}

void TcpClient::write_handler(const boost::system::error_code& error, size_t bytes_transferred, TcpSendDataCallback fnCallback, DWORD dwUserData1, DWORD dwUserData2)
{
	if (fnCallback != NULL)
	{
		fnCallback(error, bytes_transferred, dwUserData1, dwUserData2);
	}
	if (error == boost::asio::error::eof)
	{
		//对端方关闭连接
		if (m_pSocket->is_open())
		{
			m_pSocket->close();
		}
		//printf("close connect \n");
		return;
	}
	if (!error)
	{
		//发送数据失败
		return;
	}

#ifdef _DEBUG
	//写数据
	printf("write data!!!\n");
#endif
}