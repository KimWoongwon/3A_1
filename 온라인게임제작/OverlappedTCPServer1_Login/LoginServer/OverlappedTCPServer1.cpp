#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 4096
#define IDSIZE 255
#define PWSIZE 255
#define NICKNAMESIZE 255

#define ID_ERROR_MSG "없는 아이디입니다\n"
#define PW_ERROR_MSG "패스워드가 틀렸습니다.\n"
#define LOGIN_SUCCESS_MSG "로그인에 성공했습니다.\n"
#define ID_EXIST_MSG "이미 있는 아이디 입니다.\n"
#define JOIN_SUCCESS_MSG "가입에 성공했습니다.\n"
#define LOGOUT_MSG "로그아웃되었습니다.\n"


enum STATE
{	
	MENU_SELECT_STATE = 1,
	LOGIN_STATE,
	RESULT_SEND_STATE
};

enum RESULT
{
	NODATA = -1,
	ID_EXIST = 1,
	ID_ERROR,
	PW_ERROR,
	JOIN_SUCCESS,
	LOGIN_SUCCESS,
	LOGOUT_SUCCESS
};

enum PROTOCOL
{
	JOIN_INFO,
	LOGIN_INFO,
	JOIN_RESULT,
	LOGIN_RESULT,
	LOGOUT,
	LOGOUT_RESULT
};

enum
{
	SOC_ERROR=1, 
	SOC_TRUE, 
	SOC_FALSE
};

struct _User_Info
{
	char id[IDSIZE];
	char pw[PWSIZE];
	char nickname[NICKNAMESIZE];
};

struct _Client_info
{
	WSAOVERLAPPED	r_overlapped;
	WSAOVERLAPPED	s_overlapped;

	SOCKET			sock;
	SOCKADDR_IN		addr;
	_User_Info		userinfo;
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

	
	//WSAEVENT		hEvent;	
};

_User_Info* Join_List[100];
int Join_Count = 0;

_Client_info* Client_List[100];
int Client_Count = 0;

HANDLE hReadEvent, hWriteEvent;
//WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION cs;

// 비동기 입출력 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);
// 소켓 관리 함수

