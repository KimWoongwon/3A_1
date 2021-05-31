#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255

#define INTRO_MESSAGE "가위바위보 게임입니다.\n"
#define OTHER_WAIT_MESSAGE "다른 플레이어를 기다리는 중입니다.\n"
#define GAME_STATE_MESSAGE "게임을 시작합니다.\n1.가위\n2.바위\n3.보\n선택해주세요 : "
#define DRAW_MESSAGE "비겼습니다.\n"
#define WIN_MESSAGE "졌습니다.\n"
#define LOSE_MESSAGE "이겼습니다.\n"

enum STATE
{
	INTRO_SEND_STATE = 1,
	WAIT_STATE,
	GAME_INFO_STATE,
	GAME_STATE,
	GAME_END_STATE,
	DISCONNECTED_STATE
};

enum RESULT
{
	NODATA = -1,

	ACCEPT_SUCCESS,
	INTRO_SUCCESS,
	WAIT_SUCCESS,
	GAME_INFO_SUCCESS,
	GAME_LOSE,
	GAME_WIN,
	GAME_DRAW

};

enum PROTOCOL
{
	INTRO,
	WAIT,
	GAME_INFO,
	GAME_RESULT
};


enum
{
	SOC_ERROR = 1,
	SOC_TRUE,
	SOC_FALSE
};

enum IO_TYPE
{
	IO_RECV = 1,
	IO_SEND
};

struct _Client_info;
struct Game_Group;

struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;
	_Client_info*	  ptr;
	IO_TYPE       type;
};

struct _Client_info
{
	WSAOVERLAPPED_EX	r_overlapped;
	WSAOVERLAPPED_EX	s_overlapped;

	SOCKET			sock;
	SOCKADDR_IN		addr;
	STATE			state;
	bool			r_sizeflag;

	int				recvbytes;
	int				comp_recvbytes;
	int				sendbytes;
	int				comp_sendbytes;

	char			recvbuf[BUFSIZE];
	char			sendbuf[BUFSIZE];

	WSABUF			r_wsabuf;
	WSABUF			s_wsabuf;

	Game_Group* gameinfo;
	bool gamestated;

	HANDLE hEvent;
};

struct Game_Group
{
	_Client_info* player_01 = NULL;
	_Client_info* player_02 = NULL;

	bool gamestate;
	int playerindex;
};

Game_Group* Game_List[50];
int Group_Count = 0;

_Client_info* Client_List[100];
int Client_Count = 0;

CRITICAL_SECTION cs;

SOCKET client_sock;

// 비동기 입출력 시작과 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);

void err_quit(char* msg);
void err_display(char* msg);
//void err_display(int errcode);

void GetProtocol(char* _ptr, PROTOCOL& _protocol);

void PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, char* _str1, int& _size);

void UnPackPacket(char* _buf, char* _str1, char* _str2, char* _str3);
void UnPackPacket(char* _buf, char* _str1, char* _str2);
void UnPackPacket(char* _buf, int _val);

_Client_info* AddClientInfo(SOCKET _sock, SOCKADDR_IN _addr);
void RemoveClientInfo(_Client_info* _ptr);

void AddGameGroup(_Client_info* _ptr);
void RemoveGameGroup(_Client_info* _ptr);

bool Recv(_Client_info* _ptr);

int CompleteRecv(_Client_info* _ptr, int _completebyte);

bool Send(_Client_info* _ptr, int _size);

int CompleteSend(_Client_info* _ptr, int _completebyte);

void CompleteRecvProcess(_Client_info* _ptr);

void CompleteSendProcess(_Client_info* _ptr);

void IntroSendProcess(_Client_info* _ptr);
void WaitProcess(_Client_info* _ptr);
void GameInfoProcess(_Client_info* _ptr);
void GameProcess(_Client_info* _ptr);


int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// 입출력 완료 포트 생성
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(hcp == NULL) return 1;

	InitializeCriticalSection(&cs);
	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	// (CPU 개수 * 2)개의 작업자 스레드 생성
	HANDLE hThread;
	for(int i=0; i<(int)si.dwNumberOfProcessors*2; i++){
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if(hThread == NULL) return 1;
		CloseHandle(hThread);
	}

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}
		
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		_Client_info* ptr = AddClientInfo(client_sock, clientaddr);
		

		if (!Recv(ptr))
		{
			continue;
		}
		
	}

	DeleteCriticalSection(&cs);
	// 윈속 종료
	WSACleanup();
	return 0;
}

