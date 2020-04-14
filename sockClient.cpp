#include "EasyTcpClient.hpp"
#include <thread>
#include <iostream>

void cmdThread(EasyTcpClient* sockClient)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			sockClient->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "ggy");
			strcpy(login.passWord, "ggy");
			sockClient->sendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Loginout logout;
			strcpy(logout.userName, "ggy");
			sockClient->sendData(&logout);
		}
		else
		{
			printf("不支持的命令\n");
		}
	}
}
int main()
{
	
	EasyTcpClient client;
	client.initSocket();
	client.Connect("127.0.0.1", 4567);
	//启动线程函数
	std::thread t1(cmdThread, &client);
	t1.detach();
	
	while (client.isRun())
	{
		client.onRun();
	}
	client.Close();
	//4.关闭套接字
	printf("退出");
	getchar();
	return 0;
}