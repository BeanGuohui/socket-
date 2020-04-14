#pragma once
#define WIN32_LEAD_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include <cstdio>
#include <thread>
#include "MessageHeader.hpp"
#pragma comment(lib,"ws2_32.lib")
class EasyTcpClient
{
	SOCKET sockClient;
public:
	EasyTcpClient()
	{
		sockClient = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient() {
		Close();
	}

	//初始化socket
	int initSocket()
	{
		//启动winsocket环境
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		//1.建立一个socket;
		if (INVALID_SOCKET == sockClient)
		{
			printf("<socket = %d>关闭旧连接\n", sockClient);
			Close();
		}
		sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockClient == INVALID_SOCKET)
		{
			printf("创建失败\n");
		}
		else {
			printf("创建成功\n");
		}
		return 0;
	}

	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == sockClient)
		{
			initSocket();
		}
		//2.连接服务器 connect
		SOCKADDR_IN addrClient = {};
		addrClient.sin_family = AF_INET;
		addrClient.sin_port = htons(port);
		addrClient.sin_addr.S_un.S_addr = inet_addr(ip);
		//addrClient.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");


		if (connect(sockClient, (SOCKADDR*)&addrClient, sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			printf("连接失败\n");
		}
		else {
			printf("连接成功\n");
		}
		return 0;

	}

	//关闭socket
	void Close()
	{
		if (sockClient != INVALID_SOCKET)
		{
			//关闭环境
			//4.关闭套接字
			closesocket(sockClient);
			sockClient = INVALID_SOCKET;
			WSACleanup();
			
		}
	}
	//收数据
	//发数据
	//处理网络消息

	bool isRun()
	{
		return sockClient != INVALID_SOCKET;
	}
	bool onRun()
	{
		if (isRun())
		{
			fd_set fdReader;
			FD_ZERO(&fdReader);
			FD_SET(sockClient, &fdReader);

			timeval t = { 1,0 };
			int ret = select(sockClient, &fdReader, NULL, NULL, &t);
			if (ret < 0)
			{
				printf("<socket = %d>select任务结束1\n", sockClient);
				return false;
			}
			if (FD_ISSET(sockClient, &fdReader))
			{
				FD_CLR(sockClient, &fdReader);
				if (-1 == RecvData())
				{
					printf("<select = %d>任务结束2\n", sockClient);
					return false;
				}
			}
			return true;
		}
		return false;
	}



	//接收数据 处理粘包 拆分包问题
	int RecvData()
	{
		char recvBuf[1024];

		//5.接收客户端数据
		DataHeader* header = (DataHeader*)recvBuf;
		if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
		{
			printf("与服务器断开连接,任务结束.\n");
			return -1;
		}
		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg( header);
		
		return 0;
	}

	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			loginResult* login = (loginResult*)header;
			printf("收到服务器数据:CM_LOGIN_RESULT ,数据长度%d\n", login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{

			LoginoutResult* logout = (LoginoutResult*)header;
			printf("收到服务器消息:CM_LOGINOUT ,数据长度%d\n", logout->dataLength);
		}
		break;
		case  CMD_UEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("收到服务器消息:CM_UEW_USER_JOIN ,数据长度%d\n", userJoin->dataLength);
		}
		break;
		}
	}
	int  sendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			//printf("走到了send");
			return send(sockClient, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}

};