// 작업자 스레드 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;
	
	while(1)
	{
		// 비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		SOCKET client_sock;
		WSAOVERLAPPED_EX* overlapped;

		retval = GetQueuedCompletionStatus(hcp, &cbTransferred,
			(LPDWORD)&client_sock, (LPOVERLAPPED *)&overlapped, INFINITE);

		_Client_info* ptr = overlapped->ptr;		
		
		// 비동기 입출력 결과 확인
		if(retval == 0 || cbTransferred == 0)
		{
			if(retval == 0)			
			{				
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &overlapped->overlapped,
					&temp1, FALSE, &temp2);
				err_display("WSAGetOverlappedResult()");
			}

			ptr->state = DISCONNECTED_STATE;			
		}

		if (ptr->state == DISCONNECTED_STATE)
		{
			RemoveClientInfo(ptr);
			continue;
		}


		int result;

		switch (overlapped->type)
		{
		case IO_TYPE::IO_RECV:
			result = CompleteRecv(ptr, cbTransferred);
			switch (result)
			{
			case SOC_ERROR:
				continue;
			case SOC_FALSE:
				continue;
			case SOC_TRUE:
				break;
			}

			CompleteRecvProcess(ptr);

			if (!Recv(ptr))
			{
				continue;
			}

			break;
		case IO_TYPE::IO_SEND:
			result = CompleteSend(ptr, cbTransferred);
			switch (result)
			{
			case SOC_ERROR:
				continue;
			case SOC_FALSE:
				continue;
			case SOC_TRUE:
				break;
			}

			CompleteSendProcess(ptr);
			break;
		}
		
	}
	return 0;
}

void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

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
// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


void GetProtocol(char* _ptr, PROTOCOL& _protocol)
{
	memcpy(&_protocol, _ptr, sizeof(PROTOCOL));
}

void PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, char* _str1, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	_size = 0;

	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_result, sizeof(_result));
	ptr = ptr + sizeof(_result);
	_size = _size + sizeof(_result);

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

void UnPackPacket(char* _buf, char* _str1, char* _str2, char* _str3)
{
	int str1size, str2size, str3size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;

	memcpy(&str2size, ptr, sizeof(str2size));
	ptr = ptr + sizeof(str2size);

	memcpy(_str2, ptr, str2size);
	ptr = ptr + str2size;

	memcpy(&str3size, ptr, sizeof(str3size));
	ptr = ptr + sizeof(str3size);

	memcpy(_str3, ptr, str3size);
	ptr = ptr + str3size;
}

void UnPackPacket(char* _buf, char* _str1, char* _str2)
{
	int str1size, str2size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;

	memcpy(&str2size, ptr, sizeof(str2size));
	ptr = ptr + sizeof(str2size);

	memcpy(_str2, ptr, str2size);
	ptr = ptr + str2size;
}

void UnPackPacket(char* _buf, int* _val)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_val, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

}


_Client_info* AddClientInfo(SOCKET _sock, SOCKADDR_IN _addr)
{
	EnterCriticalSection(&cs);
	
	_Client_info* ptr = new _Client_info;
	memset(ptr, 0, sizeof(_Client_info));
	ptr->sock = _sock;
	memcpy(&ptr->addr, &_addr, sizeof(SOCKADDR_IN));
	ptr->r_sizeflag = false;
	ptr->state = INTRO_SEND_STATE;
	ptr->gamestated = false;
	ptr->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ptr->hEvent == NULL) return NULL;

	ptr->r_overlapped.ptr = ptr;
	ptr->r_overlapped.type = IO_TYPE::IO_RECV;

	ptr->s_overlapped.ptr = ptr;
	ptr->s_overlapped.type = IO_TYPE::IO_SEND;

	Client_List[Client_Count] = ptr;
	Client_Count++;
	LeaveCriticalSection(&cs);

	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(ptr->addr.sin_addr), ntohs(ptr->addr.sin_port));

	return ptr;
}

void RemoveClientInfo(_Client_info* _ptr)
{
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

	EnterCriticalSection(&cs);
	for (int i = 0; i < Client_Count; i++)
	{
		if (Client_List[i] == _ptr)
		{
			closesocket(Client_List[i]->sock);

			delete Client_List[i];

			for (int j = i; j < Client_Count - 1; j++)
			{
				Client_List[j] = Client_List[j + 1];
			}

			break;
		}
	}

	Client_Count--;
	LeaveCriticalSection(&cs);

}

