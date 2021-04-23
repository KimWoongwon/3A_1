/*
 * echo_selserv_win.c
 * Written by SW. YOON
 */

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define ERROR_DISCONNECTED -2
#define DISCONNECTED -1
#define SOC_TRUE 1
#define SOC_FALSE 0
#define JOIN_COUNT 3

#define INTRO_MSG "아이디와 패스워드를 입력하세요\n"
#define IDERROR_MSG "없는 아이디입니다\n"
#define PWERROR_MSG "패스워드가 틀렸습니다.\n"
#define LOGINSUCCESS_MSG "로그인에 성공했습니다.\n"

enum PROTOCOL
{
	INTRO = 1,
	LOGIN,

};

enum STATE 
{
	INIT_STATE=-1, 
	INTRO_SEND_STATE =1, 
	LOGIN_INFO_RECV_STATE,
	LOGIN_RESULT_SEND_STATE,
	END_STATE,
	DISCONNECTED_STATE
};

enum RESULT 
{
	NODATA = -1, 	
	ID_ERROR=1, 
	PW_ERROR, 
	LOGIN_SUCCESS 
};

struct _SendPacket
{
	RESULT result;
	char msg[255];
};

struct _LoginInfo
{
	char id[255];
	char pw[255];
}LoginInfo[3] = { {"kja0204","1234"},{"aaa","111"},{"bbb","222"} };


struct _ClientInfo
{
	SOCKET		sock;
	SOCKADDR_IN addr;
	_LoginInfo  userinfo;
	STATE		state;
	
	int			recvbytes;
	int			comp_recvbytes;
	int			sendbytes;
	int			comp_sendbytes;

	char		recvbuf[BUFSIZE];
	char		sendbuf[BUFSIZE];

};

_ClientInfo* ClientInfo[100];
int Count = 0;

FD_SET Rset, Wset;

void LoginProcess(_ClientInfo* _ptr);
void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
int MessageRecv(_ClientInfo* _info);
int MessageSend(_ClientInfo* _info);
_ClientInfo* AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr);
void RemoveClient(_ClientInfo* _ptr);
void RecvPacketProcess(_ClientInfo* _ptr);
void SendPacketProcess(_ClientInfo* _ptr);
void LoginProcess(_ClientInfo* _ptr);

int PackPacket(char* buf, PROTOCOL protocol, int result, const char* msg);
void UnPackPacket(const char* buf, char* id, char* pw);

int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	int retval;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		err_quit("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		err_quit("socket() error");

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9000);

	u_long on = 1;
	retval = ioctlsocket(hServSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");


	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		err_quit("bind() error");
	if (listen(hServSock, 100) == SOCKET_ERROR)
		err_quit("listen() error");


	while (1)
	{
		FD_ZERO(&Rset);
		FD_ZERO(&Wset);

		FD_SET(hServSock, &Rset);

		for (int i = 0; i < Count; i++)
		{
			FD_SET(ClientInfo[i]->sock, &Rset);

			switch (ClientInfo[i]->state)
			{
			case INTRO_SEND_STATE:
			case LOGIN_RESULT_SEND_STATE:			
				FD_SET(ClientInfo[i]->sock, &Wset);
				break;
			}			
		}

		if (select(0, &Rset, &Wset, 0, NULL) == SOCKET_ERROR)
		{
			err_quit("select() error");
		}			

 		if (FD_ISSET(hServSock, &Rset)) 
		{
			int clntLen = sizeof(clntAddr);
			hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntLen);
			_ClientInfo* ptr = AddClient(hClntSock, clntAddr);
			_SendPacket packet;
			/*memset(&packet, 0, sizeof(_SendPacket));
			packet.result = NODATA;
			strcpy(packet.msg, INTRO_MSG);
			memcpy(ptr->sendbuf, &packet, sizeof(_SendPacket));
			ptr->sendbytes = sizeof(_SendPacket);*/
			ZeroMemory(ptr->sendbuf, sizeof(ptr->sendbuf));
			ptr->sendbytes = PackPacket(ptr->sendbuf, INTRO, NODATA, INTRO_MSG);
			ptr->state = INTRO_SEND_STATE;			
		}		

		for (int i = 0; i < Count; i++)
		{
			_ClientInfo* ptr = ClientInfo[i];

			if (FD_ISSET(ptr->sock, &Rset))
			{
				ptr->recvbytes = sizeof(_LoginInfo);

				int result = MessageRecv(ptr);
				switch (result)
				{
				case ERROR_DISCONNECTED:
					err_display("connect end");
				case DISCONNECTED:
					ptr->state = DISCONNECTED_STATE;
					break;
				case SOC_FALSE:
					continue;
				case SOC_TRUE:
					break;
				}

				RecvPacketProcess(ptr);
			}					

			if (FD_ISSET(ptr->sock, &Wset))
			{
				int result;

				result = MessageSend(ptr);
				switch (result)
				{
				case ERROR_DISCONNECTED:
					err_display("connect end");
				case DISCONNECTED:
					ptr->state = DISCONNECTED_STATE;
					break;
				case SOC_FALSE:
					continue;
				case SOC_TRUE:					
					break;
				}				

				SendPacketProcess(ptr);
				
			}

			if (ptr->state == DISCONNECTED_STATE)
			{
				RemoveClient(ptr);
				i--;
				continue;
			}

		}
	}	

	closesocket(hServSock);
	WSACleanup();
	return 0;
}



void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}



