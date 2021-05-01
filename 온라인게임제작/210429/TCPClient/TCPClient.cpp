#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

enum PROTOCOL
{
	INTRO = 1,
	LOGIN,

};

enum RESULT
{
	NODATA = -1,
	ID_ERROR = 1,
	PW_ERROR,
	LOGIN_SUCCESS
};

struct _RecvPacket
{
	RESULT result;
	char msg[255];
};

struct _LoginInfo
{
	char id[255];
	char pw[255];
};
// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0)
	{
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int PackPacket(char* buf, PROTOCOL protocol, char* id, char* pw);
void UnPackPacket(const char* buf, RESULT* result, char* msg);

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE+1];
	int len;
	_RecvPacket packet;
	ZeroMemory(&packet, sizeof(packet));
	ZeroMemory(buf, sizeof(buf));

	int size;
	retval = recvn(sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("packetsize recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(sock, buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_quit("intro recv()");		
	}
	else if (retval == 0)
	{
		err_quit("intro recv()");
	}
		
	UnPackPacket(buf, &packet.result, packet.msg);
	printf("%s", packet.msg);

	// 서버와 데이터 통신
	while(1)
	{		
		_LoginInfo info;
		ZeroMemory(&info, sizeof(_LoginInfo));

		printf("ID:");
		scanf("%s", info.id);

		printf("PW:");
		scanf("%s", info.pw);

		int size = PackPacket(buf, LOGIN, info.id, info.pw);
		retval = send(sock, buf, size,0);
		if (retval == SOCKET_ERROR)
		{
			err_display("login info send()");
			break;
		}

		_RecvPacket result;
		ZeroMemory(&result, sizeof(result));
		ZeroMemory(buf, sizeof(buf));
		retval = recvn(sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("packetsize recv error()");
			return false;
		}
		else if (retval == 0)
		{
			return false;
		}

		retval = recvn(sock, buf, size, 0);
		UnPackPacket(buf, &result.result, result.msg);
 		if (retval == SOCKET_ERROR)
		{
			err_display("intro recv()");
			break;
		}
		else if (retval == 0)
			break;
	
		printf("%s", result.msg);
		
		if (result.result==LOGIN_SUCCESS)
		{
			break;
		}
		
	}//while end

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
int PackPacket(char* buf, PROTOCOL protocol, char* id, char* pw)
{
	char* ptr = buf;
	int size = 0;
	ptr = ptr + sizeof(int);

	memcpy(ptr, &protocol, sizeof(PROTOCOL));
	ptr = ptr + sizeof(PROTOCOL);
	size = size + sizeof(PROTOCOL);

	int strsize = strlen(id);
	memcpy(ptr, &strsize, sizeof(int));
	ptr = ptr + sizeof(int);
	size = size + sizeof(int);

	memcpy(ptr, id, strsize);
	ptr = ptr + strsize;
	size = size + strsize;

	strsize = strlen(pw);
	memcpy(ptr, &strsize, sizeof(int));
	ptr = ptr + sizeof(int);
	size = size + sizeof(int);

	memcpy(ptr, pw, strsize);
	ptr = ptr + strsize;
	size = size + strsize;

	ptr = buf;

	memcpy(ptr, &size, sizeof(int));
	size = size + sizeof(int);

	return size;
}

void UnPackPacket(const char* buf, RESULT* result, char* msg)
{
	int strsize;
	const char* ptr = buf + sizeof(PROTOCOL);

	memcpy(result, ptr, sizeof(RESULT));
	ptr = ptr + sizeof(RESULT);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(msg, ptr, strsize);
	ptr = ptr + strsize;
}