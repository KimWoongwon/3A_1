#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <mstcpip.h>

#define SERVERPORT 9900
#define NICK_SIZE 20
#define MAXUSER 10

#define BUFSIZE 512

// Nagle �˰���
// Nagle �˰���(���� ���̱� �˰���)�� '�����ϸ� ���ݾ� ������ ������ ���� �ѹ��� ���� �����͸� ������'
// ��� ��Ģ�� ������� ������� �˰����̴�. ���̱� �˰����� ���� ���¶�� 'hello'��� �����͸� ������ ����
// h ������ e ������ l ������ l ������ o �� ������ ��Ŷ�� 5���� ������ �Ǿ��ٸ� �޴��ʿ��� �޾Ҵٴ� ACK�� 5���� ���ʷ�
// ������ �������� ���߿� �ް� �ȴ�. ���̱� �˰����� ����ϰ� �Ǹ� ó�� h�� ������ ACK�� �޴� ���ð����� ������ ello �����͸�
// ��Ŷ���� ����� ACK�� �ް� �Ǹ� �׶� ��Ŷ�� �����ش�. ���� 5���� ��Ŷ�� 2���� ��Ŷ���� ���� �ٿ��� ��Ʈ��ũ ������ �۽���
// ��ȿ������ ����� ���� �˰����̴�. ���� �� �˰��� ���� ��Ʈ��ũ�� ȿ������ ���������� ù �����͸� ������ ACK�� ��ٸ��� ������
// ��������� �ӵ��� �������� �ȴ�. ���� ���� �ɼ��� TCP_NODELAY�ɼ��� ���� ���̱� �˰����� �Ѱ� ���� �� �� �ִ�.

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

#pragma region �ۼ��Ź��� ũ�� ������
	int oldoptval, optval, optlen;

	SOCKET senrev_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (senrev_sock == INVALID_SOCKET) err_quit("socket()");
	// ���� ������ ũ�⸦ ��´�.
	optlen = sizeof(optval);
	retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&optval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("���� ���� ũ�� = %d����Ʈ\n", optval);

	oldoptval = optval;

	for (int i = 0; i < 3; i++)
	{
		// ���� ������ ũ�⸦ �� ��� �ø���.
		optval *= 2;
		retval = setsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		optlen = sizeof(optval);
		retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("���� ���� ũ�� = %d����Ʈ\n", optval);
	}

	// ���� ������ ũ�⸦ ���� ���� �Ѵ�.
	retval = setsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	optlen = sizeof(oldoptval);
	retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("���� ���� ũ�� = %d����Ʈ\n", oldoptval);
#pragma endregion
	
#pragma region SO_KEEPALIVE �ɼ� 
	// ���� ������ ó���ϱ� ���� �ֱ������� ������ �Ǿ� �ִٰ� ��ȣ�� �����־�� �Ѵ�.
	// ������� SO_KEEPALIVE �ɼ��� �Ҵ� ���ָ� �ȴ�. �̷��� ���ָ� �����ϱ� ���� �ؼ����ڸ� KEEPALIVE��� ��Ŷ�� �ڵ����� �����ִ�
	// �ɼ��̶� �����ϸ� �ȴ�. ������ �� ������ 2�ð��̰� �� ������ ���ָ� �ý����� ������Ʈ���� ����Ǿ��ְ� �� ����Ʈ���� �ٲٰ� �Ǵ°��̶� 
	// �ý��� ��ü�� ���Ͽ� ���ؼ� ����ȴ�. ���� �Ϲ������� ������� �ʰ� Windows 2000 ���ķ� SIO_KEEPALIVE_VALS �� �̿��� �� ���Ϻ���
	// KEEPALIVE�� ���� ������ ������ ����� �� �ִٰ� �Ѵ�.
#pragma endregion

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	DWORD dwRet;
	tcp_keepalive ka;
	ka.onoff = 1;
	ka.keepalivetime = 1000;		// 1�� ���� keepalive ��ȣ�� �����ٰ� ����
	ka.keepaliveinterval = 1000;	// ������ ������ 1�ʸ��� �������Ѵ�. ms tcp�� ��� 10ȸ ��õ� �Ѵ�.

	WSAIoctl(listen_sock, SIO_KEEPALIVE_VALS, &ka, sizeof(tcp_keepalive), 0, 0, &dwRet, NULL, NULL);


	// ���� ����
	WSACleanup();

	getchar();

	return 0;
}