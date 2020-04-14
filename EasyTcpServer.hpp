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
	//��ʼ��socket
	SOCKET initSocket()
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		//1.����һ��socket�׽���
		socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//1.����һ��socket;
		if (INVALID_SOCKET == socketSrc)
		{
			printf("<socket = %d>�رվ�����\n", socketSrc);
			Close();
		}
		socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socketSrc == INVALID_SOCKET)
		{
			printf("����ʧ��\n");
		}
		else {
			printf("�����ɹ�\n");
		}
		return socketSrc;
	}


	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port )
	{
		if (INVALID_SOCKET == socketSrc)
		{
			initSocket();
			 
		}
		
		//2.bind�����ڽ��ܿͻ������ӵ�����˿�
		SOCKADDR_IN addrSrc;
		addrSrc.sin_family = AF_INET;//�����������ӵ�ipv4����
		addrSrc.sin_port = htons(port);//host to net unsigned short
		
		if (ip)
		{
			addrSrc.sin_addr.S_un.S_addr = inet_addr(ip);//intenet_address
		}
		else {
			addrSrc.sin_addr.S_un.S_addr = INADDR_ANY;//intenet_address

		}
		//127.0.0.1:�����������ʣ������196.128..�������������ʣ�
		//INADDR_ANY:���һ��IP��ַ���������綼���Է��ʣ����޶���һ������
		int ret = bind(socketSrc, (SOCKADDR*)&addrSrc, sizeof(SOCKADDR));
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR:��<%d>�˿�ʧ��\n",port);
		}
		else {
			printf("�󶨶˿�<%d>�ɹ�.\n",port);
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n)
	{
		//3.listen ��������ӿ�

		if (SOCKET_ERROR == listen(socketSrc, n))//��������ӿ�,���5����ͬʱ����;
		{
			printf("Socket = <%d>��������ӿ�ʧ��\n",socketSrc);
		}
		else {
			printf("Socket = <%d>��������ӿڳɹ�\n", socketSrc);
		}
		return 0;
	}
	//���ܿͻ�������
	SOCKET Accept()
	{
		//4.accept�ȴ����ܿͻ�������
		SOCKADDR_IN addrClient;
		//addrClient.sin_family = AF_INET;
		int len = sizeof(addrClient);
		char msgBuf[] = "hello,I'm Server.";
		SOCKET sockClient = INVALID_SOCKET;//��ʼ����Ч��socket��ַ
		sockClient = accept(socketSrc, (SOCKADDR*)&addrClient, &len);
		if (sockClient == INVALID_SOCKET)
		{
			printf("Socket = <%d>����:���ܵ���Ч�ͻ���socket...\n", socketSrc);
		}
		else
		{
			NewUserJoin userJoin;
			sendDataToAll(&userJoin);
			g_clients.push_back(sockClient);
			printf("Socket = <%d>�¿ͻ��˼���:socket= %d,IP=%s\n", (int)sockClient,(int)sockClient, inet_ntoa(addrClient.sin_addr));
		}
		return sockClient;
		
	}
	//�ر�socket
	void Close()
	{
		if (socketSrc != INVALID_SOCKET)
		{
			//�رջ���
			//4.�ر��׽���
			closesocket(socketSrc);
			socketSrc = INVALID_SOCKET;
			WSACleanup();
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//������ socket���� ������
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

			//nfds ��һ������ֵ,��ָfd_set����������������(socket)�ķ�Χ,����������.
			//���������ļ����������ֵ+1,��windows�������������д0
			timeval t = { 1,0 };
			int ret = select(socketSrc + 1, &fdReader, &fdWriter, &fdExp, &t);
			/*if (ret == 0)
			{
				continue;
			}*/

			if (ret < 0)
			{
				printf("select�������\n");
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
			//printf("����ʱ�䴦������ҵ��...\n");

			return true;
		}
		return false;
		

	}
	//�Ƿ�����
	bool isRun()
	{
		return socketSrc != INVALID_SOCKET;
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET sockClient)
	{
		char recvBuf[1024];

		//5.���տͻ�������
		DataHeader* header = (DataHeader*)recvBuf;
		if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
		{
			printf("�ͻ���<SOCKET:%d>���˳�,�������\n", sockClient);
			return -1;
		}
		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(sockClient, header);
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET sockClient,DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("�յ��ͻ���<SOCKET:%d>����:CM_LOGIN ,���ݳ���%d UsreName:%s passWord=%s\n", sockClient, login->dataLength, login->userName, login->passWord);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			loginResult ret;
			send(sockClient, (char*)&ret, sizeof(loginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{

			Loginout* logout = (Loginout*)header;
			printf("�յ�<SOCKET:%d>����:CM_LOGINOUT ,���ݳ���%d UsreName:%s\n", sockClient, logout->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
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
	//����ָ����socket����

	int  sendData(SOCKET sockClient,DataHeader* header)
	{
		if (isRun() && header)
		{
			//printf("�ߵ���send");
			return send(sockClient, (const char*)header, header->dataLength, 0);
		}

		return SOCKET_ERROR;
	}
	//Ⱥ��socket����

	void sendDataToAll(DataHeader* header)
	{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				sendData(g_clients[n],header);
			}
	}

};