int MessageRecv(_ClientInfo* _info)
{
	int size, comp_size;
	int retval = recvn(_info->sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("packetsize recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(_info->sock, _info->recvbuf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		return ERROR_DISCONNECTED;
	}
	else if (retval == 0)
	{
		return DISCONNECTED;
	}
	else
	{
		if (retval == size)
		{
			return SOC_TRUE;
		}
		return SOC_FALSE;
	}

	/*int retval = recv(_info->sock, _info->recvbuf + _info->comp_recvbytes, _info->recvbytes - _info->comp_recvbytes, 0);
	if (retval == SOCKET_ERROR)
	{
		return ERROR_DISCONNECTED;
	}
	else if (retval == 0)
	{
		return DISCONNECTED;
	}
	else
	{
		_info->comp_recvbytes += retval;
		if (_info->comp_recvbytes == _info->recvbytes)
		{
			_info->comp_recvbytes = 0;
			_info->recvbytes = 0;
			return SOC_TRUE;
		}
		return SOC_FALSE;
	}*/

}

int MessageSend(_ClientInfo* _info)
{ 
	int retval = send(_info->sock, _info->sendbuf,	_info->sendbytes, 0);
	if (retval == SOCKET_ERROR)
	{
		return ERROR_DISCONNECTED;
	}
	else
	{
		//_info->comp_sendbytes = _info->comp_sendbytes + retval;

		if (_info->sendbytes == retval)
		{
			_info->sendbytes = 0;
			_info->comp_sendbytes = 0;

			return SOC_TRUE;
		}
		else
		{
			return SOC_FALSE;
		}
	}
}


_ClientInfo* AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr)
{
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//소켓 구조체 배열에 새로운 소켓 정보 구조체 저장
	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));

	ptr->sock = _sock;
	memcpy(&ptr->addr, &_clientaddr, sizeof(SOCKADDR_IN));
	ptr->state = INIT_STATE;
	ClientInfo[Count++] = ptr;
	return ptr;
}

void RemoveClient(_ClientInfo* _ptr)
{
	closesocket(_ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

	for (int i = 0; i < Count; i++)
	{
		if (ClientInfo[i] == _ptr)
		{
			delete ClientInfo[i];

			for (int j = i; j < Count - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[Count - 1] = nullptr;
			Count--;
			break;
		}
	}

}

void RecvPacketProcess(_ClientInfo* _ptr)
{
	switch (_ptr->state)
	{
	case LOGIN_INFO_RECV_STATE:
		memset(&_ptr->userinfo, 0, sizeof(_LoginInfo));
		UnPackPacket(_ptr->recvbuf, _ptr->userinfo.id, _ptr->userinfo.pw);
		//memcpy(&(_ptr->userinfo), _ptr->recvbuf, sizeof(_LoginInfo));
		LoginProcess(_ptr);
		break;
	}

}

void SendPacketProcess(_ClientInfo* _ptr)
{
	switch (_ptr->state)
	{
	case INTRO_SEND_STATE:
		_ptr->state = LOGIN_INFO_RECV_STATE;
		break;
	case LOGIN_RESULT_SEND_STATE:
		if (strlen(_ptr->userinfo.id) == 0)
		{
			_ptr->state = LOGIN_INFO_RECV_STATE;
		}
		else
		{
			_ptr->state = END_STATE;
		}
		break;
	}
}

void LoginProcess(_ClientInfo* _ptr)
{
	RESULT result = NODATA;
	char msg[255];
	ZeroMemory(msg, sizeof(msg));
	/*_SendPacket login_result;
	memset(&login_result, 0, sizeof(_SendPacket));
	login_result.result = NODATA;*/

	for (int i = 0; i < JOIN_COUNT; i++)
	{
		if (!strcmp(LoginInfo[i].id, _ptr->userinfo.id))
		{
			if (!strcmp(LoginInfo[i].pw, _ptr->userinfo.pw))
			{
				result = LOGIN_SUCCESS;
				strcpy(msg, LOGINSUCCESS_MSG);
			}
			else
			{
				result = PW_ERROR;
				strcpy(msg, PWERROR_MSG);
			}
			break;
		}
	}

	if (result == NODATA)
	{
		result = ID_ERROR;
		strcpy(msg, IDERROR_MSG);
	}

	if (result != LOGIN_SUCCESS)
	{
		memset(&_ptr->userinfo, 0, sizeof(_LoginInfo));
	}
	/*memcpy(_ptr->sendbuf, &login_result, sizeof(_SendPacket));
	_ptr->sendbytes = sizeof(_SendPacket);*/
	ZeroMemory(_ptr->sendbuf, sizeof(_ptr->sendbuf));
	_ptr->sendbytes = PackPacket(_ptr->sendbuf, LOGIN, result, msg);
	_ptr->state = LOGIN_RESULT_SEND_STATE;
}

int PackPacket(char* _buf, PROTOCOL _protocol, int result, const char* _str1)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(_protocol);

	memcpy(ptr, &result, sizeof(result));
	ptr = ptr + sizeof(result);
	size = size + sizeof(result);

	int strsize1 = strlen(_str1);
	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);
	size = size + sizeof(strsize1);

	memcpy(ptr, _str1, strsize1);
	ptr = ptr + strsize1;
	size = size + strsize1;

	ptr = _buf;

	memcpy(ptr, &size, sizeof(size));
	size = size + sizeof(size);
	return size;
}

void UnPackPacket(const char* buf, char* id, char* pw)
{
	int strsize;
	const char* ptr = buf + sizeof(PROTOCOL);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(id, ptr, strsize);
	ptr = ptr + strsize;

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(pw, ptr, strsize);
	ptr = ptr + strsize;
}