#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9900
#define NICK_SIZE 20
#define MAXUSER 10

#define BUFSIZE 512
#define INTROMSG "대화명을 입력해주세요\r\n"
#define ROOMMSG "채팅방을 고르세요!!! \r\n1.박용택방\r\n2.오지환방\r\n3.김현수방\r\n"
#define ERROR_MSG "잘못된 방번호 입니다.\r\n"

enum PROTOCOL
{
	ERR = -1,
	INTRO = 1,
	SELECT,
	CHAT,
	CHATQUIT,
	ADDR,
	DISCONNECT,
};

enum STATE
{
	INIT_STATE,
	NICK_STATE,
	SELECT_STATE,
	ADDR_STATE,
	CHAT_STATE,
	QUIT_STATE,
	CONNECT_END_STATE,
};

struct _RequestPacket
{
	PROTOCOL protocol;
	int Data;
}RequestPacket;

struct _ClientInfo
{
	SOCKET sock;
	SOCKADDR_IN clientaddr;
	STATE state;
	char  nickname[NICK_SIZE];
	char  sendbuf[BUFSIZE];
	char  recvbuf[BUFSIZE];
};

CRITICAL_SECTION cs;
_ClientInfo* ClientInfo[MAXUSER];
int Client_Count = 0;

SOCKADDR_IN chatR_1, chatR_2, chatR_3;
SOCKADDR_IN multiaddr;
// 소켓 함수 오류 출력 후 종료
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
// 소켓 함수 오류 출력
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
int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
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

bool PacketRecv(SOCKET _sock, char* _buf)
{
	int size;

	int retval = recvn(_sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("packetsize recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(_sock, _buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv error()");
		return false;

	}
	else if (retval == 0)
	{
		return false;
	}

	return true;
}
PROTOCOL GetProtocol(const char* _ptr)
{
	PROTOCOL protocol;
	memcpy(&protocol, _ptr, sizeof(PROTOCOL));

	return protocol;
}

_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	printf("\nClient 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	_ClientInfo* ptr = new _ClientInfo;
	ZeroMemory(ptr, sizeof(_ClientInfo));
	ptr->sock = sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));

	ptr->state = INIT_STATE;
	ClientInfo[Client_Count++] = ptr;

	return ptr;
}
void RemoveClient(_ClientInfo* ptr)
{
	closesocket(ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->clientaddr.sin_addr),
		ntohs(ptr->clientaddr.sin_port));

	for (int i = 0; i < Client_Count; i++)
	{
		if (ClientInfo[i] == ptr)
		{
			delete ptr;
			int j;
			for (j = i; j < Client_Count - 1; j++)
			{
				ClientInfo[j] = ClientInfo[j + 1];
			}
			ClientInfo[j] = nullptr;
			break;
		}
	}

	Client_Count--;
}

int PackPacket(char* _buf, PROTOCOL _protocol)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(size);

	ptr = _buf;

	memcpy(ptr, &size, sizeof(size));
	size = size + sizeof(size);
	return size;
}
int PackPacket(char* _buf, PROTOCOL _protocol, const char* _str)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(size);

	int strsize1 = strlen(_str);
	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);
	size = size + sizeof(strsize1);

	memcpy(ptr, _str, strsize1);
	ptr = ptr + strsize1;
	size = size + strsize1;

	ptr = _buf;

	memcpy(ptr, &size, sizeof(size));
	size = size + sizeof(size);
	return size;
}
int PackPacket(char* _buf, PROTOCOL _protocol, SOCKADDR_IN* _addr)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(size);

	memcpy(ptr, _addr, sizeof(SOCKADDR_IN));
	ptr = ptr + sizeof(SOCKADDR_IN);
	size = size + sizeof(SOCKADDR_IN);

	ptr = _buf;

	memcpy(ptr, &size, sizeof(size));
	size = size + sizeof(size);
	return size;
}

void UnPackPacket(const char* _buf, int* _data)
{
	int strsize;
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_data, ptr, sizeof(_data));
	ptr = ptr + sizeof(_data);
}
void UnPackPacket(const char* _buf, char* _str)
{
	int strsize;
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&strsize, ptr, sizeof(strsize));
	ptr = ptr + sizeof(strsize);

	memcpy(_str, ptr, strsize);
	ptr = ptr + strsize;
}

