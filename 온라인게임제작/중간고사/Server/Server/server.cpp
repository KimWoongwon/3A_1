#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255
#define ERROR_DISCONNECTED -2
#define DISCONNECTED -1
#define SOC_TRUE 1
#define SOC_FALSE 0

#define NICKNAME_MSG "대화명 입력이 성공적으로 완료 되었습니다.\r\n"
#define ROOM_SELECT_MSG "채팅방 번호를 입력해주세요.\r\n1.윤루트방\r\n2.딩셉션방\r\n3.주안방\r\n"
#define SELECT_ROOM_MSG "%d번 방에 입장하셨습니다.\r\n"

#define CHAT_ENTER_MSG "[ %s ]님이 입장하셨습니다.\r\n"
#define CHAT_QUIT_MSG "[ %s ]님이 퇴장하셨습니다.\r\n"
#define CHAT_ING_MSG "[ %s ] : %s\r\n"


enum STATE
{
	INIT_STATE = -1,

	NICKNAME_INPUT_STATE = 1,		// 닉네임 설정 상태
	NICKNAME_RESULT_STATE,			// 닉네임 설정 후 Send해주기 위한 중간 상태

	SELECT_ROOM_MSG_STATE,			// 방 안내 및 선택 메시지 보내주는 상태
	SELECT_ROOM_MSG_END_STATE,		// 메시지 보내준 후 Send해주기 위한 중간 상태

	SELECT_ROOM_STATE,				// 클라이언트가 선택한 방을 서버가 받아 방을 셋팅해주는 상태
	SELECT_ROOM_END_STATE,			// 셋팅이 끝난 후 다 됬다고 Send해주는 중간 상태

	CHAT_ENTER_STATE,				// 방 요청을 받아서 해당 방의 모든 클라이언트에게 입장메시지를 발송하기위한 준비 상태
	CHAT_ENTER_END_STATE,			// 패키징 해놨던 패킷을 보내기 위한 상태
	CHAT_ING_STATE,					// 클라이언트끼리의 통신을 위한 상태
	CHAT_QUIT_STATE,				// 클라이언트가 방을 나가고 새로운 방을 들어갈수 있게 셋팅해주는 상태
		
	DISCONNECTED_STATE,				// 연결 종료 상태
};

enum RESULT
{
	NODATA = -1,
		
	NICKNAME_SUCCESS = 1,			// 닉네임 설정이 완료 되었다고 클라이언트 측에 보내줄때 사용할 값

	SELECT_ROOM_MSGSEND,			// 방 리스트를 보내줄때 사용할 값
	SELECT_ROOM_SUCCESS,			// 방 선택이 잘 완료 되었다고 보내줄때 사용할 값

	CHAT_ENTER_SUCCESS,				// 방 입장이 잘 완료 됬고 입장 메시지를 보내줄때 사용할 값
	CHAT_MSG_SEND,					// 채팅을 주고받고 할때 사용할 값
	CHAT_QUIT_SUCCESS,				// 방을 나가는걸 정상적으로 완료했다고 보낼때 사용할값
	
};

enum ROOM
{
	ROOM_NODATA = -1,
	ROOM_01 = 1,
	ROOM_02,
	ROOM_03
};

enum PROTOCOL
{
	NONE = -1,

	// client send protocol
	NICKNAME = 1,
	SELECTROOMMSG,			// 방 리스트 출력해달라고 클라이언트가 보내는 프로토콜
	CHOICEROOMNUM,			// 1, 2, 3번 방을 골랐다고 클라이언트가 보내는 프로토콜
	CHATENTERREQUSET,		// 방에 입장 하겠다고 입장메시지 출력해달라고 클라이언트가 보내는 프로토콜
	CHATING,				// 채팅을 진행하는 중이라고 클라이언트가 보내는 프로토콜
	CHATQUIT,				// 채팅방에서 나가겠다고 클라이언트가 보내는 프로토콜

	// server send protocol
	NICK_RESULT,			// 닉네임 관련 셋팅이 처리되었다고 결과값과 함께 보내지는 프로토콜
	ROOM_RESULT,			// 방 관련 셋팅이 처리되었다고 결과값과 함께 보내지는 프로토콜
	CHAT_RESULT,			// 채팅 관련 처리가 완료되었다고 결과값과 함께 보내지는 프로토콜
};


struct _ClientInfo
{
	SOCKET		sock;
	SOCKADDR_IN addr;
	char		nickname[NICKNAMESIZE];

	STATE		state;

	bool		r_sizeflag;

	int			recvbytes;
	int			comp_recvbytes;
	int			sendbytes;
	int			comp_sendbytes;

	char		recvbuf[BUFSIZE];
	char		sendbuf[BUFSIZE];
	bool		isroomenter;		// 현재 방에 들어가 있는지 체크하기 위한 변수
	ROOM		curroomidx;			// 현재 몇번 방에 들어가있는지 확인하기 위한 변수
};

_ClientInfo* User_List[100];
int Count = 0;

// select 모델을 위한 변수
FD_SET Rset, Sset;

void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
void GetProtocol(const char* _ptr, PROTOCOL& _protocol);

