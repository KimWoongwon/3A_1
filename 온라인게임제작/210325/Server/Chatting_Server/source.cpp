#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9900
#define NICK_SIZE 20
#define MAXUSER 10

#define BUFSIZE 512

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
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
// ���� �Լ� ���� ���
void err_display(char* msg)
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

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	int oldoptval, optval, optlen;

	// ���� ������ ũ�⸦ ��´�.
	optlen = sizeof(optval);
	retval = getsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&optval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("���� ���� ũ�� = %d����Ʈ\n", optval);

	oldoptval = optval;

	for (int i = 0; i < 3; i++)
	{
		// ���� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		optlen = sizeof(optval);
		retval = getsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ�� = %d����Ʈ\n", optval);
	}

	// ���� ������ ũ�⸦ ���� ���� �Ѵ�.
	retval = setsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	optlen = sizeof(oldoptval);
	retval = getsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("���� ���� ũ�� = %d����Ʈ\n", oldoptval);

	// ���� ����
	WSACleanup();

	getchar();

	return 0;
}