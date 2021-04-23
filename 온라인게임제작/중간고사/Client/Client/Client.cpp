
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

#define BUFSIZE 4096
#define NICKNAMESIZE 255

enum PROTOCOL
{
	NONE = -1,

	// client send protocol
	NICKNAME = 1,
	SELECTROOMMSG,
	CHOICEROOMNUM,
	CHATENTERREQUSET,
	CHATING,
	CHATQUIT,
	DISCONNECT,

	// server send protocol
	NICK_RESULT,
	ROOM_RESULT,
	CHAT_RESULT,
};

enum STATE
{
	INIT_STATE = -1,
	NICKNAME_INPUT_STATE = 1,
	NICKNAME_RESULT_STATE,

	SELECT_ROOM_MSG_STATE,
	SELECT_ROOM_MSG_END_STATE,

	SELECT_ROOM_STATE,
	SELECT_ROOM_END_STATE,

	CHAT_ENTER_STATE,
	CHAT_ING_STATE,
	CHAT_END_STATE,

	DISCONNECTED_STATE,
};

enum RESULT
{
	NODATA = -1,

	NICKNAME_SUCCESS = 1,

	SELECT_ROOM_MSGSEND,
	SELECT_ROOM_SUCCESS,

	CHAT_ENTER_SUCCESS,
	CHAT_MSG_SEND,
	CHAT_QUIT_SUCCESS,

};

enum SELECT
{
	SEND = 1,
	ROOM_EXIT,
	EXIT
};