int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, int _proid, const char* _str1);	// 클라이언트가 방 나가기 시에 해당 프로세스아이디를 같이 패킹하는 함수
int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, const char* _str1);				// 서버가 처리하는 각종 결과값과 거기에 맞는 프로토콜, 메시지가 패킹되는 함수

void UnPackPacket(const char* _buf, char* _str1);			// 클라이언트가 보내는 닉네임 정보와 채팅메시지 등을 저장 혹은 포멧에 맞게 처리해 주기 위해 언패킹 하는 함수
void UnPackPacket(const char* _buf, int* _num);				// 클라이언트가 선택한 방번호 혹은 클라이언트의 현재 프로세스아이디 값을 언패킹 하는 함수

int MessageRecv(_ClientInfo* _info);		
int MessageSend(_ClientInfo* _info);		
int PacketRecv(_ClientInfo* _ptr);			

_ClientInfo* AddClient(SOCKET _sock, SOCKADDR_IN _clientaddr);
void RemoveClient(_ClientInfo* _ptr);

void RecvPacketProcess(_ClientInfo* _ptr);
void SendPacketProcess(_ClientInfo* _ptr);

int main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	int retval;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		err_quit("WSAStartup() error!");

	// 소켓 생성
	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		err_quit("socket() error");

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9000);

	// 넌블로킹 소켓 설정
	u_long on = 1;
	retval = ioctlsocket(hServSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");

	// bind & listen
	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		err_quit("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		err_quit("listen() error");

	while (1)
	{
		FD_ZERO(&Rset);
		FD_ZERO(&Sset);

		FD_SET(hServSock, &Rset);

		for (int i = 0; i < Count; i++)
		{
			FD_SET(User_List[i]->sock, &Rset);

			switch (User_List[i]->state)
			{
			case NICKNAME_RESULT_STATE:
			case SELECT_ROOM_MSG_END_STATE:
			case SELECT_ROOM_END_STATE:
			case CHAT_ENTER_END_STATE:
			case CHAT_ING_STATE:
			case CHAT_QUIT_STATE:
				FD_SET(User_List[i]->sock, &Sset);
				break;
			}
		}

		if (select(0, &Rset, &Sset, 0, NULL) == SOCKET_ERROR)
		{
			err_quit("select() error");
		}

		if (FD_ISSET(hServSock, &Rset))
		{
			int clntLen = sizeof(clntAddr);
			hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntLen);
			_ClientInfo* ptr = AddClient(hClntSock, clntAddr);
			ptr->state = NICKNAME_INPUT_STATE;
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

			if (FD_ISSET(ptr->sock, &Sset))
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

void GetProtocol(const char* _ptr, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _ptr, sizeof(PROTOCOL));

}

int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, int _pro_id, const char* _str1)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	int size = 0;

	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(_protocol);

	memcpy(ptr, &_result, sizeof(_result));
	ptr = ptr + sizeof(_result);
	size = size + sizeof(_result);

	memcpy(ptr, &_pro_id, sizeof(_pro_id));
	ptr = ptr + sizeof(_pro_id);
	size = size + sizeof(_pro_id);

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
int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, const char* _str1)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	int size = 0;

	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(_protocol);

	memcpy(ptr, &_result, sizeof(_result));
	ptr = ptr + sizeof(_result);
	size = size + sizeof(_result);

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

void UnPackPacket(const char* _buf, char* _str1)
{
	int str1size;

	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}