_Client_info* AddClientInfo(SOCKET _sock, SOCKADDR_IN _addr)
{
	EnterCriticalSection(&cs);
	if (Client_Count >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		return nullptr;
	}
	_Client_info* ptr = new _Client_info;
	memset(ptr, 0, sizeof(_Client_info));
	ptr->sock = _sock;	
	memcpy(&ptr->addr, &_addr, sizeof(SOCKADDR_IN));
	
	/*ptr->hEvent = WSACreateEvent();
	if (ptr->hEvent == WSA_INVALID_EVENT)
	{
		return nullptr;
	}	
	EventArray[Client_Count] = ptr->hEvent;*/
	ptr->r_sizeflag = false;

	ptr->state = MENU_SELECT_STATE;
	
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
			//WSACloseEvent(EventArray[i]);


			/*for (int j = i; j < Client_Count - 1; j++)
			{
				EventArray[j] = EventArray[j + 1];
			}*/

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

// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

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

BOOL SearchFile(char *filename)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(filename, &FindFileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return FALSE;
	else {
		FindClose(hFindFile);
		return TRUE;
	}
}

bool FileDataLoad()
{
	if (!SearchFile("UserInfo.info"))
	{
		FILE* fp = fopen("UserInfo.info", "wb");
		fclose(fp);
		return true;
	}

	FILE* fp = fopen("UserInfo.info", "rb");
	if (fp == NULL)
	{
		return false;
	}

	_User_Info info;
	memset(&info, 0, sizeof(_User_Info));

	while (1)
	{
		fread(&info, sizeof(_User_Info), 1, fp);
		if (feof(fp))
		{
			break;
		}
		_User_Info* ptr = new _User_Info;
		memcpy(ptr, &info, sizeof(_User_Info));
		Join_List[Join_Count++] = ptr;
	}

	fclose(fp);
	return true;
}

bool FileDataAdd(_User_Info* _info)
{
	EnterCriticalSection(&cs);
	FILE* fp = fopen("UserInfo.info", "ab");
	if (fp == NULL)
	{
		return false;
	}

	int retval = fwrite(_info, 1, sizeof(_User_Info), fp);

	if (retval != sizeof(_User_Info))
	{
		return false;
	}

	fclose(fp);
	LeaveCriticalSection(&cs);
	return true;
}

bool Recv(_Client_info* _ptr)
{
	int retval;
	DWORD recvbytes;
	DWORD flags=0;
	
	ZeroMemory(&_ptr->r_overlapped, sizeof(_ptr->r_overlapped));
	//_ptr->r_overlapped.hEvent = _ptr->hEvent;
	
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
		&flags, &_ptr->r_overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			RemoveClientInfo(_ptr);
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

	ZeroMemory(&_ptr->s_overlapped, sizeof(_ptr->s_overlapped));
	//_ptr->s_overlapped.hEvent = _ptr->hEvent;
	
	if (_ptr->sendbytes == 0)
	{
		_ptr->sendbytes = _size;
	}

	_ptr->s_wsabuf.buf = _ptr->sendbuf + _ptr->comp_sendbytes;
	_ptr->s_wsabuf.len = _ptr->sendbytes- _ptr->comp_sendbytes;

	retval = WSASend(_ptr->sock, &_ptr->s_wsabuf, 1, &sendbytes,
		0, &_ptr->s_overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSASend()");
			RemoveClientInfo(_ptr);
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

void JoinProcess(_Client_info* _ptr);

void LoginProcess(_Client_info* _ptr);

void LogoutProcess(_Client_info* _ptr);

int main(int argc, char *argv[])
{
	int retval;
	InitializeCriticalSection(&cs);

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

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

	if (!FileDataLoad())
	{
		return -1;
	}

	// 더미(dummy) 이벤트 객체 생성
	/*WSAEVENT hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
		err_quit("WSACreateEvent()");
	EventArray[Client_Count++] = hEvent;*/

	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;

	// 스레드 생성
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if(hThread == NULL) return 1;
	CloseHandle(hThread);

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while(1){

		WaitForSingleObject(hReadEvent, INFINITE);
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}
		// 소켓 정보 추가
		_Client_info* ptr = AddClientInfo(client_sock, clientaddr);
				
		if (!Recv(ptr))
		{
			continue;
		}
		// 소켓의 개수(nTotalSockets) 변화를 알림
		SetEvent(hWriteEvent);
		//WSASetEvent(EventArray[0]);
	}

	// 윈속 종료
	WSACleanup();
	DeleteCriticalSection(&cs);
	return 0;
}

// 비동기 입출력 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while(1)
	{
		while (1) {
			// alertable wait
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE);
			if (result == WAIT_OBJECT_0) break;
			if (result != WAIT_IO_COMPLETION) return 1;
		}

		// 이벤트 객체 관찰
		DWORD index = WSAWaitForMultipleEvents(Client_Count,
			EventArray, FALSE, WSA_INFINITE, FALSE);
		if(index == WSA_WAIT_FAILED) continue;
		index -= WSA_WAIT_EVENT_0;
		WSAResetEvent(EventArray[index]);
		if(index == 0) continue;

		// 클라이언트 정보 얻기
		_Client_info *ptr = Client_List[index];
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

		// 비동기 입출력 결과 확인
		DWORD cbTransferred, flags;
		LPWSAOVERLAPPED overlapped;
		int size;
		
		switch (ptr->state)
		{
		case LOGIN_STATE:
		case MENU_SELECT_STATE:
			overlapped = &ptr->r_overlapped;
			break;
		case RESULT_SEND_STATE:
			overlapped = &ptr->s_overlapped; 
			break;
		}

		retval = WSAGetOverlappedResult(ptr->sock, overlapped,
			&cbTransferred, FALSE, &flags);
		if(retval == FALSE || cbTransferred == 0){
			RemoveClientInfo(ptr);			
			continue;
		}
		
		switch (ptr->state)
		{
		case LOGIN_STATE:
		case MENU_SELECT_STATE:
			{
				int result = CompleteRecv(ptr, cbTransferred);
				switch (result)
				{
				case SOC_ERROR:
					continue;
				case SOC_FALSE:
					continue;
				case SOC_TRUE:
					break;
				}

				PROTOCOL protocol;
				GetProtocol(ptr->recvbuf, protocol);

				switch (protocol)
				{
				case JOIN_INFO:
					memset(&ptr->userinfo, 0, sizeof(_User_Info));
					UnPackPacket(ptr->recvbuf, ptr->userinfo.id, ptr->userinfo.pw, ptr->userinfo.nickname);
					JoinProcess(ptr);
					break;
				case LOGIN_INFO:
					memset(&ptr->userinfo, 0, sizeof(_User_Info));
					UnPackPacket(ptr->recvbuf, ptr->userinfo.id, ptr->userinfo.pw);
					LoginProcess(ptr);					
					break;
				case LOGOUT:
					LogoutProcess(ptr);
					break;
				}	
			}					

			break;
		case RESULT_SEND_STATE:
			{
				int result = CompleteSend(ptr, cbTransferred);
				switch (result)
				{
				case SOC_ERROR:
					continue;
				case SOC_FALSE:
					continue;
				case SOC_TRUE:
					break;
				}

				if (strlen(ptr->userinfo.id) == 0)
				{
					ptr->state = MENU_SELECT_STATE;
				}
				else
				{
					ptr->state = LOGIN_STATE;
				}			
			}

			Recv(ptr);
		
			break;
		}		
	}
	
}


