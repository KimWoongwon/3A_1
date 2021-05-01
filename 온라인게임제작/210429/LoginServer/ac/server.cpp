#pragma comment(lib,"ws2_32")
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define WM_SOCKET  (WM_USER+1)
#define SERVERPORT 9000
#define BUFSIZE 512

#define IDSIZE 255
#define PWSIZE 255
#define ERROR_DISCONNECTED -2
#define DISCONNECTED -1
#define SOC_TRUE 1
#define SOC_FALSE 0
#define JOIN_COUNT 3

#define INTRO_MSG "아이디와 패스워드를 입력하세요\n"
#define IDERROR_MSG "없는 아이디입니다\n"
#define PWERROR_MSG "패스워드가 틀렸습니다.\n"
#define LOGINSUCCESS_MSG "로그인에 성공했습니다.\n"

enum PROTOCOL
{
	INTRO = 1,
	LOGIN,

};

enum STATE 
{
	INIT_STATE=-1, 
	INTRO_SEND_STATE =1, 
	LOGIN_INFO_RECV_STATE,
	LOGIN_RESULT_SEND_STATE,
	END_STATE,
	DISCONNECTED_STATE
};

enum RESULT 
{
	NODATA = -1, 	
	ID_ERROR=1, 
	PW_ERROR, 
	LOGIN_SUCCESS 
};

struct _LoginInfo
{
	char id[255];
	char pw[255];
}LoginInfo[3] = { {"kja0204","1234"},{"aaa","111"},{"bbb","222"} };


struct SOCKETINFO
{
	SOCKET sock;
	char		buf[BUFSIZE + 1];
	int			recvbytes;
	int			sendbytes;
	BOOL recvdelayed;
	SOCKETINFO* next;

	//User Custom
	_LoginInfo  userinfo;
	STATE		state;
	
};

SOCKETINFO* SocketInfoList;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

SOCKETINFO* AddSocketInfo(SOCKET sock);
SOCKETINFO* GetSocketInfo(SOCKET sock);
void RemoveSocketInfo(SOCKET sock);

void LoginProcess(SOCKETINFO* _ptr);
void RecvPacketProcess(SOCKETINFO* _ptr);
void SendPacketProcess(SOCKETINFO* _ptr);

int recvn(SOCKET s, char* buf, int len, int flags);
int PackPacket(char* buf, PROTOCOL protocol, int result, const char* msg);
void UnPackPacket(const char* buf, char* id, char* pw);

void err_quit(char* msg);
void err_display(char* msg);
void err_display(int errcode);

int main(int argc, char **argv)
{
	int retval;
	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;

	// 윈도우 생성
	HWND hWnd = CreateWindow("MyWndClass", "TCP 서버", WS_OVERLAPPEDWINDOW,
		0, 0, 600, 200, NULL, NULL, NULL, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */
		err_quit("WSAStartup() error!");

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket() error");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR) err_quit("WSAAsyncSelect()");

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_SOCKET: // 소켓 관련 윈도우 메시지
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 데이터 통신에 사용할 변수
	SOCKETINFO* ptr;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen, retval, result, size;

	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam)) {
		err_display(WSAGETSELECTERROR(lParam));
		RemoveSocketInfo(wParam);
		return;
	}

	// 메시지 처리
	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_ACCEPT:
		addrlen = sizeof(clientaddr);
		client_sock = accept(wParam, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			return;
		} 
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		ptr = AddSocketInfo(client_sock);
		retval = WSAAsyncSelect(client_sock, hWnd,
			WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
		if (retval == SOCKET_ERROR) {
			err_display("WSAAsyncSelect()");
			RemoveSocketInfo(client_sock);
		}

		ZeroMemory(ptr->buf, sizeof(ptr->buf));
		ptr->sendbytes = PackPacket(ptr->buf, INTRO, NODATA, INTRO_MSG);
		ptr->state = INTRO_SEND_STATE;
		break;
	case FD_READ:
		ptr = GetSocketInfo(wParam);
		if (ptr->recvbytes > 0) {
			ptr->recvdelayed = TRUE;
			return;
		}

		retval = recvn(ptr->sock, (char*)&size, sizeof(size), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("packetsize recv error()");
			break;
		}
		else if (retval == 0)
		{
			break;
		}
		// 데이터 받기
		retval = recvn(ptr->sock, ptr->buf, size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			RemoveSocketInfo(wParam);
			return;
		}

		RecvPacketProcess(ptr);

		if(ptr->state == LOGIN_RESULT_SEND_STATE)
			PostMessage(hWnd, WM_SOCKET, wParam, FD_WRITE);
		break;
	case FD_WRITE:
		ptr = GetSocketInfo(wParam);
		// 데이터 보내기
		retval = send(ptr->sock, ptr->buf, ptr->sendbytes, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			RemoveSocketInfo(wParam);
			return;
		}
		ptr->sendbytes += retval;
		// 받은 데이터를 모두 보냈는지 체크
		if (ptr->recvbytes == ptr->sendbytes) {
			ptr->recvbytes = ptr->sendbytes = 0;
			if (ptr->recvdelayed) {
				ptr->recvdelayed = FALSE;
				PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
			}
		}

		SendPacketProcess(ptr);
		break;
	case FD_CLOSE:
		RemoveSocketInfo(wParam);
		break;
	}
}