void UnPackPacket(const char* _buf, int* _num)
{
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_num, ptr, sizeof(_num));
	ptr = ptr + sizeof(_num);

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
		DISCONNECTED;
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
	ptr->state = INIT_STATE;
	ptr->r_sizeflag = false;
	ptr->curroomidx = ROOM_NODATA;
	ptr->isroomenter = false;
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
	PROTOCOL protocol;

	GetProtocol(_ptr->recvbuf, protocol);

	int pro_id;
	char msg[255];
	char finalmsg[255];

	switch (_ptr->state)
	{
	case NICKNAME_INPUT_STATE:
		switch (protocol)
		{
		case NICKNAME:	// 클라이언트 측에서 닉네임 설정후 보내주면 받아서 중복검사 없이 그냥 셋팅후 완료 메시지를 보내준다.
			memset(_ptr->nickname, 0, sizeof(_ptr->nickname));
			UnPackPacket(_ptr->recvbuf, _ptr->nickname);
			_ptr->sendbytes = PackPacket(_ptr->sendbuf, NICK_RESULT, NICKNAME_SUCCESS, NICKNAME_MSG);
			_ptr->state = NICKNAME_RESULT_STATE;
			break;
		}
		break;
	case SELECT_ROOM_MSG_STATE:
		switch (protocol)
		{
		case SELECTROOMMSG:	// 클라이언트 측에서 방 리스트를 달라고 요청하면 방 리스트를 보내준다.
			_ptr->sendbytes = PackPacket(_ptr->sendbuf, ROOM_RESULT, SELECT_ROOM_MSGSEND, ROOM_SELECT_MSG);
			_ptr->state = SELECT_ROOM_MSG_END_STATE;
			break;
		}
		break;
	case SELECT_ROOM_STATE:
		switch (protocol)
		{
		case CHOICEROOMNUM:	// 클라이언트가 방번호를 입력해 보내주면 유저정보에 입력후 '%d번 방에 입장하셨습니다.' 라는 메시지를 보내준다.
			UnPackPacket(_ptr->recvbuf, (int*)&_ptr->curroomidx);
			_ptr->isroomenter = true;

			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, SELECT_ROOM_MSG, _ptr->curroomidx);

			_ptr->sendbytes = PackPacket(_ptr->sendbuf, ROOM_RESULT, SELECT_ROOM_SUCCESS, finalmsg);
			_ptr->state = SELECT_ROOM_END_STATE;
			break;
		}
		break;
	case CHAT_ENTER_STATE:
		switch (protocol)
		{
		case CHATENTERREQUSET:	
			// 클라이언트가 메시지를 받은후 확인 버튼을 누르면 입장 요청 프로토콜을 보내준다.
			// 그러면 해당 방의 모든 유저들에게 입장 메시지 출력 해준다.

			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, CHAT_ENTER_MSG, _ptr->nickname);

			// 본인을 포함한 해당 방에 같이 들어가 있는 모든 클라이언트에게 메시지를 뿌려준다.
			for (int i = 0; i < Count; i++)
			{
				if (User_List[i]->curroomidx == _ptr->curroomidx)
					User_List[i]->sendbytes = PackPacket(User_List[i]->sendbuf, CHAT_RESULT, CHAT_ENTER_SUCCESS, finalmsg);
			}

			_ptr->state = CHAT_ENTER_END_STATE;
			break;
		}
		break;
	case CHAT_ING_STATE:
		switch (protocol)
		{
		case CHATING:
			// 그후에는 채팅 내용을 전달하면서 중간에 클라이언트가 방나가기 버튼을 누를경우 CHATQUIT로 넘어간다.
			
			memset(msg, 0, sizeof(msg));
			UnPackPacket(_ptr->recvbuf, msg);
						
			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, CHAT_ING_MSG, _ptr->nickname, msg);
			// 채팅 내용 전송도 마찬가지로 자신을 포함한 해당방의 입장한 모든 클라이언트에게 뿌려준다.
			for (int i = 0; i < Count; i++)
			{
				if (User_List[i]->curroomidx == _ptr->curroomidx)
				{
					ZeroMemory(User_List[i]->sendbuf, sizeof(User_List[i]->sendbuf));
					User_List[i]->sendbytes = PackPacket(User_List[i]->sendbuf, CHAT_RESULT, CHAT_MSG_SEND, finalmsg);
				}
					
			}
			_ptr->state = CHAT_ING_STATE;
			break;
		case CHATQUIT:	
			// CHAT_ING_STATE에서는 QUIT에 대한 처리를 하지않고 CHAT_QUIT_STATE로 넘겨준다.
			// 따라서 클라이언트 측은 send를 2번해서 CHAT_QUIT_STATE로 넘어갈수 있게 해준다.
			_ptr->state = CHAT_QUIT_STATE;
			_ptr->isroomenter = false;
			break;
		}
		

		break;
	case CHAT_QUIT_STATE:
		switch (protocol)	
		{
		case CHATQUIT:
			// 두번 연속 받은 CHATQUIT 프로토콜
			// 모든 클라이언트에게 나가기요청 한 프로세스의 아이디와 함께 포멧에 맞게 수정된 퇴장메세지를 출력한다.
			// 혹여나 클라이언트 측에서 방나가기 요청을 한적이 없는데도 방이 나가지지 않게 
			// 받은 아이디가 자신의 프로세스 아이디와 같다면 방나가기 하는것으로 간주하게 클라이언트에서 처리한다.
			UnPackPacket(_ptr->recvbuf, &pro_id);

			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, CHAT_QUIT_MSG, _ptr->nickname);

			for (int i = 0; i < Count; i++)
			{
				if (User_List[i]->curroomidx == _ptr->curroomidx)
				{
					User_List[i]->sendbytes = PackPacket(User_List[i]->sendbuf,
						CHAT_RESULT, CHAT_QUIT_SUCCESS, pro_id, finalmsg);
				}
			}

			_ptr->curroomidx = ROOM_NODATA;
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
	case NICKNAME_RESULT_STATE:
		_ptr->state = SELECT_ROOM_MSG_STATE;
		break;
	case SELECT_ROOM_MSG_END_STATE:
		_ptr->state = SELECT_ROOM_STATE;
		break;
	case SELECT_ROOM_END_STATE:
		_ptr->state = CHAT_ENTER_STATE;
		break;
	case CHAT_ENTER_END_STATE:
		_ptr->state = CHAT_ING_STATE;
		break;
	case CHAT_ING_STATE:
		/*if (!_ptr->isroomenter)
			_ptr->state = CHAT_QUIT_STATE;*/
		break;
	case CHAT_QUIT_STATE:
		_ptr->state = SELECT_ROOM_MSG_STATE;
		break;
	}
}