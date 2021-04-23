/*
 * echo_selserv_win.c
 * Written by SW. YOON
 */

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <list>

#define BUFSIZE 4096
#define ERROR_DISCONNECTED -2
#define DISCONNECTED -1
#define SOC_TRUE 1
#define SOC_FALSE 0
#define NODATA -1

#define BUFSIZE 4096
#define NICKNAMESIZE 255
#define CHAT_ROOM_LIST_MSG "채팅방을 선택하세요\r\n1.박용택방\r\n2.오지환방\r\n3.김현수방\r\n"

const char* chat_room_name[3] = {"박용택방", "오지환방", "김현수방"};

enum STATE
{
	INITE_STATE=1,
	CHAT_ROOM_LIST_SEND_STATE,
	CHAT_ROOM_SELECT_SATE,
	CHATTING_SEND_STATE,
	CHATTING_RECV_STATE,
	DISCONNECTED_STATE, 
	ERROR_CODE_SEND_STATE
};

enum PROTOCOL
{
	INPUT_DATA_ERROR = -1, 
	REQ_CHAT_ROOM_LIST = 1,
	CHAT_ROOM_LIST,
	CHAT_ROOM_SELECT,

	CHAT_ROOM_ENTER,
	CHAT_ROOM_ENTER_COMPLETE,
	CHAT_MSG,
	CHAT_ROOM_EXIT,
	CHAT_ROOM_EXIT_COMPLETE
};

enum ERROR_CODE
{
	NO_CODE=-1,
	CHAT_ROOM_SELECT_ERROR=-2
};

struct _ClientInfo;

struct _ChatRoom
{
	const char* chat_room_name;	
	//int		chat_room_num;
	std::list<_ClientInfo*>  chat_list;	
	std::list<_ClientInfo*>::iterator s_iter;
	bool s_flag;	

	_ChatRoom(const char* _roomname)
	{
		chat_room_name = _roomname;	
		s_flag = false;		
	}

	void enter_chat_room(_ClientInfo* _ptr)
	{
		chat_list.push_back(_ptr);		
	}

	bool exit_chat_room(_ClientInfo* _ptr)
	{		
		std::list<_ClientInfo*>::iterator iter;

		for (iter = chat_list.begin(); iter != chat_list.end(); iter++)
		{
			if ((*iter) == _ptr)
			{
				chat_list.erase(iter);				
				return true;
			}
		}
		return false;
	}

	_ClientInfo* get_chat_user()
	{
		if (!chat_list.empty())
		{
			if (!s_flag)
			{
				s_iter = chat_list.begin();
				s_flag = true;
			}

			if (s_iter != chat_list.end())
			{
				return (*s_iter++);
			}			
		}
		
		return nullptr;

	}

	void get_chat_user_end()
	{
		s_flag = false;
	}
};

_ChatRoom* ChatRoom[3];

struct _ClientInfo
{
	SOCKET			sock;
	SOCKADDR_IN		addr;
	STATE			state;
	bool			r_sizeflag;
	ERROR_CODE		last_error;

	char			nick_name[NICKNAMESIZE];
	_ChatRoom*		chat_room;

	int				recvbytes;
	int				comp_recvbytes;
	int				sendbytes;
	int				comp_sendbytes;

	char			recvbuf[BUFSIZE];
	char			sendbuf[BUFSIZE];

};

_ClientInfo* User_List[100];
int Count = 0;

FD_SET Rset, Wset;

void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
_ClientInfo* AddClient(SOCKET sock, SOCKADDR_IN clientaddr);
void RemoveClient(_ClientInfo*);


PROTOCOL GetProtocol(const char* _ptr);
int PackPacket(char*, PROTOCOL, const char*);
int PackPacket(char*, PROTOCOL);

void UnPackPacket(char*, char*);
void UnPackPacket(char*, int&);
void UnPackPacket(char* _buf, int& _data, char* _str1);

int MessageRecv(_ClientInfo* _info);
int MessageSend(_ClientInfo* _info);
int PacketRecv(_ClientInfo* _ptr);

void RecvPacketProcess(_ClientInfo* _ptr);
void SendPacketProcess(_ClientInfo* _ptr);

