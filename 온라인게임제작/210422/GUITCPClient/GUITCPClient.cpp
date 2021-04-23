
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

#define BUFSIZE 4096
#define NICKNAMESIZE 255

enum STATE
{
	INITE_STATE = 1,
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
	NO_CODE = -1,
	CHAT_ROOM_SELECT_ERROR = -2
};

enum SELECT
{
	SEND = 1,
	ROOM_EXIT,
	EXIT
};

struct _Mystate
{
	SOCKET tcp_sock;
	STATE state;
	bool room_flag = false;
	char nickname[BUFSIZE];
	char sendbuf[BUFSIZE];
	char recvbuf[BUFSIZE];
}Mystate;


// 대화상자 메시지 처리 함수
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수
void DisplayText(char* fmt, ...);
// 소켓 통신 함수
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ClientDisplay(LPVOID arg);

// 오류 출력 함수
void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
// 사용자 정의 데이터 수신 함수


char buf[BUFSIZE]; // 데이터 송수신 버퍼
char temp_buf[BUFSIZE];

HWND hEdit1, hEdit2, hOKButton, hExitButton, hEndButton; // 에디트 컨트롤과 OK 버튼
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND g_hDlg;
HANDLE hClientThread;

SOCKADDR_IN localaddr;
SELECT button;

bool PacketRecv(SOCKET, char*);
void GetProtocol(char* _ptr, PROTOCOL& _protocol);

void PackPacket(char*, PROTOCOL, int, const char*, int&);
void PackPacket(char*, PROTOCOL, const char*, int&);
void PackPacket(char*, PROTOCOL, int, int&);
void PackPacket(char*, PROTOCOL, int&);

void UnPackPacket(char*, char*);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return -1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return -1;

	memset(&Mystate, 0, sizeof(Mystate));
	// 소켓 통신을 담당하는 스레드 생성
	DWORD ThreadId;
	hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, &ThreadId);


	// 대화 상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	WaitForSingleObject(hClientThread, INFINITE);
	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);


	// 윈속 종료
	WSACleanup();

	return 0;
}

// 대화상자 메시지 처리
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		Mystate.state = INITE_STATE;
		g_hDlg = hDlg;
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hOKButton = GetDlgItem(hDlg, IDOK);
		hExitButton = GetDlgItem(hDlg, IDEXIT);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		SetWindowText(hEdit2, "대화명을 입력하세요");
		EnableWindow(hExitButton, FALSE);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// OK 버튼 비활성화
			EnableWindow(hOKButton, FALSE);
			// 읽기 완료를 기다림
			WaitForSingleObject(hReadEvent, INFINITE);

			memset(buf, 0, sizeof(buf));
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			GetWindowText(hOKButton, temp_buf, sizeof(temp_buf));

			if (strcmp(temp_buf, "보내기") == 0 && strlen(buf) == 0)
			{
				EnableWindow(hOKButton, TRUE);
				SetEvent(hReadEvent);
				return TRUE;
			}
			SetWindowText(hEdit1, "");
			button = SEND;
			SetEvent(hWriteEvent);// 쓰기 완료를 알림
			SetFocus(hEdit1);
			return TRUE;
		case IDEXIT:
			button = ROOM_EXIT;
			EnableWindow(hExitButton, FALSE);
			SetDlgItemText(hDlg, IDC_EDIT2, TEXT(""));
			SetEvent(hWriteEvent);
			return TRUE;
		case IDCANCEL:
			button = EXIT;
			SetEvent(hWriteEvent);
			EndDialog(hDlg, 0);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// TCP 클라이언트 시작 부분
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;


	// 서버 통신용 TCP socket()
	Mystate.tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Mystate.tcp_sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9900);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	retval = connect(Mystate.tcp_sock, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR));
	if (retval == SOCKET_ERROR) err_quit("connect() error");

	SOCKADDR_IN multicastaddr;
	HANDLE hClientDisplay = nullptr;

	int size;
	bool endflg = false;

	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE);
		memset(temp_buf, 0, sizeof(temp_buf));

		switch (button)
		{
		case SEND:

			switch (Mystate.state)
			{
			case INITE_STATE:
				if (Mystate.nickname[0] == '\0')
				{
					strcpy(Mystate.nickname, buf);
				}

				GetWindowText(hOKButton, temp_buf, sizeof(temp_buf));
				if (strcmp(temp_buf, "다른방입장") == 0)
				{
					SetWindowText(hOKButton, "보내기");
				}

				Mystate.state = CHAT_ROOM_LIST_SEND_STATE;

			case CHAT_ROOM_LIST_SEND_STATE:
			{
				PackPacket(Mystate.sendbuf, REQ_CHAT_ROOM_LIST, Mystate.nickname, size);

				retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.tcp_sock);
					err_quit("REQ_CHAT_ROOM_LIST send error");
				}

				if (!PacketRecv(Mystate.tcp_sock, Mystate.recvbuf))
				{
					closesocket(Mystate.tcp_sock);
					err_quit("chatroom list recvn() error");
				}

				char msg[BUFSIZE];
				memset(msg, 0, sizeof(msg));

				UnPackPacket(Mystate.recvbuf, msg);

				SetWindowText(hEdit2, "");
				DisplayText("%s\r\n", msg);

				EnableWindow(hOKButton, true);
				SetEvent(hReadEvent);
				Mystate.state = CHAT_ROOM_SELECT_SATE;
			}
			break;
			case CHAT_ROOM_SELECT_SATE:
			{
				int room_number = atoi(buf);

				PackPacket(Mystate.sendbuf, CHAT_ROOM_ENTER, room_number, Mystate.nickname, size);

				retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.tcp_sock);
					err_quit("chatinfo SELECT send() error");
				}

				if (!PacketRecv(Mystate.tcp_sock, Mystate.recvbuf))
				{
					closesocket(Mystate.tcp_sock);
					err_quit("chatroom list recvn() error");
				}

				SetWindowText(hEdit2, "");
				EnableWindow(hExitButton, TRUE);
				EnableWindow(hOKButton, TRUE);
				SetEvent(hReadEvent);

				Mystate.room_flag = true;
				//쓰기 쓰레드 생성
				DWORD ThreadId;
				hClientDisplay = CreateThread(NULL, 0, ClientDisplay, NULL, 0, &ThreadId);
				Mystate.state = CHATTING_SEND_STATE;
			}
			break;
			case CHATTING_SEND_STATE:
			{
				ZeroMemory(temp_buf, sizeof(temp_buf));
				strcpy(temp_buf, Mystate.nickname);
				strcat(temp_buf, ": ");
				strcat(temp_buf, buf);
				PackPacket(Mystate.sendbuf, CHAT_MSG, temp_buf, size);

				retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("sendto()");
				}
				// OK 버튼 활성화
				EnableWindow(hOKButton, TRUE);
				// 읽기 완료를 알림
				SetEvent(hReadEvent);
			}
			break;
			}
			break;
		case ROOM_EXIT:
		{
			PackPacket(Mystate.sendbuf, CHAT_ROOM_EXIT, size);

			retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()");
			}

			WaitForSingleObject(hClientDisplay, INFINITE);
			hClientDisplay = nullptr;

			PackPacket(Mystate.sendbuf, CHAT_ROOM_EXIT, size);
			retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				closesocket(Mystate.tcp_sock);
				err_quit("chat exit send() error");
			}

			Mystate.room_flag = false;
			Mystate.state = INITE_STATE;
			SetWindowText(hOKButton, "다른방입장");
			EnableWindow(hExitButton, FALSE);
			SetEvent(hReadEvent);
		}
		break;
		case EXIT:
		{
			if (Mystate.room_flag)
			{
				PackPacket(Mystate.sendbuf, CHAT_ROOM_EXIT, size);

				retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("sendto()");
				}

				WaitForSingleObject(hClientDisplay, INFINITE);
				hClientDisplay = nullptr;

				PackPacket(Mystate.sendbuf, CHAT_ROOM_EXIT, size);
				retval = send(Mystate.tcp_sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.tcp_sock);
					err_quit("chat exit send() error");
				}

			}

			if (Mystate.tcp_sock != NULL)
			{
				closesocket(Mystate.tcp_sock);
			}

			endflg = true;
		}
		break;
		}


		if (endflg)
		{
			break;
		}
	}

	return 0;
}

