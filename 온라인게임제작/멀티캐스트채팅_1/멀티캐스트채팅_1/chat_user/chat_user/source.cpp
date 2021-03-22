#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512

enum PROTOCOL
{
	CHATINFOREQ = 1, ADDRREQ
};

struct _RequestPacket
{
	PROTOCOL protocol;
	int Data;
}RequestPacket;

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

DWORD CALLBACK ChatProcess(LPVOID prt);

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");	

	
	SOCKADDR_IN remoteaddr;
	ZeroMemory(&remoteaddr, sizeof(remoteaddr));
	remoteaddr.sin_family = AF_INET;
	remoteaddr.sin_port = htons(9900);
	remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	SOCKADDR_IN peeraddr;
	int addrlen;
	char buf[BUFSIZE+1];
	ZeroMemory(&RequestPacket, sizeof(RequestPacket));	

	RequestPacket.protocol=CHATINFOREQ;
	retval=sendto(sock, (char*)&RequestPacket, sizeof(RequestPacket), 0, (SOCKADDR*)&remoteaddr, sizeof(SOCKADDR_IN));
	if(retval == SOCKET_ERROR){
		err_quit("recvfrom()");		
	}

	while(1)
	{
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0, 
			(SOCKADDR *)&peeraddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		if(memcmp(&peeraddr,&remoteaddr, sizeof(SOCKADDR_IN)))
		{
			continue;
		}
		break;
	}

	// 받은 데이터 출력
	buf[retval] = '\0';
	printf("%s\n",buf);

	scanf("%d", &RequestPacket.Data);
	while (getchar() != '\n');

	RequestPacket.protocol=ADDRREQ;

	retval=sendto(sock, (char*)&RequestPacket, sizeof(RequestPacket), 0, (SOCKADDR*)&remoteaddr, sizeof(SOCKADDR_IN));
	if(retval == SOCKET_ERROR){
		err_quit("recvfrom()");		
	}

	SOCKADDR_IN multicastaddr, localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));

	while(1)
	{			
		retval = recvfrom(sock, (char*)&multicastaddr, sizeof(multicastaddr), 0,
			(SOCKADDR *)&peeraddr, &addrlen);
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}

		if(memcmp(&peeraddr,&remoteaddr, sizeof(SOCKADDR_IN)))
		{
			continue;
		}	

		break;
	}

	closesocket(sock);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");	

	BOOL optval = true;
	retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
		(char *)&optval, sizeof(optval));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");	

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = multicastaddr.sin_port;
	localaddr.sin_addr.s_addr =inet_addr("127.0.0.1");

	retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");	

	// 멀티캐스트 그룹 가입
	struct ip_mreq mreq;
	memcpy(&mreq.imr_multiaddr.s_addr, &multicastaddr.sin_addr, sizeof(multicastaddr.sin_addr));
	
	mreq.imr_interface.s_addr = inet_addr("127.0.0.1");

	retval = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char *)&mreq, sizeof(mreq));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()fgfg");	
	
	
	char Nname[BUFSIZE];
	printf("대화명입력:");
	
	gets_s(Nname, BUFSIZE);

	DWORD ThreadId;
	CreateThread(NULL, 0, ChatProcess, (LPVOID)sock, 0, &ThreadId);

	while(1)
	{
		ZeroMemory(buf, sizeof(buf));
		sprintf(buf,"[%s]:", Nname);
		printf("%s", buf);
		
		gets_s(buf+strlen(buf), BUFSIZE-strlen(buf));

		retval=sendto(sock, buf,strlen(buf)+1,0,(SOCKADDR*)&multicastaddr, sizeof(multicastaddr));
		if(retval == SOCKET_ERROR){
			err_display("recvfrom()");
			continue;
		}		

	}
	// 멀티캐스트 그룹 탈퇴
	retval = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&mreq, sizeof(mreq));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

DWORD CALLBACK ChatProcess(LPVOID ptr)
{
	SOCKET sock = (SOCKET)ptr;

	SOCKADDR_IN peeraddr;
	int addrlen=sizeof(peeraddr);
	char buf[BUFSIZE + 1];
	ZeroMemory(&peeraddr, sizeof(peeraddr));

	while (1)
	{
		int retval = recvfrom(sock, buf, sizeof(buf), 0,(SOCKADDR *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR){
			break;
		}
		printf("%s\n", buf);
	}

	return 0;
}