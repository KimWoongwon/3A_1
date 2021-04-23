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

#define NICKNAME_MSG "��ȭ�� �Է��� ���������� �Ϸ� �Ǿ����ϴ�.\r\n"
#define ROOM_SELECT_MSG "ä�ù� ��ȣ�� �Է����ּ���.\r\n1.����Ʈ��\r\n2.�����ǹ�\r\n3.�־ȹ�\r\n"
#define SELECT_ROOM_MSG "%d�� �濡 �����ϼ̽��ϴ�.\r\n"

#define CHAT_ENTER_MSG "[ %s ]���� �����ϼ̽��ϴ�.\r\n"
#define CHAT_QUIT_MSG "[ %s ]���� �����ϼ̽��ϴ�.\r\n"
#define CHAT_ING_MSG "[ %s ] : %s\r\n"


enum STATE
{
	INIT_STATE = -1,

	NICKNAME_INPUT_STATE = 1,		// �г��� ���� ����
	NICKNAME_RESULT_STATE,			// �г��� ���� �� Send���ֱ� ���� �߰� ����

	SELECT_ROOM_MSG_STATE,			// �� �ȳ� �� ���� �޽��� �����ִ� ����
	SELECT_ROOM_MSG_END_STATE,		// �޽��� ������ �� Send���ֱ� ���� �߰� ����

	SELECT_ROOM_STATE,				// Ŭ���̾�Ʈ�� ������ ���� ������ �޾� ���� �������ִ� ����
	SELECT_ROOM_END_STATE,			// ������ ���� �� �� ��ٰ� Send���ִ� �߰� ����

	CHAT_ENTER_STATE,				// �� ��û�� �޾Ƽ� �ش� ���� ��� Ŭ���̾�Ʈ���� ����޽����� �߼��ϱ����� �غ� ����
	CHAT_ENTER_END_STATE,			// ��Ű¡ �س��� ��Ŷ�� ������ ���� ����
	CHAT_ING_STATE,					// Ŭ���̾�Ʈ������ ����� ���� ����
	CHAT_QUIT_STATE,				// Ŭ���̾�Ʈ�� ���� ������ ���ο� ���� ���� �ְ� �������ִ� ����
		
	DISCONNECTED_STATE,				// ���� ���� ����
};

enum RESULT
{
	NODATA = -1,
		
	NICKNAME_SUCCESS = 1,			// �г��� ������ �Ϸ� �Ǿ��ٰ� Ŭ���̾�Ʈ ���� �����ٶ� ����� ��

	SELECT_ROOM_MSGSEND,			// �� ����Ʈ�� �����ٶ� ����� ��
	SELECT_ROOM_SUCCESS,			// �� ������ �� �Ϸ� �Ǿ��ٰ� �����ٶ� ����� ��

	CHAT_ENTER_SUCCESS,				// �� ������ �� �Ϸ� ��� ���� �޽����� �����ٶ� ����� ��
	CHAT_MSG_SEND,					// ä���� �ְ�ް� �Ҷ� ����� ��
	CHAT_QUIT_SUCCESS,				// ���� �����°� ���������� �Ϸ��ߴٰ� ������ ����Ұ�
	
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
	SELECTROOMMSG,			// �� ����Ʈ ����ش޶�� Ŭ���̾�Ʈ�� ������ ��������
	CHOICEROOMNUM,			// 1, 2, 3�� ���� ����ٰ� Ŭ���̾�Ʈ�� ������ ��������
	CHATENTERREQUSET,		// �濡 ���� �ϰڴٰ� ����޽��� ����ش޶�� Ŭ���̾�Ʈ�� ������ ��������
	CHATING,				// ä���� �����ϴ� ���̶�� Ŭ���̾�Ʈ�� ������ ��������
	CHATQUIT,				// ä�ù濡�� �����ڴٰ� Ŭ���̾�Ʈ�� ������ ��������

	// server send protocol
	NICK_RESULT,			// �г��� ���� ������ ó���Ǿ��ٰ� ������� �Բ� �������� ��������
	ROOM_RESULT,			// �� ���� ������ ó���Ǿ��ٰ� ������� �Բ� �������� ��������
	CHAT_RESULT,			// ä�� ���� ó���� �Ϸ�Ǿ��ٰ� ������� �Բ� �������� ��������
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
	bool		isroomenter;		// ���� �濡 �� �ִ��� üũ�ϱ� ���� ����
	ROOM		curroomidx;			// ���� ��� �濡 ���ִ��� Ȯ���ϱ� ���� ����
};

_ClientInfo* User_List[100];
int Count = 0;

// select ���� ���� ����
FD_SET Rset, Sset;

void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
void GetProtocol(const char* _ptr, PROTOCOL& _protocol);

