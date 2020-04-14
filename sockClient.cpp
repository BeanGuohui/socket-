#define WIN32_LEAD_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include <cstdio>
#pragma comment(lib,"ws2_32.lib")
//消息头
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
	short dataLength;//数据长度
	short cmd;//命令
};
//数据包
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
struct LoginoutResult :public DataHeader
{
	LoginoutResult() {
		dataLength = sizeof(LoginoutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};
struct  NewUserJoin :public DataHeader
{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_UEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

int processor(SOCKET sockClient)
{
	char recvBuf[1024];

	//5.接收客户端数据

	if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
	{
		printf("与服务器断开连接,任务结束.\n");
		return -1;
	}
	DataHeader* header = (DataHeader*)recvBuf;
	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			loginResult* login = (loginResult*)recvBuf;
			printf("收到服务器数据:CM_LOGIN_RESULT ,数据长度%d\n", login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{

			recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginoutResult* logout = (LoginoutResult*)recvBuf;
			printf("收到服务器消息:CM_LOGINOUT ,数据长度%d\n", logout->dataLength);
		}
		break;
		case  CMD_UEW_USER_JOIN:
		{
			recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* userJoin = (NewUserJoin*)recvBuf;
			printf("收到服务器消息:CM_UEW_USER_JOIN ,数据长度%d\n", userJoin->dataLength);
		}
		break;
	}
}


int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//1.建立一个socket;
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClient == INVALID_SOCKET)
	{
		printf("创建失败\n");
	}
	else {
		printf("创建成功\n");
	}
	//2.连接服务器 connect
	SOCKADDR_IN addrClient = {};
	addrClient.sin_family = AF_INET;
	addrClient.sin_port = htons(4567);
	addrClient.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//addrClient.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");

	
	if (connect(sockClient, (SOCKADDR*)&addrClient, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("连接失败\n");
	}
	else {
		printf("连接成功\n");
	}
	
	while (1)
	{
		fd_set fdReader;
		FD_ZERO(&fdReader);
		FD_SET(sockClient, &fdReader);

		timeval t = { 1,0 };
		int ret = select(sockClient, &fdReader, NULL, NULL, &t);
		if (ret < 0)
		{
			printf("select任务结束1\n");
			break;
		}
		if (FD_ISSET(sockClient,&fdReader))
		{
			FD_CLR(sockClient, &fdReader);
			if (-1 == processor(sockClient))
			{
				printf("select任务结束2\n");
				break;
			}
		}
	
		printf("空闲时间处理其他业务\n");
		Login login;
		strcpy(login.userName, "ggy");
		strcpy(login.passWord, "ggy");
		send(sockClient, (const char*)&login, sizeof(Login), 0);
		Sleep(1000);
	}
	//4.关闭套接字
	closesocket(sockClient);

	WSACleanup();
	printf("退出");
	getchar();
	return 0;
}