DWORD WINAPI ClientDisplay(LPVOID arg)
{
	SOCKADDR_IN peeraddr;
	int addrlen;
	int retval;
	char msgbuf[BUFSIZE + 1];
	int pro_id;
	addrlen = sizeof(peeraddr);
	bool endflag = false;

	while (1)
	{
		if (!PacketRecv(Mystate.tcp_sock, Mystate.recvbuf))
		{
			closesocket(Mystate.tcp_sock);
			err_quit("chatroom list recvn() error");
		}

		PROTOCOL protocol;
		GetProtocol(Mystate.recvbuf, protocol);
		memset(msgbuf, 0, sizeof(msgbuf));

		switch (protocol)
		{
		case CHAT_MSG:
			UnPackPacket(Mystate.recvbuf, msgbuf);
			break;
		case CHAT_ROOM_EXIT_COMPLETE:
			endflag = true;
			break;
		}

		if (endflag)
		{
			break;
		}

		DisplayText("%s\r\n", msgbuf);
	}

	DisplayText("쓰레드 종료\r\n");
	return 0;
}

// 에디트 컨트롤에 문자열 출력
void DisplayText(char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

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
	DisplayText("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
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
		err_display("packet size recv error()");
		return false;
	}
	else if (retval == 0)
	{
		return false;
	}

	retval = recvn(_sock, _buf, size, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("packet data recv error()");
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

void PackPacket(char* _buf, PROTOCOL _protocol, int _data, const char* _str1, int& _size)
{
	char* ptr = _buf;
	int strsize1 = strlen(_str1);
	_size = 0;
	ptr = ptr + sizeof(_size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	memcpy(ptr, &_data, sizeof(_data));
	ptr = ptr + sizeof(_data);
	_size = _size + sizeof(_data);

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

void PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1, int& _size)
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

void PackPacket(char* _buf, PROTOCOL _protocol, int _data, int& _size)
{
	char* ptr = _buf;

	ptr = ptr + sizeof(_size);
	_size = 0;

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);


	memcpy(ptr, &_data, sizeof(_data));
	ptr = ptr + sizeof(_data);
	_size = _size + sizeof(_data);

	ptr = _buf;

	memcpy(ptr, &_size, sizeof(_size));
	_size = _size + sizeof(_size);

}

void PackPacket(char* _buf, PROTOCOL _protocol, int& _size)
{
	char* ptr = _buf;

	ptr = ptr + sizeof(_size);
	_size = 0;

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	_size = _size + sizeof(_protocol);

	ptr = _buf;

	memcpy(ptr, &_size, sizeof(_size));
	_size = _size + sizeof(_size);

}

void UnPackPacket(char* _buf, char* _str1)
{
	int str1size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(&str1size, ptr, sizeof(str1size));
	ptr = ptr + sizeof(str1size);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}

