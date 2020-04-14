#pragma once
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


