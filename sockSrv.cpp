#define WIN32_LEAD_AND_MEAD
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
using namespace std;
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

	//5.接收客户端数据

	if (recv(sockClient, recvBuf, sizeof(DataHeader), 0) <= 0)
	{
		printf("客户端<SOCKET:%d>已退出,任务结束\n",sockClient);
		return -1;
	}
	DataHeader* header = (DataHeader*)recvBuf;
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)recvBuf;
		printf("收到客户端<SOCKET:%d>请求:CM_LOGIN ,数据长度%d UsreName:%s passWord=%s\n", sockClient,login->dataLength, login->userName, login->passWord);
		//忽略判断用户名密码是否正确的过程
		loginResult ret;
		send(sockClient, (char*)&ret, sizeof(loginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{

		recv(sockClient, recvBuf + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout* logout = (Loginout*)recvBuf;
		printf("收到<SOCKET:%d>命令:CM_LOGINOUT ,数据长度%d UsreName:%s\n", sockClient, logout->dataLength, logout->userName);
		//忽略判断用户名密码是否正确的过程
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
	
	//1.建立一个socket套接字
	SOCKET socketSrc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.bind绑定用于接受客户端连接的网络端口
	SOCKADDR_IN addrSrc;
	addrSrc.sin_family = AF_INET;//用于网络连接的ipv4类型
	addrSrc.sin_port = htons(4567);//host to net unsigned short
	addrSrc.sin_addr.S_un.S_addr = INADDR_ANY;//intenet_address
	//127.0.0.1:限制外网访问，如果在196.128..可以在内网访问；
	//INADDR_ANY:随便一个IP地址，所有网络都可以访问，不限定哪一个网络

	if (bind(socketSrc, (SOCKADDR*)&addrSrc, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("ERROR:绑定用于接受客户端连接的网络端口失败\n");
	}
	else {
		printf("绑定端口成功.\n");
	}
	//3.listen 监听网络接口

	if (SOCKET_ERROR == listen(socketSrc, 5))//监听网络接口,最大5个人同时链接;
	{
		printf("监听网络接口失败\n");
	}
	else {
		printf("监听网络接口成功\n");
	}
	
	while (TRUE)
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
		for (int n = (int)g_clients.size()-1; n >= 0 ;n--)
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
			break;
		}
		if (FD_ISSET(socketSrc, &fdReader))
		{
			FD_CLR(socketSrc, &fdReader);
			//4.accept等待接受客户端链接
			SOCKADDR_IN addrClient;
			//addrClient.sin_family = AF_INET;
			int len = sizeof(addrClient);
			char msgBuf[] = "hello,I'm Server.";
			SOCKET sockClient = INVALID_SOCKET;//初始化无效的socket地址
			sockClient = accept(socketSrc, (SOCKADDR*)&addrClient, &len);
			if (sockClient == INVALID_SOCKET)
			{
				printf("错误:接受到无效客户端socket...\n");
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				NewUserJoin userJoin;
				send(g_clients[n], (const char *)&userJoin, sizeof(NewUserJoin), 0);
			}
			g_clients.push_back(sockClient);
			printf("新客户端加入:socket= %d,IP=%s\n", (int)sockClient, inet_ntoa(addrClient.sin_addr));			
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
		printf("空闲时间处理其他业务...\n");

	}
	for (int n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	//6.关闭套接字closesocket
	closesocket(socketSrc);
	WSACleanup();
	printf("服务端退出,任务结束\n");
	getchar();
	return 0;
}