void JoinProcess(_Client_info* _ptr)
{
	RESULT join_result = NODATA;
	char msg[BUFSIZE];
	int size;

	for (int i = 0; i < Join_Count; i++)
	{
		if (!strcmp(Join_List[i]->id, _ptr->userinfo.id))
		{
			join_result = ID_EXIST;
			strcpy(msg, ID_EXIST_MSG);
			break;
		}
	}

	if (join_result == NODATA)
	{
		_User_Info* user = new _User_Info;
		memset(user, 0, sizeof(_User_Info));
		strcpy(user->id, _ptr->userinfo.id);
		strcpy(user->pw, _ptr->userinfo.pw);
		strcpy(user->nickname, _ptr->userinfo.nickname);

		FileDataAdd(user);

		Join_List[Join_Count++] = user;
		join_result = JOIN_SUCCESS;
		strcpy(msg, JOIN_SUCCESS_MSG);
	}

	PROTOCOL protocol = JOIN_RESULT;


	PackPacket(_ptr->sendbuf, protocol, join_result, msg, size);

	_ptr->state = RESULT_SEND_STATE;

	Send(_ptr, size);
}

void LoginProcess(_Client_info* _ptr)
{
	RESULT login_result = NODATA;

	char msg[BUFSIZE];
	PROTOCOL protocol;
	int size;	

	for (int i = 0; i < Join_Count; i++)
	{
		if (!strcmp(Join_List[i]->id, _ptr->userinfo.id))
		{
			if (!strcmp(Join_List[i]->pw, _ptr->userinfo.pw))
			{
				login_result = LOGIN_SUCCESS;
				strcpy(msg, LOGIN_SUCCESS_MSG);
			}
			else
			{
				login_result = PW_ERROR;
				strcpy(msg, PW_ERROR_MSG);
			}
			break;
		}
	}

	if (login_result == NODATA)
	{
		login_result = ID_ERROR;
		strcpy(msg, ID_ERROR_MSG);
	}	
	
	_ptr->state = RESULT_SEND_STATE;

	protocol = LOGIN_RESULT;

	PackPacket(_ptr->sendbuf, protocol, login_result, msg, size);	

	Send(_ptr, size);
}

void LogoutProcess(_Client_info* _ptr)
{
	int size;

	memset(&_ptr->userinfo, 0, sizeof(_User_Info));

	_ptr->state = RESULT_SEND_STATE;
	
	PackPacket(_ptr->sendbuf, LOGOUT_RESULT, LOGOUT_SUCCESS, LOGOUT_MSG, size);
		
	Send(_ptr, size);
}

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