DWORD CALLBACK ProcessClient(LPVOID  _ptr)
{
	_ClientInfo* Client_ptr = (_ClientInfo*)_ptr;

	int size;
	char nickbuf[20];
	PROTOCOL protocol;

	bool breakflag = false;

	while (1)
	{

		switch (Client_ptr->state)
		{
		case INIT_STATE:
			size = PackPacket(Client_ptr->sendbuf, INTRO, INTROMSG);
			if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("intro Send()");
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}
			Client_ptr->state = NICK_STATE;
			break;
		case NICK_STATE:
			if (!PacketRecv(Client_ptr->sock, Client_ptr->recvbuf))
			{
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}

			if (GetProtocol(Client_ptr->recvbuf) == DISCONNECT)
			{
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}

			UnPackPacket(Client_ptr->recvbuf, Client_ptr->nickname);
			Client_ptr->state = SELECT_STATE;
			break;
		case SELECT_STATE:
			size = PackPacket(Client_ptr->sendbuf, SELECT, ROOMMSG);
			if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("intro Send()");
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}
			Client_ptr->state = ADDR_STATE;
			break;
		case ADDR_STATE:
			if (!PacketRecv(Client_ptr->sock, Client_ptr->recvbuf))
			{
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}

			protocol = GetProtocol(Client_ptr->recvbuf);

			switch (protocol)
			{
			//case CHAT:
			//	/*size = PackPacket(Client_ptr->sendbuf, INTRO, INTROMSG);
			//	if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
			//	{
			//		err_display("intro Send()");
			//		Client_ptr->state = CONNECT_END_STATE;
			//		break;
			//	}*/
			//	break;
			case ADDR:
				ZeroMemory(&multiaddr, sizeof(multiaddr));
				UnPackPacket(Client_ptr->recvbuf, &RequestPacket.Data);

				switch (RequestPacket.Data)
				{
				case 1:
					memcpy(&multiaddr, &chatR_1, sizeof(SOCKADDR));
					break;
				case 2:
					memcpy(&multiaddr, &chatR_2, sizeof(SOCKADDR));
					break;
				case 3:
					memcpy(&multiaddr, &chatR_3, sizeof(SOCKADDR));
					break;
				default:
					Client_ptr->state = SELECT_STATE;
					RequestPacket.Data = -1;
					break;
				}

				if (RequestPacket.Data == -1)
				{
					size = PackPacket(Client_ptr->sendbuf, ERR, ERROR_MSG);
					if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
					{
						err_display("intro Send()");
						Client_ptr->state = CONNECT_END_STATE;
					}
					break;
				}

				size = PackPacket(Client_ptr->sendbuf, ADDR, &multiaddr);
				if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
				{
					err_display("intro Send()");
					Client_ptr->state = CONNECT_END_STATE;
					break;
				}

				printf("[UDP/%s:%d] %d번방 선택\n", inet_ntoa(Client_ptr->clientaddr.sin_addr),
					ntohs(Client_ptr->clientaddr.sin_port), RequestPacket.Data);
				Client_ptr->state = CHAT_STATE;
				break;
			case DISCONNECT:
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}
			break;
		case CHAT_STATE:
			if (!PacketRecv(Client_ptr->sock, Client_ptr->recvbuf))
			{
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}

			protocol = GetProtocol(Client_ptr->recvbuf);

			switch (protocol)
			{
			case CHATQUIT:
				Client_ptr->state = QUIT_STATE;
				size = PackPacket(Client_ptr->sendbuf, CHATQUIT);
				if (send(Client_ptr->sock, Client_ptr->sendbuf, size, 0) == SOCKET_ERROR)
				{
					err_display("connect end Send()");
				}
				break;
			case DISCONNECT:
				Client_ptr->state = CONNECT_END_STATE;
				break;
			}
			break;
		case QUIT_STATE:
			Client_ptr->state = SELECT_STATE;
			break;
		case CONNECT_END_STATE:
			RemoveClient(Client_ptr);
			breakflag = true;
			break;
		}

		if (breakflag)
		{
			break;
		}

	}


	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");


	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// room setting
	
	ZeroMemory(&chatR_1, sizeof(chatR_1));
	chatR_1.sin_family = AF_INET;
	chatR_1.sin_port = htons(9001);
	chatR_1.sin_addr.s_addr = inet_addr("235.7.8.9");

	ZeroMemory(&chatR_2, sizeof(chatR_2));
	chatR_2.sin_family = AF_INET;
	chatR_2.sin_port = htons(9002);
	chatR_2.sin_addr.s_addr = inet_addr("235.7.8.10");

	ZeroMemory(&chatR_3, sizeof(chatR_3));
	chatR_3.sin_family = AF_INET;
	chatR_3.sin_port = htons(9003);
	chatR_3.sin_addr.s_addr = inet_addr("235.7.8.11");




	// 데이터 통신에 사용할 변수
	SOCKET sock;
	SOCKADDR_IN clientaddr, multiaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 멀티캐스트 데이터 받기
	while (1)
	{
		// 데이터 받기
		addrlen = sizeof(clientaddr);
		sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}

		_ClientInfo* ptr = AddClient(sock, clientaddr);

		HANDLE hThread = CreateThread(NULL, 0, ProcessClient, ptr, 0, nullptr);
		if (hThread != NULL)
		{
			CloseHandle(hThread);
		}

		/*retval = recvfrom(sock, (char*)&RequestPacket, sizeof(RequestPacket), 0,
			(SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}

		switch (RequestPacket.protocol)
		{
		case CHATINFOREQ:
			retval = sendto(sock, msg, strlen(msg), 0, (SOCKADDR*)&peeraddr, sizeof(peeraddr));
			if (retval == SOCKET_ERROR)
			{
				err_display("recvfrom()");
				continue;
			}

			break;
		case ADDRREQ:
			switch (RequestPacket.Data)
			{
			case 1:
				memcpy(&multiaddr, &chatR_1, sizeof(SOCKADDR));
				break;
			case 2:
				memcpy(&multiaddr, &chatR_2, sizeof(SOCKADDR));
				break;
			case 3:
				memcpy(&multiaddr, &chatR_3, sizeof(SOCKADDR));
				break;
			}

			retval = sendto(sock, (char*)&multiaddr, sizeof(SOCKADDR), 0, (SOCKADDR*)&peeraddr, sizeof(peeraddr));
			if (retval == SOCKET_ERROR)
			{
				err_display("recvfrom()");
				continue;
			}
			buf[retval] = '\0';
			printf("[UDP/%s:%d] %d번방 선택\n", inet_ntoa(peeraddr.sin_addr),
				ntohs(peeraddr.sin_port), RequestPacket.Data);
			break;
		}*/
	}
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}