// 소켓 정보 추가
SOCKETINFO* AddSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	ptr->recvdelayed = FALSE;
	ptr->next = SocketInfoList;
	SocketInfoList = ptr;

	return ptr;
}

// 소켓 정보 얻기
SOCKETINFO* GetSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = SocketInfoList;

	while (ptr) {
		if (ptr->sock == sock)
			return ptr;
		ptr = ptr->next;
	}

	return NULL;
}

// 소켓 정보 제거
void RemoveSocketInfo(SOCKET sock)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	SOCKETINFO* curr = SocketInfoList;
	SOCKETINFO* prev = NULL;

	while (curr) {
		if (curr->sock == sock) {
			if (prev)
				prev->next = curr->next;
			else
				SocketInfoList = curr->next;
			closesocket(curr->sock);
			delete curr;
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
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

void RecvPacketProcess(SOCKETINFO* _ptr)
{
	switch (_ptr->state)
	{
	case LOGIN_INFO_RECV_STATE:
		memset(&_ptr->userinfo, 0, sizeof(_LoginInfo));
		UnPackPacket(_ptr->buf, _ptr->userinfo.id, _ptr->userinfo.pw);
		LoginProcess(_ptr);
		break;
	}

}

void SendPacketProcess(SOCKETINFO* _ptr)
{
	switch (_ptr->state)
	{
	case INTRO_SEND_STATE:
		_ptr->state = LOGIN_INFO_RECV_STATE;
		break;
	case LOGIN_RESULT_SEND_STATE:
		if (strlen(_ptr->userinfo.id) == 0)
		{
			_ptr->state = LOGIN_INFO_RECV_STATE;
		}
		else
		{
			_ptr->state = END_STATE;
		}
		break;
	}
}

void LoginProcess(SOCKETINFO* _ptr)
{
	RESULT result = NODATA;
	char msg[255];
	ZeroMemory(msg, sizeof(msg));

	for (int i = 0; i < JOIN_COUNT; i++)
	{
		if (!strcmp(LoginInfo[i].id, _ptr->userinfo.id))
		{
			if (!strcmp(LoginInfo[i].pw, _ptr->userinfo.pw))
			{
				result = LOGIN_SUCCESS;
				strcpy(msg, LOGINSUCCESS_MSG);
			}
			else
			{
				result = PW_ERROR;
				strcpy(msg, PWERROR_MSG);
			}
			break;
		}
	}

	if (result == NODATA)
	{
		result = ID_ERROR;
		strcpy(msg, IDERROR_MSG);
	}

	if (result != LOGIN_SUCCESS)
	{
		memset(&_ptr->userinfo, 0, sizeof(_LoginInfo));
	}
	ZeroMemory(_ptr->buf, sizeof(_ptr->buf));
	_ptr->sendbytes = PackPacket(_ptr->buf, LOGIN, result, msg);
	_ptr->state = LOGIN_RESULT_SEND_STATE;
}

int PackPacket(char* _buf, PROTOCOL _protocol, int result, const char* _str1)
{
	char* ptr = _buf;
	int size = 0;
	ptr = ptr + sizeof(size);

	memcpy(ptr, &_protocol, sizeof(_protocol));
	ptr = ptr + sizeof(_protocol);
	size = size + sizeof(_protocol);

	memcpy(ptr, &result, sizeof(result));
	ptr = ptr + sizeof(result);
	size = size + sizeof(result);

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

void UnPackPacket(const char* buf, char* id, char* pw)
{
	int strsize;
	const char* ptr = buf + sizeof(PROTOCOL);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(id, ptr, strsize);
	ptr = ptr + strsize;

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(pw, ptr, strsize);
	ptr = ptr + strsize;
}