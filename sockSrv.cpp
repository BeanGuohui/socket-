#include "EasyTcpServer.hpp"
int main() 
{
	EasyTcpServer server;
	//server.initSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);

	while (server.isRun())
	{
		server.OnRun();
		//for (int n = g_clients.size() - 1; n >= 0; n--)
		//{
	//		closesocket(g_clients[n]);
		//}
	}
	//6.�ر��׽���closesocket
	server.Close();
	printf("������˳�,�������\n");
	getchar();
	return 0;
}