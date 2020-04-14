#pragma once
#define WIN32_LEAD_AND_MEAD
#include "MessageHeader.hpp"
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
class EasyTcpServer
{
private:
	SOCKET socketSrc;
	vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		socketSrc = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
	}
	//初始化socket
	SOCKET initSocket()
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		//1.建立一个socket套接字
		socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//1.建立一个socket;
		if (INVALID_SOCKET == socketSrc)
		{
			printf("<socket = %d>关闭旧连接\n", socketSrc);
			Close();
		}
		socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socketSrc == INVALID_SOCKET)
		{
			printf("创建失败\n");
		}
		else {
			printf("创建成功\n");
		}
		return socketSrc;
	}


	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port )
	{
		if (INVALID_SOCKET == socketSrc)
		{
			initSocket();
			 
		}
		
		//2.bind绑定用于接受客户端连接的网络端口
		SOCKADDR_IN addrSrc;
		addrSrc.sin_family = AF_INET;//用于网络连接的ipv4类型
		addrSrc.sin_port = htons(port);//host to net unsigned short
		
		if (ip)
		{
			addrSrc.sin_addr.S_un.S_addr = inet_addr(ip);//intenet_address
		}
		else {
			addrSrc.sin_addr.S_un.S_addr = INADDR_ANY;//intenet_address

		}
		//127.0.0.1:限制外网访问，如果在196.128..可以在内网访问；
		//INADDR_ANY:随便一个IP地址，所有网络都可以访问，不限定哪一个网络
		int ret = bind(socketSrc, (SOCKADDR*)&addrSrc, sizeof(SOCKADDR));
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR:绑定<%d>端口失败\n",port);
		}
		else {
			printf("绑定端口<%d>成功.\n",port);
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		//3.listen 监听网络接口

		if (SOCKET_ERROR == listen(socketSrc, n))//监听网络接口,最大5个人同时链接;
		{
			printf("Socket = <%d>监听网络接口失败\n",socketSrc);
		}
		else {
			printf("Socket = <%d>监听网络接口成功\n", socketSrc);
		}
		return 0;
	}
	//接受客户端连接
	SOCKET Accept()
	{
		//4.accept等待接受客户端链接
		SOCKADDR_IN addrClient;
		//addrClient.sin_family = AF_INET;
		int len = sizeof(addrClient);
		char msgBuf[] = "hello,I'm Server.";
		SOCKET sockClient = INVALID_SOCKET;//初始化无效的socket地址
		sockClient = accept(socketSrc, (SOCKADDR*)&addrClient, &len);
		if (sockClient == INVALID_SOCKET)
		{
			printf("Socket = <%d>错误:接受到无效客户端socket...\n", socketSrc);
		}
		else
		{
			NewUserJoin userJoin;
			sendDataToAll(&userJoin);
			g_clients.push_back(sockClient);
			printf("Socket = <%d>新客户端加入:socket= %d,IP=%s\n", (int)sockClient,(int)sockClient, inet_ntoa(addrClient.sin_addr));
		}
		return sockClient;
		
	}
	//关闭socket
	void Close()
	{
		if (socketSrc != INVALID_SOCKET)
		{
			//关闭环境
			//4.关闭套接字
			closesocket(socketSrc);
			socketSrc = INVALID_SOCKET;
			WSACleanup();
		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利 socket集合 描述符
			fd_set fdReader;
			fd_set fdWriter;
			fd_set fdExp;

			FD_ZERO(&fdReader);
			FD_ZERO(&fdWriter);
			FD_ZERO(&fdExp);

			FD_SET(socketSrc, &fdReader);
			FD_SET(socketSrc, &fdWriter);
			FD_SET(socketSrc, &fdExp);
			//printf("%d", g_clients.size());
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdReader);
			}

			//nfds 是一个整数值,是指fd_set集合中所有描述符(socket)的范围,而不是数量.
			//即是所有文件描述符最大值+1,在windows中这个参数可以写0
			timeval t = { 1,0 };
			int ret = select(socketSrc + 1, &fdReader, &fdWriter, &fdExp, &t);
			/*if (ret == 0)
			{
				continue;
			}*/

			if (ret < 0)
			{
				printf("select任务结束\n");
				Close();
				return false;
			}
			if (FD_ISSET(socketSrc, &fdReader))
			{
				FD_CLR(socketSrc, &fdReader);
				Accept();

			}
			int n = 0;
			//printf("%d", fdReader.fd_count);
			for (n; n < fdReader.fd_count; n++)
			{
				if (-1 == RecvData(fdReader.fd_array[n]))
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdReader.fd_array[n]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}
			//printf("空闲时间处理其他业务...\n");

			return true;
		}
		return false;
		

	}
	//是否工作中
	bool isRun()
	{
		return socketSrc != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET sockClient)
	{
		char recvBuf[1024];

		//5.接收客户端数据
		DataHeader* header = (DataHeader*)recvBuf;
		if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
		{
			printf("客户端<SOCKET:%d>已退出,任务结束\n", sockClient);
			return -1;
		}
		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(sockClient, header);
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(SOCKET sockClient,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("收到客户端<SOCKET:%d>请求:CM_LOGIN ,数据长度%d UsreName:%s passWord=%s\n", sockClient, login->dataLength, login->userName, login->passWord);
			//忽略判断用户名密码是否正确的过程
			loginResult ret;
			send(sockClient, (char*)&ret, sizeof(loginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{

			Loginout* logout = (Loginout*)header;
			printf("收到<SOCKET:%d>命令:CM_LOGINOUT ,数据长度%d UsreName:%s\n", sockClient, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确的过程
			LoginoutResult ret;
			send(sockClient, (char*)&ret, sizeof(LoginoutResult), 0);
		}
		break;
		default:
		{
			DataHeader header = { 0,CMD_ERROR };
			send(sockClient, (char*)&header, sizeof(header), 0);
		}
		break;
		}
	}
	//发送指定的socket数据

	int  sendData(SOCKET sockClient,DataHeader* header)
	{
		if (isRun() && header)
		{
			//printf("走到了send");
			return send(sockClient, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}
	//群发socket数据

	void sendDataToAll(DataHeader* header)
	{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				sendData(g_clients[n],header);
			}
	}

};
