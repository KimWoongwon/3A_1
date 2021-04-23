#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <mstcpip.h>

#define SERVERPORT 9900
#define NICK_SIZE 20
#define MAXUSER 10

#define BUFSIZE 512

// Nagle 알고리즘
// Nagle 알고리즘(이하 네이글 알고리즘)은 '가능하면 조금씩 여러번 보내지 말고 한번에 많은 데이터를 보내라'
// 라는 원칙을 기반으로 만들어진 알고리즘이다. 네이글 알고리즘이 꺼진 상태라면 'hello'라는 데이터를 보내기 위해
// h 보내고 e 보내고 l 보내고 l 보내고 o 를 보내서 패킷이 5개가 생성이 되었다면 받는쪽에서 받았다는 ACK를 5번을 차례로
// 보내는 측에서는 나중에 받게 된다. 네이글 알고리즘을 사용하게 되면 처음 h를 보내고 ACK를 받는 대기시간동안 나머지 ello 데이터를
// 패킷으로 만들어 ACK를 받게 되면 그때 패킷을 보내준다. 따라서 5개의 패킷을 2개의 패킷으로 수를 줄여서 네트워크 데이터 송신의
// 비효율적인 사용을 막는 알고리즘이다. 물론 이 알고리즘에 의해 네트워크의 효율성이 좋아지지만 첫 데이터를 전송후 ACK를 기다리기 때문에
// 상대적으로 속도가 떨어지게 된다. 따라서 소켓 옵션의 TCP_NODELAY옵션을 통해 네이글 알고리즘을 켜고 끄고 할 수 있다.

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
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

#pragma region 송수신버퍼 크기 재정의
	int oldoptval, optval, optlen;

	SOCKET senrev_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (senrev_sock == INVALID_SOCKET) err_quit("socket()");
	// 수신 버퍼의 크기를 얻는다.
	optlen = sizeof(optval);
	retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&optval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("수신 버퍼 크기 = %d바이트\n", optval);

	oldoptval = optval;

	for (int i = 0; i < 3; i++)
	{
		// 수신 버퍼의 크기를 두 배로 늘린다.
		optval *= 2;
		retval = setsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, sizeof(optval));
		if (retval == SOCKET_ERROR) err_quit("setsockopt()");

		optlen = sizeof(optval);
		retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
			(char*)&optval, &optlen);
		if (retval == SOCKET_ERROR) err_quit("getsockopt()");
		printf("수신 버퍼 크기 = %d바이트\n", optval);
	}

	// 수신 버퍼의 크기를 원상 복구 한다.
	retval = setsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	optlen = sizeof(oldoptval);
	retval = getsockopt(senrev_sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&oldoptval, &optlen);
	if (retval == SOCKET_ERROR) err_quit("getsockopt()");
	printf("수신 버퍼 크기 = %d바이트\n", oldoptval);
#pragma endregion
	
#pragma region SO_KEEPALIVE 옵션 
	// 좀비 섹션을 처리하기 위해 주기적으로 연결이 되어 있다고 신호를 보내주어야 한다.
	// 방법으로 SO_KEEPALIVE 옵션을 할당 해주면 된다. 이렇게 해주면 이해하기 쉽게 해석하자면 KEEPALIVE라는 패킷을 자동으로 보내주는
	// 옵션이라 생각하면 된다. 하지만 이 간격이 2시간이고 이 설정을 해주면 시스템의 레지스트리에 저장되어있고 이 디폴트값을 바꾸게 되는것이라 
	// 시스템 전체의 소켓에 대해서 적용된다. 따라서 일반적으로 권장되지 않고 Windows 2000 이후로 SIO_KEEPALIVE_VALS 를 이용해 각 소켓별로
	// KEEPALIVE의 값과 간격을 설정해 사용할 수 있다고 한다.
#pragma endregion

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	DWORD dwRet;
	tcp_keepalive ka;
	ka.onoff = 1;
	ka.keepalivetime = 1000;		// 1초 마다 keepalive 신호를 보낸다고 설정
	ka.keepaliveinterval = 1000;	// 응답이 없을시 1초마다 재전송한다. ms tcp의 경우 10회 재시도 한다.

	WSAIoctl(listen_sock, SIO_KEEPALIVE_VALS, &ka, sizeof(tcp_keepalive), 0, 0, &dwRet, NULL, NULL);


	// 윈속 종료
	WSACleanup();

	getchar();

	return 0;
}