void AddGameGroup(_Client_info* _ptr)
{
	EnterCriticalSection(&cs);

	Game_Group* gptr = NULL;

	for (int i = 0; i < Group_Count; i++)
	{
		if (Game_List[i]->playerindex != 2)
		{
			gptr = Game_List[i];
			break;
		}
	}

	if (gptr == NULL)
	{
		gptr = new Game_Group();

		

		gptr->player_01 = _ptr;
		gptr->player_02 = NULL;

		_ptr->gameinfo = gptr;

		gptr->gamestate = false;
		++gptr->playerindex;

		Game_List[Group_Count] = gptr;
		++Group_Count;
	}
	else
	{
		gptr->player_02 = _ptr;
		_ptr->gameinfo = gptr;

		gptr->gamestate = true;
		++gptr->playerindex;

		gptr->player_01->gamestated = true;
		gptr->player_02->gamestated = true;

		SetEvent(gptr->player_01->hEvent);
		SetEvent(gptr->player_02->hEvent);
	}

	LeaveCriticalSection(&cs);
}
void RemoveGameGroup(_Client_info* _ptr)
{
	EnterCriticalSection(&cs);
	for (int i = 0; i < Group_Count; i++)
	{
		if (Game_List[i] == _ptr->gameinfo)
		{
			delete Game_List[i];
			if(_ptr->gameinfo != NULL)
				delete _ptr->gameinfo;

			for (int j = i; j < Group_Count - 1; j++)
			{
				Game_List[j] = Game_List[j + 1];
			}

			break;
		}
	}

	Group_Count--;
	LeaveCriticalSection(&cs);
}

bool Recv(_Client_info* _ptr)
{
	int retval;
	DWORD recvbytes;
	DWORD flags = 0;

	ZeroMemory(&_ptr->r_overlapped.overlapped, sizeof(_ptr->r_overlapped.overlapped));

	_ptr->r_wsabuf.buf = _ptr->recvbuf + _ptr->comp_recvbytes;

	if (_ptr->r_sizeflag)
	{
		_ptr->r_wsabuf.len = _ptr->recvbytes - _ptr->comp_recvbytes;
	}
	else
	{
		_ptr->r_wsabuf.len = sizeof(int) - _ptr->comp_recvbytes;
	}

	retval = WSARecv(_ptr->sock, &_ptr->r_wsabuf, 1, &recvbytes,
		&flags, &_ptr->r_overlapped.overlapped, nullptr);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			//RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

int CompleteRecv(_Client_info* _ptr, int _completebyte)
{

	if (!_ptr->r_sizeflag)
	{
		_ptr->comp_recvbytes += _completebyte;

		if (_ptr->comp_recvbytes == sizeof(int))
		{
			memcpy(&_ptr->recvbytes, _ptr->recvbuf, sizeof(int));
			_ptr->comp_recvbytes = 0;
			_ptr->r_sizeflag = true;
		}

		if (!Recv(_ptr))
		{
			return SOC_ERROR;
		}

		return SOC_FALSE;
	}

	_ptr->comp_recvbytes += _completebyte;

	if (_ptr->comp_recvbytes == _ptr->recvbytes)
	{
		_ptr->comp_recvbytes = 0;
		_ptr->recvbytes = 0;
		_ptr->r_sizeflag = false;

		return SOC_TRUE;
	}
	else
	{
		if (!Recv(_ptr))
		{
			return SOC_ERROR;
		}

		return SOC_FALSE;
	}
}

bool Send(_Client_info* _ptr, int _size)
{
	int retval;
	DWORD sendbytes;
	DWORD flags;

	ZeroMemory(&_ptr->s_overlapped.overlapped, sizeof(_ptr->s_overlapped.overlapped));
	if (_ptr->sendbytes == 0)
	{
		_ptr->sendbytes = _size;
	}

	_ptr->s_wsabuf.buf = _ptr->sendbuf + _ptr->comp_sendbytes;
	_ptr->s_wsabuf.len = _ptr->sendbytes - _ptr->comp_sendbytes;

	retval = WSASend(_ptr->sock, &_ptr->s_wsabuf, 1, &sendbytes,
		0, &_ptr->s_overlapped.overlapped, nullptr);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSASend()");
			//RemoveClientInfo(_ptr);
			return false;
		}
	}

	return true;
}

