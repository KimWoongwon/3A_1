#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define MULTICASTIP "235.7.8.9"
#define REMOTEPORT  9000
#define BUFSIZE     512

#define MESSAGE "ä�ù��� �����ϼ���!\n1.�ڿ��ù�\n2.����ȯ��\n3.��������\n"
#define ROOM_1 "235.7.8.10"
#define ROOM_2 "235.7.8.11"
#define ROOM_3 "235.7.8.12"

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

enum struct PROTOCOL
{
	CONNECT = 1,
	ENTER = 2,
	CHAT = 3
};

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");

	// ��Ƽĳ��Ʈ TTL ����
	int ttl = 2;
	retval = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ���� �ּ� ����ü �ʱ�ȭ
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(MULTICASTIP);
	remoteaddr.sin_port = htons(REMOTEPORT);

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	char buf[BUFSIZE+1];
	int len, addrlen;

	// ��Ƽĳ��Ʈ ������ ������
	while(1){
		// ������ �Է�
		ZeroMemory(buf, sizeof(buf));
		strcpy(buf, MESSAGE);
		//printf("\n[���� ������] %s\n");

		// ������ ������
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&remoteaddr, sizeof(remoteaddr));
		if(retval == SOCKET_ERROR){
			err_display("sendto()");
			continue;
		}
		printf("[UDP] %d����Ʈ�� ���½��ϴ�.\n", retval);

		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		int temp = atoi(buf);
		switch (temp)
		{
		case 1:
			strcpy(buf, ROOM_1);
			break;
		case 2:
			strcpy(buf, ROOM_2);
			break;
		case 3:
			strcpy(buf, ROOM_3);
			break;
		}
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			continue;
		}
		printf("[UDP] %d����Ʈ�� ���½��ϴ�.\n", retval);

	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}