void InitializeChatRoom();
void RemoveChatRoom();
void EnterChatRoomProcess(_ClientInfo* _ptr, int _roomnumber, const char* _msg);
void ExitChatRoomProcess(_ClientInfo* _ptr);
void ChatMessageProcess(_ClientInfo* _ptr, const char* _msg);

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
	servAddr.sin_port = htons(9900);

	u_long on = 1;
	retval = ioctlsocket(hServSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");


	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		err_quit("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		err_quit("listen() error");
		
	InitializeChatRoom();

	while (1)
	{
		FD_ZERO(&Rset);
		FD_ZERO(&Wset);

		FD_SET(hServSock, &Rset);

		for (int i = 0; i < Count; i++)
		{
			FD_SET(User_List[i]->sock, &Rset);

			switch (User_List[i]->state)
			{
			case CHAT_ROOM_LIST_SEND_STATE:
			case CHATTING_SEND_STATE:	
			case ERROR_CODE_SEND_STATE:
				FD_SET(User_List[i]->sock, &Wset);
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
			continue;
		}		

		for (int i = 0; i < Count; i++)
		{
			_ClientInfo* ptr = User_List[i];

			if (FD_ISSET(ptr->sock, &Rset))
			{				
				int result = PacketRecv(ptr);

				switch (result)
				{
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
				int result = MessageSend(ptr);
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

	RemoveChatRoom();

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
	int retval = recv(_info->sock, _info->recvbuf + _info->comp_recvbytes, _info->recvbytes - _info->comp_recvbytes, 0);
	if (retval == SOCKET_ERROR) //강제연결종료요청인 경우
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
	}

}

int MessageSend(_ClientInfo* _info)
{
	int retval = send(_info->sock, _info->sendbuf + _info->comp_sendbytes,
		_info->sendbytes - _info->comp_sendbytes, 0);
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
		_info->comp_sendbytes = _info->comp_sendbytes + retval;

		if (_info->sendbytes == _info->comp_sendbytes)
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

int PacketRecv(_ClientInfo* _ptr)
{
	if (!_ptr->r_sizeflag)
	{
		_ptr->recvbytes = sizeof(int);
		int retval = MessageRecv(_ptr);
		switch (retval)
		{
		case SOC_TRUE:
			memcpy(&_ptr->recvbytes, _ptr->recvbuf, sizeof(int));
			_ptr->r_sizeflag = true;
			return SOC_FALSE;
		case SOC_FALSE:
			return SOC_FALSE;
		case ERROR_DISCONNECTED:
			err_display("recv error()");
			return DISCONNECTED;
		case DISCONNECTED:
			return DISCONNECTED;
		}
	}

	int retval = MessageRecv(_ptr);
	switch (retval)
	{
	case SOC_TRUE:
		_ptr->r_sizeflag = false;
		return SOC_TRUE;
	case SOC_FALSE:
		return SOC_FALSE;
	case ERROR_DISCONNECTED:
		err_display("recv error()");
		return DISCONNECTED;
	case DISCONNECTED:
		return DISCONNECTED;
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
	ptr->state = INITE_STATE;
	ptr->r_sizeflag = false;
	ptr->last_error = NO_CODE;
	User_List[Count++] = ptr;
	return ptr;
}

void RemoveClient(_ClientInfo* _ptr)
{
	closesocket(_ptr->sock);

	printf("\nClient 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

	for (int i = 0; i < Count; i++)
	{
		if (User_List[i] == _ptr)
		{
			delete User_List[i];

			for (int j = i; j < Count - 1; j++)
			{
				User_List[j] = User_List[j + 1];
			}
			User_List[Count - 1] = nullptr;
			Count--;
			break;
		}
	}

}

void RecvPacketProcess(_ClientInfo* _ptr)
{
	PROTOCOL protocol = GetProtocol(_ptr->recvbuf);	

	int room_num = NODATA;
	char msg[BUFSIZE];
	memset(msg, 0, sizeof(msg));

	switch (_ptr->state)
	{
	case INITE_STATE:	

		switch (protocol)
		{
		case REQ_CHAT_ROOM_LIST:			
			_ptr->sendbytes = PackPacket(_ptr->sendbuf,CHAT_ROOM_LIST, CHAT_ROOM_LIST_MSG);
			_ptr->state = CHAT_ROOM_LIST_SEND_STATE;
			break;		
		}
		break;
	case CHAT_ROOM_SELECT_SATE:
		switch (protocol)
		{		
		case CHAT_ROOM_ENTER:
			UnPackPacket(_ptr->recvbuf, room_num, msg);
			EnterChatRoomProcess(_ptr, room_num, msg);
			break;
		}
		break;
	case CHATTING_RECV_STATE:
		switch (protocol)
		{
		case CHAT_MSG:
			UnPackPacket(_ptr->recvbuf, msg);
			ChatMessageProcess(_ptr, msg);
			break;
		case CHAT_ROOM_EXIT:
			ExitChatRoomProcess(_ptr);
			break;

		}
		break;
	}

}

void SendPacketProcess(_ClientInfo* _ptr)
{
	PROTOCOL protocol;

	switch (_ptr->state)
	{
	case CHAT_ROOM_LIST_SEND_STATE:
		_ptr->state = CHAT_ROOM_SELECT_SATE;
		break;
	case CHATTING_SEND_STATE:
		if (_ptr->chat_room != nullptr)
		{
			_ptr->state = CHATTING_RECV_STATE;
		}
		else
		{
			_ptr->state = INITE_STATE;
		}
		
		break;
	case ERROR_CODE_SEND_STATE:
		switch (_ptr->last_error)
		{
		case CHAT_ROOM_SELECT_ERROR:
			_ptr->state = CHAT_ROOM_SELECT_SATE;
		}
		break;
	
	}
}



PROTOCOL GetProtocol(const char* _ptr)
{
	PROTOCOL protocol;
	memcpy(&protocol, _ptr, sizeof(PROTOCOL));
	return protocol;
}

int PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	int _size = 0;
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

	return _size;
}

int PackPacket(char* _buf, PROTOCOL _protocol)
{
	char* ptr = _buf;
	int _size = 0;
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);
	
	ptr = _buf;

	memcpy(ptr, &_size, sizeof(_size));
	_size = _size + sizeof(_size);
	return _size;
}



void UnPackPacket(char* _buf, char* _str1)
{
	int str1size, str2size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}

void UnPackPacket(char* _buf, int& _data)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_data, ptr, sizeof(_data));
	ptr = ptr + sizeof(_data);

}

void UnPackPacket(char* _buf, int& _data, char* _str1)
{
	int str1size;
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_data, ptr, sizeof(_data));
	ptr = ptr + sizeof(_data);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}

void InitializeChatRoom()
{
	for (int i = 0; i < 3; i++)
	{
		ChatRoom[i] = new _ChatRoom(chat_room_name[i]);
	}
}
void RemoveChatRoom()
{
	for (int i = 0; i < 3; i++)
	{
		delete ChatRoom[i];
	}
}

void EnterChatRoomProcess(_ClientInfo* _ptr, int _roomnumber, const char* _msg)
{
	if (_roomnumber < 1 || _roomnumber >3)
	{
		_ptr->sendbytes = PackPacket(_ptr->sendbuf, INPUT_DATA_ERROR);
		_ptr->state = ERROR_CODE_SEND_STATE;
		_ptr->last_error = CHAT_ROOM_SELECT_ERROR;
		return;
	}

	ChatRoom[_roomnumber - 1]->enter_chat_room(_ptr);
	_ptr->chat_room = ChatRoom[_roomnumber - 1];
	strcpy(_ptr->nick_name, _msg);

	char enter_message[BUFSIZE];
	memset(enter_message, 0, sizeof(enter_message));
	strcpy(enter_message, _ptr->nick_name);
	strcat(enter_message, "님이 입장하셨습니다.");

	while (1)
	{
		_ClientInfo* ptr = _ptr->chat_room->get_chat_user();
		if (ptr == nullptr)
		{
			_ptr->chat_room->get_chat_user_end();
			break;
		}
		if (ptr == _ptr)
		{
			ptr->sendbytes = PackPacket(ptr->sendbuf, CHAT_ROOM_ENTER_COMPLETE);
		}
		else
		{
			ptr->sendbytes = PackPacket(ptr->sendbuf, CHAT_MSG, enter_message);
		}

		ptr->state = CHATTING_SEND_STATE;
	}
}

void ExitChatRoomProcess(_ClientInfo* _ptr)
{
	_ChatRoom* room = _ptr->chat_room;	

	char exit_message[BUFSIZE];
	memset(exit_message, 0, sizeof(exit_message));
	strcpy(exit_message, _ptr->nick_name);
	strcat(exit_message, "님이 퇴장하셨습니다.");

	while (1)
	{
		_ClientInfo* ptr = room->get_chat_user();
		if (ptr == nullptr)
		{
			room->get_chat_user_end();
			break;
		}
		if (ptr == _ptr)
		{
			room->exit_chat_room(_ptr);
			_ptr->chat_room = nullptr;
			ptr->sendbytes = PackPacket(ptr->sendbuf, CHAT_ROOM_EXIT_COMPLETE);
		}
		else
		{
			ptr->sendbytes = PackPacket(ptr->sendbuf, CHAT_MSG, exit_message);
		}
		ptr->state = CHATTING_SEND_STATE;
	}
}


void ChatMessageProcess(_ClientInfo* _ptr, const char* _msg)
{
	_ChatRoom* room = _ptr->chat_room;


	while (1)
	{
		_ClientInfo* ptr = _ptr->chat_room->get_chat_user();
		if (ptr == nullptr)
		{
			_ptr->chat_room->get_chat_user_end();
			break;
		}
		ptr->sendbytes = PackPacket(ptr->sendbuf, CHAT_MSG, _msg);
		ptr->state = CHATTING_SEND_STATE;
	}
}