int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, int _proid, const char* _str1);	// Ŭ���̾�Ʈ�� �� ������ �ÿ� �ش� ���μ������̵� ���� ��ŷ�ϴ� �Լ�
int PackPacket(char* _buf, PROTOCOL _protocol, RESULT _result, const char* _str1);				// ������ ó���ϴ� ���� ������� �ű⿡ �´� ��������, �޽����� ��ŷ�Ǵ� �Լ�

void UnPackPacket(const char* _buf, char* _str1);			// Ŭ���̾�Ʈ�� ������ �г��� ������ ä�ø޽��� ���� ���� Ȥ�� ���信 �°� ó���� �ֱ� ���� ����ŷ �ϴ� �Լ�
void UnPackPacket(const char* _buf, int* _num);				// Ŭ���̾�Ʈ�� ������ ���ȣ Ȥ�� Ŭ���̾�Ʈ�� ���� ���μ������̵� ���� ����ŷ �ϴ� �Լ�

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

	// ���� ����
	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		err_quit("socket() error");

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(9000);

	// �ͺ��ŷ ���� ����
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
// ���� �Լ� ���� ���
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
	if (retval == SOCKET_ERROR) //�������������û�� ���
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
	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(_clientaddr.sin_addr),
		ntohs(_clientaddr.sin_port));

	//���� ����ü �迭�� ���ο� ���� ���� ����ü ����
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

	printf("\nClient ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(_ptr->addr.sin_addr), ntohs(_ptr->addr.sin_port));

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
		case NICKNAME:	// Ŭ���̾�Ʈ ������ �г��� ������ �����ָ� �޾Ƽ� �ߺ��˻� ���� �׳� ������ �Ϸ� �޽����� �����ش�.
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
		case SELECTROOMMSG:	// Ŭ���̾�Ʈ ������ �� ����Ʈ�� �޶�� ��û�ϸ� �� ����Ʈ�� �����ش�.
			_ptr->sendbytes = PackPacket(_ptr->sendbuf, ROOM_RESULT, SELECT_ROOM_MSGSEND, ROOM_SELECT_MSG);
			_ptr->state = SELECT_ROOM_MSG_END_STATE;
			break;
		}
		break;
	case SELECT_ROOM_STATE:
		switch (protocol)
		{
		case CHOICEROOMNUM:	// Ŭ���̾�Ʈ�� ���ȣ�� �Է��� �����ָ� ���������� �Է��� '%d�� �濡 �����ϼ̽��ϴ�.' ��� �޽����� �����ش�.
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
			// Ŭ���̾�Ʈ�� �޽����� ������ Ȯ�� ��ư�� ������ ���� ��û ���������� �����ش�.
			// �׷��� �ش� ���� ��� �����鿡�� ���� �޽��� ��� ���ش�.

			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, CHAT_ENTER_MSG, _ptr->nickname);

			// ������ ������ �ش� �濡 ���� �� �ִ� ��� Ŭ���̾�Ʈ���� �޽����� �ѷ��ش�.
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
			// ���Ŀ��� ä�� ������ �����ϸ鼭 �߰��� Ŭ���̾�Ʈ�� �泪���� ��ư�� ������� CHATQUIT�� �Ѿ��.
			
			memset(msg, 0, sizeof(msg));
			UnPackPacket(_ptr->recvbuf, msg);
						
			memset(finalmsg, 0, sizeof(finalmsg));
			sprintf(finalmsg, CHAT_ING_MSG, _ptr->nickname, msg);
			// ä�� ���� ���۵� ���������� �ڽ��� ������ �ش���� ������ ��� Ŭ���̾�Ʈ���� �ѷ��ش�.
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
			// CHAT_ING_STATE������ QUIT�� ���� ó���� �����ʰ� CHAT_QUIT_STATE�� �Ѱ��ش�.
			// ���� Ŭ���̾�Ʈ ���� send�� 2���ؼ� CHAT_QUIT_STATE�� �Ѿ�� �ְ� ���ش�.
			_ptr->state = CHAT_QUIT_STATE;
			_ptr->isroomenter = false;
			break;
		}
		

		break;
	case CHAT_QUIT_STATE:
		switch (protocol)	
		{
		case CHATQUIT:
			// �ι� ���� ���� CHATQUIT ��������
			// ��� Ŭ���̾�Ʈ���� �������û �� ���μ����� ���̵�� �Բ� ���信 �°� ������ ����޼����� ����Ѵ�.
			// Ȥ���� Ŭ���̾�Ʈ ������ �泪���� ��û�� ������ ���µ��� ���� �������� �ʰ� 
			// ���� ���̵� �ڽ��� ���μ��� ���̵�� ���ٸ� �泪���� �ϴ°����� �����ϰ� Ŭ���̾�Ʈ���� ó���Ѵ�.
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