#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9900
#define BUFSIZE 512
#define NICKNAMESIZE 255
#define NODATA -1

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
	CHAT_STATE,
	QUIT_STATE,
	CONNECT_END_STATE,
};

struct _MyInfo
{
	SOCKET server_sock;
	SOCKET udp_sock;
	STATE state;
	char nickname[20];
	char sendbuf[4096];
	char recvbuf[4096];
	bool chat_state;
	bool quit_state;
}*MyInfo;

bool PacketRecv(SOCKET, char*);

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);
// 오류 출력 함수
void err_quit(const char*);
void err_display(const char*);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET , char*, int, int);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID);
DWORD CALLBACK RecvThread(LPVOID);

PROTOCOL GetProtocol(const char*);
int UDP_PackPacket(char*, PROTOCOL);
int UDP_PackPacket(char*, PROTOCOL, const char*);
int PackPacket(char*, PROTOCOL, const char*);
int PackPacket(char*, PROTOCOL);
int PackPacket(char*, PROTOCOL, int);

//void UDP_UnPackPacket(const char*, char*);
void UnPackPacket(const char*, SOCKADDR_IN*);
void UnPackPacket(const char*, char*, int&);
void UnPackPacket(const char*, char*);
void UnPackPacket(const char*, int&, char*);

void MakeChatMessage(const char*, const char*, char*);

char buf[BUFSIZE+1]; // 데이터 송수신 버퍼
char recvbuf[BUFSIZE + 1];
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND hQuitButton;	//나가기버튼
HWND hEdit1, hEdit2; // 편집 컨트롤
HANDLE hThread[2];

HWND hGdlg = nullptr;