int CompleteSend(_Client_info* _ptr, int _completebyte)
{
	_ptr->comp_sendbytes += _completebyte;
	if (_ptr->comp_sendbytes == _ptr->sendbytes)
	{
		_ptr->comp_sendbytes = 0;
		_ptr->sendbytes = 0;

		return SOC_TRUE;
	}
	if (!Send(_ptr, _ptr->sendbytes))
	{
		return SOC_ERROR;
	}

	return SOC_FALSE;
}

void CompleteRecvProcess(_Client_info* _ptr)
{
	RESULT join_result = NODATA;
	RESULT login_result = NODATA;

	char msg[BUFSIZE];
	PROTOCOL protocol;
	int size;

	GetProtocol(_ptr->recvbuf, protocol);

	switch (_ptr->state)
	{
	case INTRO_SEND_STATE:
		IntroSendProcess(_ptr);
		if(_ptr->gameinfo == NULL)
			AddGameGroup(_ptr);
		break;
	case WAIT_STATE:
		WaitProcess(_ptr);
		break;
	case GAME_INFO_STATE:
		WaitForSingleObject(_ptr->hEvent, INFINITE);
		GameInfoProcess(_ptr);
		break;
	case GAME_STATE:
		if (_ptr->gameinfo->gamestate == true)
			SetEvent(_ptr->hEvent);
		GameProcess(_ptr);
		break;
	case GAME_END_STATE:
		RemoveGameGroup(_ptr);
		break;
	}

}

void CompleteSendProcess(_Client_info* _ptr)
{
	switch (_ptr->state)
	{
	case INTRO_SEND_STATE:
		if(_ptr->gamestated == false)
			_ptr->state = WAIT_STATE; 
		else
			_ptr->state = GAME_INFO_STATE;
		break;
	case WAIT_STATE:
		_ptr->state = GAME_INFO_STATE;
		break;
	case GAME_INFO_STATE:
		_ptr->state = GAME_STATE;
		break;
	case GAME_STATE:
		_ptr->state = GAME_END_STATE;
		break;
	case GAME_END_STATE:
		_ptr->state = DISCONNECTED_STATE;
		break;
	}
}

void IntroSendProcess(_Client_info* _ptr)
{
	int size = 0;

	PackPacket(_ptr->sendbuf, INTRO, INTRO_SUCCESS, INTRO_MESSAGE, size);

	Send(_ptr, size);

	
}

void WaitProcess(_Client_info* _ptr)
{
	int size = 0;

	PackPacket(_ptr->sendbuf, WAIT, WAIT_SUCCESS, OTHER_WAIT_MESSAGE, size);

	Send(_ptr, size);

	
}

void GameInfoProcess(_Client_info* _ptr)
{
	int size = 0;

	PackPacket(_ptr->sendbuf, GAME_INFO, GAME_INFO_SUCCESS, GAME_STATE_MESSAGE, size);

	Send(_ptr, size);
}

void GameProcess(_Client_info* _ptr)
{
	int a = 0, b = 0;
	RESULT result_1, result_2;
	char msg_1[200];
	char msg_2[200];
	Game_Group* gptr = _ptr->gameinfo;

	PROTOCOL p;
	GetProtocol(gptr->player_01->recvbuf, p);

	UnPackPacket(gptr->player_01->recvbuf, &a);
	UnPackPacket(gptr->player_02->recvbuf, &b);

	WaitForSingleObject(gptr->player_01->hEvent, INFINITE);
	WaitForSingleObject(gptr->player_02->hEvent, INFINITE);
	
	if (a - b == 0)
	{
		result_1 = result_2 = GAME_DRAW;
		strcpy(msg_1, DRAW_MESSAGE);
		strcpy(msg_2, DRAW_MESSAGE);
	}
	else if (a - b == -1 || a - b == 2)
	{
		result_1 = GAME_LOSE;
		result_2 = GAME_WIN;
		strcpy(msg_1, LOSE_MESSAGE);
		strcpy(msg_2, WIN_MESSAGE);
	}
	else
	{
		result_1 = GAME_WIN;
		result_2 = GAME_LOSE;
		strcpy(msg_1, WIN_MESSAGE);
		strcpy(msg_2, LOSE_MESSAGE);
	}

	gptr->gamestate = false;

	int size_1 = 0, size_2 = 0;

	PackPacket(gptr->player_01->sendbuf, GAME_RESULT, result_1, msg_1, size_1);
	PackPacket(gptr->player_02->sendbuf, GAME_RESULT, result_2, msg_2, size_2);

	Send(gptr->player_01, size_1);
	Send(gptr->player_02, size_2);
}