
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


// ��ȭ���� �޽��� ó�� �Լ�
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(char* fmt, ...);
// ���� ��� �Լ�
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ClientDisplay(LPVOID arg);

// ���� ��� �Լ�
void err_quit(char* msg);
void err_display(char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
// ����� ���� ������ ���� �Լ�

char buf[BUFSIZE]; // ������ �ۼ��� ����
char temp_buf[BUFSIZE];

HWND hEdit1, hEdit2, hOKButton, hExitButton, hEndButton; // ����Ʈ ��Ʈ�Ѱ� OK ��ư
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
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
	// �̺�Ʈ ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return -1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return -1;

	memset(&Mystate, 0, sizeof(Mystate));
	// ���� ����� ����ϴ� ������ ����
	DWORD ThreadId;
	hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, &ThreadId);


	// ��ȭ ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	WaitForSingleObject(hClientThread, INFINITE);
	// �̺�Ʈ ����
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);


	// ���� ����
	WSACleanup();

	return 0;
}

// ��ȭ���� �޽��� ó��
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
		SetWindowText(hEdit2, "��ȭ���� �Է��ϼ���\r\n");
		EnableWindow(hExitButton, FALSE);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// OK ��ư ��Ȱ��ȭ
			EnableWindow(hOKButton, FALSE);
			// �б� �ϷḦ ��ٸ�
			WaitForSingleObject(hReadEvent, INFINITE);

			memset(buf, 0, sizeof(buf));
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			
			GetWindowText(hOKButton, temp_buf, sizeof(temp_buf));
			if (strcmp(temp_buf, "������") == 0 && strlen(buf) == 0)
			{
				EnableWindow(hOKButton, TRUE);
				SetEvent(hReadEvent);
				return TRUE;
			}
			// �߰��߰� �������� �����ִ� �Ϸ�޽��� ���� ó���ϱ����� Ȯ���̶�� ���¸� �ϳ� �߰��� �������.
			// �� Ȯ�ι�ư�� Ȱ��ȭ �������� hEdit1�� ��Ȱ��ȭ �ǰ� ó���ߴ�. ���� Ȯ�� ��ư�� ������ ����
			// �ٽ� Ȱ��ȭ �ǰԲ� ó�����־���.
			if (strcmp(temp_buf, "Ȯ��") == 0 && strlen(buf) == 0)
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
			SetEvent(hWriteEvent);// ���� �ϷḦ �˸�
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

// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;


	// ���� ��ſ� TCP socket()
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
				if (strcmp(temp_buf, "�ٸ�������") == 0)
				{
					SetWindowText(hOKButton, "������");
				}

				// Ŭ���̾�Ʈ�� ������ ����Ǹ� ���� �ڽ��� ��ȭ���� ���´�.
				// �ڽ��� ��ȭ���� ���ٸ� �г����� �����Ϸ� ����
				// �ڽ��� ���� ���ͼ� �ٸ� �濡 ���� �Ѵٸ� �渮��Ʈ�� ���� ������ش�.
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

				// buf���� ������ �г����� �����ʿ� ������.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, NICKNAME, Mystate.nickname, size);
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("nickname input send error");
				}

				// ���� �����ʿ��� �޾Ҵٰ� ���������ָ� ���� ������ �޽����� ����ϰ�
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
						// Ȯ�ι�ư���� ����, ����Ʈ��Ʈ�� ��Ȱ��ȭ���� �Ѵ�.
						DisplayText("%s", msg);
						SendMessageW(hEdit1,EM_SETREADONLY, true, 0);
						SetWindowText(hOKButton, "Ȯ��");
						EnableWindow(hOKButton, true);
						SetEvent(hReadEvent);
						break;
					}
					break;
				}
				
				Mystate.state = SELECT_ROOM_MSG_STATE;
				break;
			case SELECT_ROOM_MSG_STATE:
				// �� ����Ʈ�� �޶�� ��û�Ѵ�.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, SELECTROOMMSG, size);
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("request room info send error");
				}

				// �������� �� ����Ʈ�� �����ش�.
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
						// ���⼭ �渮��Ʈ�� �Բ� Ȯ�ι�ư�̾��� ��ư�� ������� �����Ѵ�.
						SetWindowText(hEdit2, "");
						DisplayText("%s", msg);
						SetWindowText(hOKButton, "������");
						EnableWindow(hOKButton, true);
						SetEvent(hReadEvent);
						break;
					}
					break;
				}

				Mystate.state = SELECT_ROOM_STATE;
				break;
			case SELECT_ROOM_STATE:
			
				// ���� ��� �������� �����ش�.
				int room_number = atoi(buf);

				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHOICEROOMNUM, room_number, size);

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("chatinfo SELECT send() error");
				}
				// Ŭ���̾�Ʈ���� ������ �����ϴ� �����ʿ��� ������� �����Ŀ� ������� �ٽ� �����ش�.
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
						// ��� �� ��ư ����, ��Ȱ��ȭ���� �۾��� ���ش�.
						DisplayText("%s", msg);
						SendMessageW(hEdit1, EM_SETREADONLY, true, 0);
						SetWindowText(hOKButton, "Ȯ��");
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
				// �濡 ���� �Ѵٰ� ��û�Ѵ�. �׷��� �����ʿ��� �� ����(�г���, ����� �������)�� �˰�������
				// �ش� �濡 ���� �޽����� �ѷ��ش�. �� �����ؼ�
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHATENTERREQUSET, size);
				
				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					closesocket(Mystate.sock);
					err_quit("chat enter msg send() error");
				}

				// ���� ä�� ���� �ޱ� �����带 ������ش�.
				DWORD ThreadId;
				hClientDisplay = CreateThread(NULL, 0, ClientDisplay, NULL, 0, &ThreadId);
				Mystate.room_flag = true;
				Mystate.state = CHAT_ING_STATE;
				
				SetWindowText(hOKButton, "������");
				EnableWindow(hOKButton, true);
				SetEvent(hReadEvent);
				break;
			case CHAT_ING_STATE:
				// ä���� �Է��ؼ� �����ش�.
				ZeroMemory(Mystate.sendbuf, sizeof(Mystate.sendbuf));
				PackPacket(Mystate.sendbuf, CHATING, buf, size);

				retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("chat ing msg send()");
				}
				// OK ��ư Ȱ��ȭ
				EnableWindow(hOKButton, TRUE);
				// �б� �ϷḦ �˸�
				SetEvent(hReadEvent);
			
				break;
			}
			break;
		case ROOM_EXIT:
		{
			// ä�ù濡�� �������ư�� Ŭ���ϸ� �ڽ��� ���μ��� ���̵�� �Բ� ���������� ��Ű¡�ؼ� ������ ������.
			PackPacket(Mystate.sendbuf, CHATQUIT, GetCurrentProcessId(), size);

			retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()");
			}
			// �����ʿ� 2�� ������ ������ �����ʿ��� chat_ing_state���� ����� ���� 1��, chat_quit_state�� ���� ���� 2���̴�.
			retval = send(Mystate.sock, Mystate.sendbuf, size, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("sendto()");
			}

			// chat_quit_state���� ���������Ḧ ���� �������ݰ� �Բ� id���� �����ֱ� ������ ���⼭ �����尡 ����ɶ����� ��ٸ���.
			WaitForSingleObject(hClientDisplay, INFINITE);
			hClientDisplay = nullptr;

			// ���� �濡 ���ٰ� �����ϰ� init_state�� �� �̹����� �г����� ������� �ʾұ� ������
			// �ٷ� �渮��Ʈ ��� ���·� ���ư���.
			Mystate.room_flag = false;
			Mystate.state = INIT_STATE;

			SetWindowText(hOKButton, "�ٸ�������");
			EnableWindow(hExitButton, FALSE);
			SetEvent(hReadEvent);
		}
		break;
		case EXIT:
		{
			// ������ ��ư�� �����ٸ� ���� ��ȿ� ������� 
			if (Mystate.room_flag)
			{
				// ���� ������ ��ư�� ���������� �����ϰ� ó�������� init_state�� ���ư��� �ʾƵ� �Ǳ⶧����
				// �׺κ��� ����.
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

				// send�� 2���Ѱ͵� ���������� ������ �������Ḧ ���� �����ʿ��� 1�� ������� �ϱ� �����̴�.
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
		// ä�ù� ����� Ŭ���̾�Ʈ���� ä���� �ޱ� ó���ϱ� ���� �ޱ� ���� �������Լ� �̴�.
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

	DisplayText("������ ����\r\n");
	return 0;
}


// ����Ʈ ��Ʈ�ѿ� ���ڿ� ���
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

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

