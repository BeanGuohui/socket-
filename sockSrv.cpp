#define WIN32_LEAD_AND_MEAD
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
//��Ϣͷ
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_UEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader {
	short dataLength;//���ݳ���
	short cmd;//����
};
//���ݰ�
struct Login :public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
struct loginResult :public DataHeader
{
	loginResult() {
		dataLength = sizeof(loginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}

	int result;

};
struct Loginout :public DataHeader
{
	Loginout() {
		dataLength = sizeof(Loginout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};
struct LoginoutResult:public DataHeader
{
	LoginoutResult() {
		dataLength = sizeof(LoginoutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};
struct  NewUserJoin:public DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_UEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

vector<SOCKET> g_clients;

int processor(SOCKET sockClient)
{
	char recvBuf[1024];

	//5.���տͻ�������

	if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
	{
		printf("�ͻ���<SOCKET:%d>���˳�,�������\n",sockClient);
		return -1;
	}
	DataHeader* header = (DataHeader*)recvBuf;
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)recvBuf;
		printf("�յ��ͻ���<SOCKET:%d>����:CM_LOGIN ,���ݳ���%d UsreName:%s passWord=%s\n", sockClient,login->dataLength, login->userName, login->passWord);
		//�����ж��û��������Ƿ���ȷ�Ĺ���
		loginResult ret;
		send(sockClient, (char*)&ret, sizeof(loginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{

		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout* logout = (Loginout*)recvBuf;
		printf("�յ�<SOCKET:%d>����:CM_LOGINOUT ,���ݳ���%d UsreName:%s\n", sockClient, logout->dataLength, logout->userName);
		//�����ж��û��������Ƿ���ȷ�Ĺ���
		LoginoutResult ret;
		send(sockClient, (char*)&ret, sizeof(ret), 0);
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
int main() 
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2),&wsaData);
	
	//1.����һ��socket�׽���
	SOCKET socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.bind�����ڽ��ܿͻ������ӵ�����˿�
	SOCKADDR_IN addrSrc;
	addrSrc.sin_family = AF_INET;//�����������ӵ�ipv4����
	addrSrc.sin_port = htons(4567);//host to net unsigned short
	addrSrc.sin_addr.S_un.S_addr = INADDR_ANY;//intenet_address
	//127.0.0.1:�����������ʣ������196.128..�������������ʣ�
	//INADDR_ANY:���һ��IP��ַ���������綼���Է��ʣ����޶���һ������

	if (bind(socketSrc, (SOCKADDR*)&addrSrc, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("ERROR:�����ڽ��ܿͻ������ӵ�����˿�ʧ��\n");
	}
	else {
		printf("�󶨶˿ڳɹ�.\n");
	}
	//3.listen ��������ӿ�

	if (SOCKET_ERROR == listen(socketSrc, 5))//��������ӿ�,���5����ͬʱ����;
	{
		printf("��������ӿ�ʧ��\n");
	}
	else {
		printf("��������ӿڳɹ�\n");
	}
	
	while (TRUE)
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
		for (int n = (int)g_clients.size()-1; n >= 0 ;n--)
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
			break;
		}
		if (FD_ISSET(socketSrc, &fdReader))
		{
			FD_CLR(socketSrc, &fdReader);
			//4.accept�ȴ����ܿͻ�������
			SOCKADDR_IN addrClient;
			//addrClient.sin_family = AF_INET;
			int len = sizeof(addrClient);
			char msgBuf[] = "hello,I'm Server.";
			SOCKET sockClient = INVALID_SOCKET;//��ʼ����Ч��socket��ַ
			sockClient = accept(socketSrc, (SOCKADDR*)&addrClient, &len);
			if (sockClient == INVALID_SOCKET)
			{
				printf("����:���ܵ���Ч�ͻ���socket...\n");
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				NewUserJoin userJoin;
				send(g_clients[n], (const char *)&userJoin, sizeof(NewUserJoin), 0);
			}
			g_clients.push_back(sockClient);
			printf("�¿ͻ��˼���:socket= %d,IP=%s\n", (int)sockClient, inet_ntoa(addrClient.sin_addr));			
		}
		int n = 0;
		//printf("%d", fdReader.fd_count);
		for (n; n < fdReader.fd_count;  n++)
		{
			if (-1 == processor(fdReader.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdReader.fd_array[n]);
				if (iter!= g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		printf("����ʱ�䴦������ҵ��...\n");

	}
	for (int n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	//6.�ر��׽���closesocket
	closesocket(socketSrc);
	WSACleanup();
	printf("������˳�,�������\n");
	getchar();
	return 0;
}