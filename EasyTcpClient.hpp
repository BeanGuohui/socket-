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

	//��ʼ��socket
	int initSocket()
	{
		//����winsocket����
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		//1.����һ��socket;
		if (INVALID_SOCKET == sockClient)
		{
			printf("<socket = %d>�رվ�����\n", sockClient);
			Close();
		}
		sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockClient == INVALID_SOCKET)
		{
			printf("����ʧ��\n");
		}
		else {
			printf("�����ɹ�\n");
		}
		return 0;
	}

	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == sockClient)
		{
			initSocket();
		}
		//2.���ӷ����� connect
		SOCKADDR_IN addrClient = {};
		addrClient.sin_family = AF_INET;
		addrClient.sin_port = htons(port);
		addrClient.sin_addr.S_un.S_addr = inet_addr(ip);
		//addrClient.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");


		if (connect(sockClient, (SOCKADDR*)&addrClient, sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			printf("����ʧ��\n");
		}
		else {
			printf("���ӳɹ�\n");
		}
		return 0;

	}

	//�ر�socket
	void Close()
	{
		if (sockClient != INVALID_SOCKET)
		{
			//�رջ���
			//4.�ر��׽���
			closesocket(sockClient);
			sockClient = INVALID_SOCKET;
			WSACleanup();
			
		}
	}
	//������
	//������
	//����������Ϣ

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
				printf("<socket = %d>select�������1\n", sockClient);
				return false;
			}
			if (FD_ISSET(sockClient, &fdReader))
			{
				FD_CLR(sockClient, &fdReader);
				if (-1 == RecvData())
				{
					printf("<select = %d>�������2\n", sockClient);
					return false;
				}
			}
			return true;
		}
		return false;
	}



	//�������� ����ճ�� ��ְ�����
	int RecvData()
	{
		char recvBuf[1024];

		//5.���տͻ�������
		DataHeader* header = (DataHeader*)recvBuf;
		if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
		{
			printf("��������Ͽ�����,�������.\n");
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
			printf("�յ�����������:CM_LOGIN_RESULT ,���ݳ���%d\n", login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{

			LoginoutResult* logout = (LoginoutResult*)header;
			printf("�յ���������Ϣ:CM_LOGINOUT ,���ݳ���%d\n", logout->dataLength);
		}
		break;
		case  CMD_UEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("�յ���������Ϣ:CM_UEW_USER_JOIN ,���ݳ���%d\n", userJoin->dataLength);
		}
		break;
		}
	}
	int  sendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			//printf("�ߵ���send");
			return send(sockClient, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}

};