struct ip_mreq mreq;
SOCKADDR_IN multicastaddr, localaddr, peeraddr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(hWriteEvent == NULL) return 1;
		
	// 소켓 통신 스레드 생성	
	// 대화상자 생성

	MyInfo = new _MyInfo;
	memset(MyInfo, 0, sizeof(_MyInfo));

	hThread[0] = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	WaitForMultipleObjects(2, hThread, true, INFINITE);

	// 이벤트 제거
	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	// closesocket()
	closesocket(MyInfo->udp_sock);
	closesocket(MyInfo->server_sock);

	delete MyInfo;
	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_INITDIALOG:
		hGdlg = hDlg;
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hSendButton = GetDlgItem(hDlg, IDOK);		
		hQuitButton = GetDlgItem(hDlg, ID_EXIT);
		EnableWindow(hQuitButton, FALSE);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		hThread[1] = CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);
		MyInfo->state = INIT_STATE;
		MyInfo->chat_state = FALSE;
		MyInfo->quit_state = FALSE;
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE+1);
			SetEvent(hWriteEvent); // 쓰기 완료 알리기
			SetWindowText(hEdit1, "");
			SetFocus(hEdit1);			
			return TRUE;
		case IDCANCEL:
			MyInfo->state = CONNECT_END_STATE;
			SetEvent(hWriteEvent);		
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case ID_EXIT:
			MyInfo->state = QUIT_STATE;
			MyInfo->quit_state = TRUE;
			SetEvent(hWriteEvent);
			EnableWindow(hQuitButton, FALSE);
			return TRUE;
		} 
		return FALSE;
	}
	return FALSE;
}

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE+256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
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
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
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

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	MyInfo->server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(MyInfo->server_sock == INVALID_SOCKET) err_quit("socket()");

	MyInfo->udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (MyInfo->udp_sock == INVALID_SOCKET) err_quit("socket()");

	BOOL optval = true;
	retval = setsockopt(MyInfo->udp_sock, SOL_SOCKET, SO_REUSEADDR,
		(char*)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("SO_REUSEADDR setsockopt()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(MyInfo->server_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	// 서버와 데이터 통신
	char msg[BUFSIZE + 1];
	int size;
	bool endflag = false;
	int data = NODATA;
	while(1)
	{		
		if(!MyInfo->quit_state)
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기

		// 문자열 길이가 0이면 보내지 않음
		if(!MyInfo->quit_state && MyInfo->state != CONNECT_END_STATE && strlen(buf) == 0)
		{
			EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
			SetEvent(hReadEvent); // 읽기 완료 알리기
			continue;
		}

		switch (MyInfo->state)
		{			
		case INIT_STATE:
			break;
		case NICK_STATE:
			memcpy(MyInfo->nickname, buf, sizeof(MyInfo->nickname));
			size = PackPacket(MyInfo->sendbuf, SELECT, buf);
			if (send(MyInfo->server_sock, MyInfo->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("nickname Send()");
			}
			MyInfo->state = SELECT_STATE;
			break;
		case SELECT_STATE:
			data = _ttoi(buf);
			size = PackPacket(MyInfo->sendbuf, ADDR, data);
			if (send(MyInfo->server_sock, MyInfo->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("Room Number Send()");
			}

			break;
		case CHAT_STATE:
			ZeroMemory(&MyInfo->sendbuf, sizeof(MyInfo->sendbuf));

			MakeChatMessage(MyInfo->nickname, buf, msg);
			size = UDP_PackPacket(MyInfo->sendbuf, CHAT, msg);
			retval = sendto(MyInfo->udp_sock, MyInfo->sendbuf, size, 0, (SOCKADDR*)&multicastaddr, sizeof(SOCKADDR));
			if (retval == SOCKET_ERROR) 
			{
				err_display("Chat Message Send To()");
			}
			break;
		case QUIT_STATE:
			MyInfo->quit_state = FALSE;
			MyInfo->chat_state = FALSE;

			size = UDP_PackPacket(MyInfo->sendbuf, CHATQUIT);
			retval = sendto(MyInfo->udp_sock, MyInfo->sendbuf, size, 0, (SOCKADDR*)&multicastaddr, sizeof(SOCKADDR));
			if (retval == SOCKET_ERROR)
			{
				err_display("Chat Quit Send To()");
			}

			size = PackPacket(MyInfo->sendbuf, CHATQUIT);
			if (send(MyInfo->server_sock, MyInfo->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("Quit Send()");
				break;
			}

			if (setsockopt(MyInfo->udp_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
				(char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) err_quit("IP_DROP_MEMBERSHIP setsockopt()");

			closesocket(MyInfo->udp_sock);

			MyInfo->udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
			if (MyInfo->udp_sock == INVALID_SOCKET) err_quit("socket()");

			optval = true;
			retval = setsockopt(MyInfo->udp_sock, SOL_SOCKET, SO_REUSEADDR,
				(char*)&optval, sizeof(optval));
			if (retval == SOCKET_ERROR) err_quit("SO_REUSEADDR setsockopt()");

			MyInfo->state = SELECT_STATE;
			
			break;
		case CONNECT_END_STATE:
			size = PackPacket(MyInfo->sendbuf, DISCONNECT);
			if (send(MyInfo->server_sock, MyInfo->sendbuf, size, 0) == SOCKET_ERROR)
			{
				err_display("connect end Send()");
			}
			endflag = true;
			break;

		}
		
		EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
		SetEvent(hReadEvent); // 읽기 완료 알리기

		if (endflag)
		{
			break;
		}
	}

	return 0;
}

bool PacketRecv(SOCKET _sock, char* _buf)
{
	int size;

	int retval = recvn(_sock, (char*)&size, sizeof(size), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("gvalue recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(_sock, _buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("gvalue recv error()");
		return false;

	}
	else if (retval == 0)
	{
		return false;
	}

	return true;
}

DWORD CALLBACK RecvThread(LPVOID _ptr)
{
	
	PROTOCOL protocol;
	int size, retval;	
	char nickname[NICKNAMESIZE];
	char msg[BUFSIZE];
	int count, addrlen;	

	while (1)
	{
		if (!MyInfo->chat_state)
		{
			if (!PacketRecv(MyInfo->server_sock, MyInfo->recvbuf))
			{
				err_display("recv error()");
				return -1;
			}
		}
		else
		{
			ZeroMemory(&MyInfo->recvbuf, sizeof(MyInfo->recvbuf));
			addrlen = sizeof(multicastaddr);
			retval = recvfrom(MyInfo->udp_sock, MyInfo->recvbuf, sizeof(MyInfo->recvbuf), 0, (SOCKADDR*)&peeraddr, &addrlen);
			if (retval == SOCKET_ERROR) 
			{
				err_display("recvfrom()");
				break;
			}
		}

		protocol = GetProtocol(MyInfo->recvbuf);
		bool flag = false;
		int data;
		switch (protocol)
		{
		case INTRO:
			memset(msg, 0, sizeof(msg));
			UnPackPacket(MyInfo->recvbuf, msg);
			DisplayText("%s\r\n", msg);
			MyInfo->state = NICK_STATE;
			break;
		case SELECT:
			SetWindowText(hEdit2, "");
			memset(msg, 0, sizeof(msg));
			UnPackPacket(MyInfo->recvbuf, msg);
			DisplayText("%s\r\n", msg);
			MyInfo->state = SELECT_STATE;
			break;
		case ERR:
			SetWindowText(hEdit2, "");
			memset(msg, 0, sizeof(msg));
			UnPackPacket(MyInfo->recvbuf, msg);		
			DisplayText("%s\r\n", msg);
			break;		
		case ADDR:
			ZeroMemory(&multicastaddr, sizeof(SOCKADDR_IN));
			ZeroMemory(&localaddr, sizeof(SOCKADDR_IN));
			UnPackPacket(MyInfo->recvbuf, &multicastaddr);

			localaddr.sin_family = AF_INET;
			localaddr.sin_port = multicastaddr.sin_port;
			localaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

			if (bind(MyInfo->udp_sock, (SOCKADDR*)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
			{
				err_quit("bind()");
				break;
			}
			
			memcpy(&mreq.imr_multiaddr.s_addr, &multicastaddr.sin_addr, sizeof(multicastaddr.sin_addr));

			mreq.imr_interface.s_addr = inet_addr("127.0.0.1");

			retval = setsockopt(MyInfo->udp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
			if (retval == SOCKET_ERROR) err_quit("IP_ADD_MEMBERSHIP setsockopt()");

			ZeroMemory(&buf, sizeof(buf));
			retval = sendto(MyInfo->udp_sock, buf, strlen(buf) + 1, 0, (SOCKADDR*)&multicastaddr, sizeof(SOCKADDR));
			if (retval == SOCKET_ERROR)
			{
				err_display("Test Message Send To()");
			}

			EnableWindow(hQuitButton, TRUE);
			MyInfo->state = CHAT_STATE;
			MyInfo->chat_state = TRUE;
			SetWindowText(hEdit2, "");
			break;
		case CHAT:
			memset(msg, 0, sizeof(msg));
			UnPackPacket(MyInfo->recvbuf, msg);
			DisplayText("%s\r\n", msg);
			break;
		case CHATQUIT:
			MyInfo->chat_state = FALSE;
			break;
		case DISCONNECT:
			flag = TRUE;
			break;
		}

		if (flag)
			break;

	}

	return 0;
}

PROTOCOL GetProtocol(const char* _ptr)
{
	PROTOCOL protocol;
	memcpy(&protocol, _ptr, sizeof(PROTOCOL));

	return protocol;
}
int UDP_PackPacket(char* _buf, PROTOCOL _protocol)
{
	char* ptr = _buf;
	int size = 0;

	memcpy(ptr, &_protocol, sizeof(PROTOCOL));
	ptr = ptr + sizeof(PROTOCOL);
	size = size + sizeof(PROTOCOL);

	return size;
}

int UDP_PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1)
{
	char* ptr = _buf;
	int size = 0;

	memcpy(ptr, &_protocol, sizeof(PROTOCOL));
	ptr = ptr + sizeof(PROTOCOL);
	size = size + sizeof(PROTOCOL);

	int strsize = strlen(_str1);
	memcpy(ptr, &strsize, sizeof(int));
	ptr = ptr + sizeof(int);
	size = size + sizeof(int);

	memcpy(ptr, _str1, strsize);
	ptr = ptr + strsize;
	size = size + strsize;

	return size;
}

int PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(_protocol);

	int strsize1 = strlen(_str1);
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

int PackPacket(char* _buf, PROTOCOL _protocol, int _data)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(size);

	memcpy(ptr, &_data, sizeof(int));
	ptr = ptr + sizeof(int);
	size = size + sizeof(size);

	ptr = _buf;

	memcpy(ptr, &size, sizeof(size));
	size = size + sizeof(size);
	return size;
}

void UnPackPacket(const char* _buf, SOCKADDR_IN* _addr)
{
	int strsize;
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_addr, ptr, sizeof(SOCKADDR));
	ptr = ptr + sizeof(SOCKADDR);
}

void UnPackPacket(const char* _buf, char* _str)
{
	int strsize;
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(_str, ptr, strsize);
	ptr = ptr + strsize;
}

void UnPackPacket(const char* _buf, int& _data, char* _str)
{
	int strsize;
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_data, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(&strsize, ptr, sizeof(strsize));
	ptr = ptr + sizeof(strsize);

	memcpy(_str, ptr, strsize);
	ptr = ptr + strsize;
}

void UnPackPacket(const char* _buf, char* _str, int& _count)
{	
	const char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&_count, ptr, sizeof(_count));
	ptr = ptr + sizeof(_count);

	for (int i = 0; i < _count; i++)
	{
		int strsize;
		memcpy(&strsize, ptr, sizeof(strsize));
		ptr = ptr + sizeof(strsize);

		memcpy(_str, ptr, strsize);
		ptr = ptr + strsize;
		_str = _str + strsize;
		strcat(_str, ",");
		_str++;
	}

}

void MakeChatMessage(const char* _nick, const char* _msg, char* _chattmsg)
{
	sprintf(_chattmsg, "[ %s ] %s", _nick, _msg);
}
