#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI ThreadProc(LPVOID lpParameter);

char recvbuf[1500] = { 0 };//���ܻ�����
char buf[1500] = { 0 };//���ִ���
char ip[30] = { 0 };//IP��ַ����
SOCKET socketServer;
BOOL b_Flag = TRUE;//����ѭ��
HANDLE pThread = NULL;

BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		b_Flag = FALSE;
		//�رվ��
			CloseHandle(pThread);
		free(pThread);
		//���������
		closesocket(socketServer);
		WSACleanup();

	}
	return TRUE;
}
int main(void)
{
	//�������
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdSockMsg;
	int nres = WSAStartup(wdVersion, &wdSockMsg);
	if (0 != nres)
	{
		switch (nres)
		{
		case WSASYSNOTREADY:
			printf("�������Ի��������");
			break;
		case WSAVERNOTSUPPORTED:
			printf("���������");
			break;
		case WSAEINPROGRESS:
			printf("��������");
			break;
		case WSAEPROCLIM:
			printf("�ر�������̨���");
			break;
		}
	}
	//У��汾
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		//���������
		WSACleanup();
		return 0;
	}
	//��������socket
	socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int a = WSAGetLastError();
	if (INVALID_SOCKET == socketServer)
	{
		//��ѯ������
		int a = WSAGetLastError();
		//���������
		WSACleanup();
		return 0;
	}
	printf("���������IPV4��ַ��\n");
	scanf_s("%s",ip,29);
	printf("����������ǳƣ�(�ǳƸ�ʽ��&��ͷ���ԣ���β��\n");
	scanf_s("%s", buf,1499);
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_port = htons(12349);
	si.sin_addr.S_un.S_addr = inet_addr(ip);
	if (SOCKET_ERROR == connect(socketServer, (struct sockaddr*)&si, sizeof(si)))
	{
 		int a = WSAGetLastError();
		//���������
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	pThread = (HANDLE*)malloc(sizeof(HANDLE));
	//�����߳̾��
	int i = 1;
		pThread = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL);
		if (NULL == pThread)
		{
			int a = GetLastError();
			printf("CreateThread Error:%d", a);
			return 0;
		}
		send(socketServer, buf, strlen(buf), 0);
	while (1)
	{
		scanf_s("%s", buf,1499);
		if ('0' == buf[0])
		{
			break;
		}
		if(SOCKET_ERROR == send(socketServer, buf, strlen(buf), 0))
		{
			//����
			int a = WSAGetLastError();
		}
		memset(buf, 0, 1024);
	}
	//���������
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	int i = 0;
	while (b_Flag)
	{
			if (SOCKET_ERROR == recv(socketServer, recvbuf, 1024, 0))
			{
				//error
				return 1;
			}
			if (0 != recvbuf[0])
			{
				
				printf("%s", recvbuf);
				i++;
				if (i == 2)
				{
					putchar('\n');
					i = 0;
				}
			}
			else
				continue;
			memset(recvbuf, 0, sizeof(recvbuf));
	}
	return 0;
}