struct _Mystate
{
	SOCKET sock;
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

void PackPacket(char* _buf, PROTOCOL _protocol, const char* _str1, int& _size);
void PackPacket(char* _buf, PROTOCOL _protocol, int _data, int& _size);
void PackPacket(char* _buf, PROTOCOL _protocol, int& _size);

void UnPackPacket(char* _buf, RESULT* _result);
void UnPackPacket(char* _buf, RESULT* _result, char* _str1);
void UnPackPacket(char* _buf, RESULT* _result, int* _proid, char* _str1);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE,
	LPSTR lpCmdLine, int nCmdShow)
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
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg,	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		Mystate.state = INIT_STATE;
		g_hDlg = hDlg;
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hOKButton = GetDlgItem(hDlg, IDOK);
		hExitButton = GetDlgItem(hDlg, IDEXIT);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		SetWindowText(hEdit2, "대화명을 입력하세요\r\n");
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
			// 중간중간 서버에서 보내주는 완료메시지 등을 처리하기위해 확인이라는 상태를 하나 추가로 만들었다.
			// 이 확인버튼이 활성화 됬을때는 hEdit1이 비활성화 되게 처리했다. 따라서 확인 버튼을 누르고 나면
			// 다시 활성화 되게끔 처리해주었다.
			if (strcmp(temp_buf, "확인") == 0 && strlen(buf) == 0)
			{
				SetWindowText(hEdit2, "");
				SetEvent(hWriteEvent);
				SendMessageW(hEdit1, EM_SETREADONLY, false, 0);
				EnableWindow(hOKButton, TRUE);
				SetFocus(hEdit1);
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
	Mystate.sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Mystate.sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	retval = connect(Mystate.sock, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR));
	if (retval == SOCKET_ERROR) err_quit("connect() error");
	/*button = SEND;*/

	//SOCKADDR_IN multicastaddr;
	HANDLE hClientDisplay = nullptr;
	RESULT result = NODATA;
	PROTOCOL protocol = NONE;

	int size;
	bool endflg = false;
	char msg[BUFSIZE];
	while (1)
	{
		WaitForSingleObject(hWriteEvent, INFINITE);
		memset(temp_buf, 0, sizeof(temp_buf));

		switch (button)
		{
		case SEND:

			switch (Mystate.state)
			{
			case INIT_STATE:
				GetWindowText(hOKButton, temp_buf, sizeof(temp_buf));
				if (strcmp(temp_buf, "다른방입장") == 0)
				{
					SetWindowText(hOKButton, "보내기");
				}

				// 클라이언트가 켜지고 연결되면 먼저 자신의 대화명을 적는다.
				// 자신의 대화명이 없다면 닉네임을 설정하러 가고
				// 자신이 방을 나와서 다른 방에 들어가려 한다면 방리스트를 새로 출력해준다.
				if (Mystate.nickname[0] == '\0')
					Mystate.state = NICKNAME_INPUT_STATE;
				else
					Mystate.state = SELECT_ROOM_MSG_STATE;

				SetEvent(hWriteEvent);
				break;
			case NICKNAME_INPUT_STATE:
				if (Mystate.nickname[0] == '\0')
				{
					strcpy(Mystate.nickname, buf);
				}

				// buf에서 가져온 닉네임을 서버쪽에 보낸다.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, NICKNAME, Mystate.nickname, size);
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("nickname input send error");
				}

				// 그후 서버쪽에서 받았다고 재전송해주면 같이 보내준 메시지를 출력하고
				if (!PacketRecv(Mystate.sock, Mystate.recvbuf))
				{
					closesocket(Mystate.sock);
					err_quit("nickname result recv error");
				}

				GetProtocol(Mystate.recvbuf, protocol);
				switch (protocol)
				{
				case NICK_RESULT:
					memset(msg, 0, sizeof(msg));
					UnPackPacket(Mystate.recvbuf, &result, msg);
					switch (result)
					{
					case NICKNAME_SUCCESS:
						// 확인버튼으로 변경, 에디트컨트롤 비활성화등을 한다.
						DisplayText("%s", msg);
						SendMessageW(hEdit1,EM_SETREADONLY, true, 0);
						SetWindowText(hOKButton, "확인");
						EnableWindow(hOKButton, true);
						SetEvent(hReadEvent);
						break;
					}
					break;
				}
				
				Mystate.state = SELECT_ROOM_MSG_STATE;
				break;
			case SELECT_ROOM_MSG_STATE:
				// 방 리스트를 달라고 요청한다.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, SELECTROOMMSG, size);
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("request room info send error");
				}

				// 서버에서 방 리스트를 보내준다.
				if (!PacketRecv(Mystate.sock, Mystate.recvbuf))
				{
					closesocket(Mystate.sock);
					err_quit("request room info result recvn() error");
				}

				GetProtocol(Mystate.recvbuf, protocol);
				switch (protocol)
				{
				case ROOM_RESULT:
					memset(msg, 0, sizeof(msg));
					UnPackPacket(Mystate.recvbuf, &result, msg);
					switch (result)
					{
					case SELECT_ROOM_MSGSEND:
						// 여기서 방리스트와 함께 확인버튼이었던 버튼을 보내기로 변경한다.
						SetWindowText(hEdit2, "");
						DisplayText("%s", msg);
						SetWindowText(hOKButton, "보내기");
						EnableWindow(hOKButton, true);
						SetEvent(hReadEvent);
						break;
					}
					break;
				}

				Mystate.state = SELECT_ROOM_STATE;
				break;
			case SELECT_ROOM_STATE:
			
				// 방을 골라서 서버한테 보내준다.
				int room_number = atoi(buf);

				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHOICEROOMNUM, room_number, size);

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("chatinfo SELECT send() error");
				}
				// 클라이언트들의 정보를 관리하는 서버쪽에서 방셋팅을 해준후에 결과값을 다시 보내준다.
				if (!PacketRecv(Mystate.sock, Mystate.recvbuf))
				{
					closesocket(Mystate.sock);
					err_quit("chatroom list recvn() error");
				}

				GetProtocol(Mystate.recvbuf, protocol);
				switch (protocol)
				{
				case ROOM_RESULT:
					memset(msg, 0, sizeof(msg));
					UnPackPacket(Mystate.recvbuf, &result, msg);
					switch (result)
					{
					case SELECT_ROOM_SUCCESS:
						// 출력 및 버튼 변경, 비활성화등의 작업을 해준다.
						DisplayText("%s", msg);
						SendMessageW(hEdit1, EM_SETREADONLY, true, 0);
						SetWindowText(hOKButton, "확인");
						EnableWindow(hExitButton, TRUE);
						EnableWindow(hOKButton, true);
						SetEvent(hReadEvent);
						break;
					}
					break;
				}
			
				Mystate.state = CHAT_ENTER_STATE;
				break;

			case CHAT_ENTER_STATE:
				// 방에 입장 한다고 요청한다. 그러면 서버쪽에서 내 정보(닉네임, 몇번방 골랐는지)를 알고있으니
				// 해당 방에 입장 메시지를 뿌려준다. 나 포함해서
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHATENTERREQUSET, size);
				
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("chat enter msg send() error");
				}

				// 그후 채팅 전용 받기 쓰레드를 만들어준다.
				DWORD ThreadId;
				hClientDisplay = CreateThread(NULL, 0, ClientDisplay, NULL, 0, &ThreadId);
				Mystate.room_flag = true;
				Mystate.state = CHAT_ING_STATE;
				
				SetWindowText(hOKButton, "보내기");
				EnableWindow(hOKButton, true);
				SetEvent(hReadEvent);
				break;
			case CHAT_ING_STATE:
				// 채팅을 입력해서 보내준다.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHATING, buf, size);

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("chat ing msg send()");
				}
				// OK 버튼 활성화
				EnableWindow(hOKButton, TRUE);
				// 읽기 완료를 알림
				SetEvent(hReadEvent);
			
				break;
			}
			break;
		case ROOM_EXIT:
		{
			// 채팅방에서 나가기버튼을 클릭하면 자신의 프로세스 아이디와 함께 프로토콜을 패키징해서 서버에 보낸다.
			PackPacket(Mystate.sendbuf, CHATQUIT, GetCurrentProcessId(), size);

			retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()");
			}
			// 서버쪽에 2번 보내는 이유는 서버쪽에서 chat_ing_state에서 벗어나기 위한 1번, chat_quit_state로 들어가기 위해 2번이다.
			retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()");
			}

			// chat_quit_state에서 쓰레드종료를 위한 프로토콜과 함께 id값을 보내주기 때문에 여기서 쓰레드가 종료될때까지 기다린다.
			WaitForSingleObject(hClientDisplay, INFINITE);
			hClientDisplay = nullptr;

			// 그후 방에 없다고 설정하고 init_state로 들어가 이번에는 닉네임은 사라지지 않았기 때문에
			// 바로 방리스트 출력 상태로 돌아간다.
			Mystate.room_flag = false;
			Mystate.state = INIT_STATE;

			SetWindowText(hOKButton, "다른방입장");
			EnableWindow(hExitButton, FALSE);
			SetEvent(hReadEvent);
		}
		break;
		case EXIT:
		{
			// 끝내기 버튼을 눌렀다면 먼저 방안에 있을경우 
			if (Mystate.room_flag)
			{
				// 위의 나가기 버튼을 눌렀을때와 동일하게 처리하지만 init_state로 돌아가지 않아도 되기때문에
				// 그부분은 없다.
				PackPacket(Mystate.sendbuf, CHATQUIT, GetCurrentProcessId(), size);

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("sendto()");
				}

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("sendto()");
				}

				// send를 2번한것도 마찬가지로 쓰레드 정상종료를 위해 서버쪽에서 1번 보내줘야 하기 때문이다.
				WaitForSingleObject(hClientDisplay, INFINITE);
				hClientDisplay = nullptr;
			}

			if (Mystate.sock != NULL)
			{
				closesocket(Mystate.sock);
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

	RESULT result;

	while (1)
	{
		// 채팅방 입장시 클라이언트간의 채팅을 받기 처리하기 위한 받기 전용 스레드함수 이다.
		if (!PacketRecv(Mystate.sock, Mystate.recvbuf))
		{
			closesocket(Mystate.sock);
			err_quit("chatroom list recvn() error");
		}
		
		PROTOCOL protocol;
		GetProtocol(Mystate.recvbuf, protocol);
		memset(msgbuf, 0, sizeof(msgbuf));

		UnPackPacket(Mystate.recvbuf, &result);
		switch (protocol)
		{
		case CHAT_RESULT:
			switch (result)
			{
			case CHAT_ENTER_SUCCESS:
			case CHAT_MSG_SEND:
				UnPackPacket(Mystate.recvbuf, &result, msgbuf);	
				break;
			case CHAT_QUIT_SUCCESS:
				UnPackPacket(Mystate.recvbuf, &result, &pro_id, msgbuf);
				if(pro_id == GetCurrentProcessId())
					endflag = true;
				break;
			}
			break;
		}

		DisplayText("%s", msgbuf);

		if (endflag)
		{
			break;
		}

		
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

void UnPackPacket(char* _buf, RESULT* _result)
{
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_result, ptr, sizeof(RESULT));
	ptr = ptr + sizeof(RESULT);

}
void UnPackPacket(char* _buf, RESULT* _result, char* _str1)
{
	int str1size;

	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_result, ptr, sizeof(RESULT));
	ptr = ptr + sizeof(RESULT);

	memcpy(&str1size, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}
void UnPackPacket(char* _buf, RESULT* _result, int* _proid, char* _str1)
{
	int str1size;
	char* ptr = _buf + sizeof(PROTOCOL);

	memcpy(_result, ptr, sizeof(RESULT));
	ptr = ptr + sizeof(RESULT);

	memcpy(_proid, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(&str1size, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(_str1, ptr, str1size);
	ptr = ptr + str1size;
}

