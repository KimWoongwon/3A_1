#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096
#define NICKNAMESIZE 255
#define CHAT_ROOM_LIST_MSG "채팅방을 선택하세요\r\n1.박용택방\r\n2.오지환방\r\n3.김현수방\r\n"

enum STATE
{
	CHAT_ROOM_SELECT_SATE,
	CHATTING_STATE,
	DISCONNECTED_STATE
};

enum PROTOCOL
{
	REQ_CHAT_ROOM_LIST=1,
	CHAT_ROOM_LIST,
	CHAT_ROOM_SELECT,
	CHAT_ROOM_ADDR,

	CHAT_ROOM_ENTER,
	CHAT_ROOM_EXIT, 	
};

enum MSG_PROTOCOL
{
	CHAT_MSG = 1,
	EXIT
};

struct _ClientInfo
{
	SOCKET socket;
	SOCKADDR_IN clientaddr;	
	SOCKADDR_IN joinMultiaddr;	
	char nickname[NICKNAMESIZE];
	char buf[BUFSIZE];
	bool room_flag;
	STATE state;
};


void err_quit(char *msg);
void err_display(char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
DWORD WINAPI ProcessClient(LPVOID arg);
void RemoveClient(SOCKET sock);

bool PacketRecv(SOCKET, char*);

void GetProtocol(char* _ptr, PROTOCOL& _protocol);

void TCP_PackPacket(char*, PROTOCOL, const char*, int&);
void TCP_PackPacket(char*, PROTOCOL, SOCKADDR_IN&, int&);
void UDP_PackPacket(char* _buf, MSG_PROTOCOL _protocol, char* _msg, int& _size);

void TCP_UnPackPacket(char*, char*);
void TCP_UnPackPacket(char*, int&);


_ClientInfo* ClientInfo[100];
int count;

SOCKADDR_IN MultiCastAddr[3];

SOCKET udpSocket;

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == INVALID_SOCKET) err_quit("socket()");

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");	

	// bind()
	SOCKADDR_IN serveraddr;
	memset(&serveraddr, 0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9900);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;
	DWORD ThreadId;

	memset(MultiCastAddr,0, sizeof(MultiCastAddr));
	MultiCastAddr[0].sin_family=AF_INET;
	MultiCastAddr[0].sin_port=htons(9001);
	MultiCastAddr[0].sin_addr.s_addr=inet_addr("235.7.8.9");

	MultiCastAddr[1].sin_family=AF_INET;
	MultiCastAddr[1].sin_port=htons(9002);
	MultiCastAddr[1].sin_addr.s_addr=inet_addr("235.7.8.10");

	MultiCastAddr[2].sin_family=AF_INET;
	MultiCastAddr[2].sin_port=htons(9003);
	MultiCastAddr[2].sin_addr.s_addr=inet_addr("235.7.8.11");


	while(1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			continue;
		}
		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		_ClientInfo* ptr=AddClient(client_sock, clientaddr);

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient, 
			(LPVOID)ptr, 0, &ThreadId);
		if (hThread == NULL)
		{
			printf("[오류] 스레드 생성 실패!\n");
		}			
		else
		{
			CloseHandle(hThread);
		}
			
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	_ClientInfo* ClientPtr=(_ClientInfo*)arg;
	SOCKET client_sock=ClientPtr->socket;	
	
	int addrlen;
	int retval;	
	int size;

	bool endflg = false;

	while (1)
	{
		if (!PacketRecv(client_sock, ClientPtr->buf))
		{			
			ClientPtr->state = DISCONNECTED_STATE;
		}

		PROTOCOL protocol;
		if (ClientPtr->state != DISCONNECTED_STATE)
		{
			GetProtocol(ClientPtr->buf, protocol);
		}

		switch (ClientPtr->state)
		{
		case CHAT_ROOM_SELECT_SATE:

			switch (protocol)
			{
			case REQ_CHAT_ROOM_LIST:
				{
					char nickname[NICKNAMESIZE];
					memset(nickname, 0, sizeof(nickname));
					TCP_UnPackPacket(ClientPtr->buf, nickname);
					strcpy(ClientPtr->nickname, nickname);
				}

				TCP_PackPacket(ClientPtr->buf, CHAT_ROOM_LIST, CHAT_ROOM_LIST_MSG, size);

				retval = send(client_sock, ClientPtr->buf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("ChatInfoMessage send() error");
					ClientPtr->state = DISCONNECTED_STATE;
					break;
				}

				break;
			case CHAT_ROOM_SELECT:
				{
					int room_index;
					TCP_UnPackPacket(ClientPtr->buf, room_index);
					room_index--;
					memcpy(&ClientPtr->joinMultiaddr, &MultiCastAddr[room_index], sizeof(SOCKADDR_IN));

					TCP_PackPacket(ClientPtr->buf, CHAT_ROOM_LIST, ClientPtr->joinMultiaddr, size);

					retval = send(client_sock, ClientPtr->buf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("Multicast addr send() errror");
						ClientPtr->state = DISCONNECTED_STATE;
						break;
					}

					char EnterMessage[BUFSIZE];
					memset(EnterMessage, 0, sizeof(EnterMessage));
					strcpy(EnterMessage, ClientPtr->nickname);
					strcat(EnterMessage, "님이 입장하셨습니다.");

					UDP_PackPacket(ClientPtr->buf, CHAT_MSG, EnterMessage, size);

					retval = sendto(udpSocket, ClientPtr->buf, size, 0, (SOCKADDR*)&ClientPtr->joinMultiaddr, sizeof(SOCKADDR_IN));
					if (retval == SOCKET_ERROR)
					{
						err_display("EnterMessage send() error");
						ClientPtr->state = DISCONNECTED_STATE;
						break;
					}
				}
				ClientPtr->room_flag = true;
				ClientPtr->state = CHATTING_STATE;
				break;
			}

			

			break;

		case CHATTING_STATE:
			switch (protocol)
			{
			
			case CHAT_ROOM_EXIT:
				{
					char ExitMessage[BUFSIZE];
					memset(ExitMessage, 0, sizeof(ExitMessage));
					strcpy(ExitMessage, ClientPtr->nickname);
					strcat(ExitMessage, "님이 퇴장하셨습니다.");

					UDP_PackPacket(ClientPtr->buf, CHAT_MSG, ExitMessage, size);

					retval = sendto(udpSocket, ClientPtr->buf, size, 0, (SOCKADDR*)&ClientPtr->joinMultiaddr, sizeof(SOCKADDR_IN));
					if (retval == SOCKET_ERROR)
					{
						err_display("ExitMessage send() error");
						ClientPtr->state = DISCONNECTED_STATE;
						break;
					}
					ClientPtr->room_flag = false;
					ClientPtr->state= CHAT_ROOM_SELECT_SATE;
					memset(&(ClientPtr->joinMultiaddr), 0, sizeof(SOCKADDR_IN));
				}
				break;
			}

			break;

		case DISCONNECTED_STATE:
			
			RemoveClient(ClientPtr->socket);
			endflg = true;
			break;
		}

		if (endflg)
		{
			break;
		}
	}

	return 0;
}	


// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr)
{
	_ClientInfo* ptr=new _ClientInfo;
	memset(ptr,0, sizeof(_ClientInfo));
	ptr->socket=sock;
	memcpy(&(ptr->clientaddr), &clientaddr, sizeof(clientaddr));
	ptr->state = CHAT_ROOM_SELECT_SATE;
	ptr->room_flag = false;
	ClientInfo[count++] = ptr;
	return ptr;
}

void RemoveClient(SOCKET sock)
{
	for(int i=0; i<count; i++)
	{
		if(ClientInfo[i]->socket==sock)
		{
			closesocket(sock);
			printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", 
				inet_ntoa(ClientInfo[i]->clientaddr.sin_addr), 
				ntohs(ClientInfo[i]->clientaddr.sin_port));
			delete ClientInfo[i];
			for(int j=i;j<count-1; j++)
			{
				ClientInfo[j]=ClientInfo[j+1];
			}
			break;
		}
	}

	count--;	
}

int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0){
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

bool PacketRecv(SOCKET _sock, char* _buf)
{
	int size;

	int retval = recvn(_sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recv error()");
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

void GetProtocol(char* _ptr, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _ptr, sizeof(PROTOCOL));

}


void TCP_PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	_size = 0;
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &strsize1, sizeof(strsize1));
	ptr = ptr + sizeof(strsize1);
	_size = _size + sizeof(strsize1);

	memcpy(ptr, _str1, strsize1);
	ptr = ptr + strsize1;
	_size = _size + strsize1;

	ptr = _buf;

	memcpy(ptr, &_size, sizeof(_size));
	_size = _size + sizeof(_size);
}

void TCP_PackPacket(char* _buf, PROTOCOL _protocol, SOCKADDR_IN& _addr, int& _size)
{
	char* ptr = _buf;	
	_size = 0;
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);	

	memcpy(ptr, &_addr, sizeof(SOCKADDR_IN));
	ptr = ptr + sizeof(SOCKADDR_IN);
	_size = _size + sizeof(SOCKADDR_IN);

	ptr = _buf;

	memcpy(ptr, &_size, sizeof(_size));
	_size = _size + sizeof(_size);
}

void UDP_PackPacket(char* _buf, MSG_PROTOCOL _protocol, char* _msg, int& _size)
{
	char* ptr = _buf;
	int strsize = strlen(_msg);

	_size = 0;

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &strsize, sizeof(strsize));
	ptr = ptr + sizeof(strsize);
	_size = _size + sizeof(strsize);


	memcpy(ptr, _msg, strsize);	
	_size = _size + strsize;
}


void TCP_UnPackPacket(char* _buf, char* _str1)
{
	int str1size, str2size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;	
}

void TCP_UnPackPacket(char* _buf, int& _data)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_data, ptr, sizeof(_data));
	ptr = ptr + sizeof(_data);

}

