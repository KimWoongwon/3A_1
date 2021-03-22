#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define REMOTEIP   "255.255.255.255"
#define REMOTEPORT 9000
#define BUFSIZE    512

#define MAXUSER 4
#define MESSAGE "��������\n1.�ڿ���\n2.����ȯ\n3.������\n4.ä����\n5.������\n"

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

	// ��ε�ĳ���� Ȱ��ȭ
	BOOL bEnable = TRUE;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		(char *)&bEnable, sizeof(bEnable));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ���� �ּ� ����ü �ʱ�ȭ
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_addr.s_addr = inet_addr(REMOTEIP);
	remoteaddr.sin_port = htons(REMOTEPORT);

	// ������ ��ſ� ����� ����
	SOCKADDR_IN peeraddr;
	char buf[BUFSIZE+1];
	int result[5];

	int len, count, temp;

	// ������ �Է�
	strcpy(buf, MESSAGE);

	// ������ ������
	retval = sendto(sock, buf, strlen(buf), 0,
		(SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
	if (retval == SOCKET_ERROR) {
		err_display("sendto()");
	}
	printf("[UDP] %d����Ʈ�� ���½��ϴ�.\n", retval);

	count = 0;
	ZeroMemory(buf, sizeof(buf));

	for (int i = 0; i < 5; i++)
		result[i] = 0;

	// ��ε�ĳ��Ʈ ������ ������
	while(1){
		if (count > MAXUSER - 1)
			break;

		int addrlen = sizeof(remoteaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(SOCKADDR*)&remoteaddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			break;
		}

		printf("[����������] %s \n", buf);
		temp = atoi(buf);
		if (temp >= 1 && temp <= 5)
		{
			result[temp - 1] += 1;
			++count;
		}
			
	}

	printf("�������� ���\n");
	printf("1. �ڿ���\t%d\t%f\n", result[0], ((float)result[0] / (float)MAXUSER) * 100);
	printf("2. ����ȯ\t%d\t%f\n", result[1], ((float)result[1] / (float)MAXUSER) * 100);
	printf("3. ������\t%d\t%f\n", result[2], ((float)result[2] / (float)MAXUSER) * 100);
	printf("4. ä����\t%d\t%f\n", result[3], ((float)result[3] / (float)MAXUSER) * 100);
	printf("5. ������\t%d\t%f\n", result[4], ((float)result[4] / (float)MAXUSER) * 100);

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}