#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <mswsock.h>
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

#define MAX_COUNT 1024
#define MAX_RECV_COUNT 1024

int SocketPrepore(void);//׼������
int SocketError(SOCKET sock);//�����ѯ
int PostRecv(int iIndex);//Ͷ����
int PostAccept(void);//Ͷ������
void Clear(void);//
int PostSend(int iIndex);//Ͷ�ݷ�
DWORD WINAPI ThreadProc(LPVOID lpParameter);//�ص�����

char g_strbuf[1024] = { 0 };//WSARecv���ջ�����
char g_name[20][20] = { 0 };//���ִ���
int i = 1;
SOCKET g_allsock[MAX_COUNT];//�������(���������������
OVERLAPPED g_alllap[MAX_COUNT];//�¼�����
int g_count;//������¼�����  !!������¼�����һһ��Ӧ!!!!
HANDLE hPort;//�˿�
int nProcessorsCount;//�߳�����(ϵͳ������)
HANDLE* pThread;
//BOOL���ڿ���ThreadProcѭ��
BOOL g_flag = TRUE;

BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT://���ڹرս����
		g_flag = FALSE;
		//�رվ��
		for (int i = 0; i < nProcessorsCount; i++)
		{
			CloseHandle(pThread[i]);
		}
		free(pThread);
		//�ͷ�����socket�ص�io��hPort
		Clear();
		//���������
		WSACleanup();
	}
	return TRUE;
}
int main(void)
{
	SetConsoleCtrlHandler(fun, TRUE);//�¼�����
	SOCKET _sockServer = (SOCKET)SocketPrepore();//׼������(�������....)
	if (0 == _sockServer)
	{
		//error
		printf("SocketPrepore Error\n");
		return 0;
	}
	if (0 != PostAccept())
	{
		//error
		printf("PostAccept Error\n");
		return 0;
	}
	//�����߳�����
	SYSTEM_INFO systemProcessorsCount;
	GetSystemInfo(&systemProcessorsCount);//��ȡϵͳ������,�����Ҳ�ѯ�ֶ�����
	nProcessorsCount = systemProcessorsCount.dwNumberOfProcessors;
	pThread = (HANDLE*)malloc(sizeof(HANDLE) * nProcessorsCount);
	//�����߳̾��
	for (int i = 0; i < nProcessorsCount; i++)
	{
		pThread[i] = CreateThread(NULL, 0, ThreadProc, hPort, 0, NULL);
		if (NULL == pThread[i])
		{
			int a = GetLastError();
			printf("CreateThread Error:%d", a);
			CloseHandle(hPort);
			return SocketError(_sockServer);
		}
	}
	while (1)
	{
		//����ɽ��������������������ᱻ��һ������
		Sleep(1000);
	}
	//ɾ���߳̾��
	for (int i = 0; i < nProcessorsCount; i++)
	{
		CloseHandle(pThread[i]);
	}
	free(pThread);
	Clear();
	WSACleanup();
	system("pause");
	return 0;
}
int SocketPrepore(void)
{
	//��
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdsockMsg;
	int nRes = WSAStartup(wdVersion, &wdsockMsg);
	if (0 != nRes)
	{
		switch (nRes)
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
	if (2 != HIBYTE(wdsockMsg.wVersion) || 2 != LOBYTE(wdsockMsg.wVersion))
	{
		WSACleanup();
		return 0;
	}
	//������socket
	SOCKET sockServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == sockServer)
	{
		return SocketError(sockServer);
	}
	//�ṹ��
	struct sockaddr_in si = { 0 };
	si.sin_family = AF_INET;
	si.sin_port = htons(12349);//ע�ⲻҪռ��ϵͳ�Ѿ�ʹ�õĶ˿ںţ��ɲ�ѯ
	si.sin_addr.S_un.S_addr = inet_addr("172.18.9.165");//172.18.9.165
	if (SOCKET_ERROR == bind(sockServer, (const struct sockaddr*)&si, sizeof(si)))
	{
		return SocketError(sockServer);
	}
	g_allsock[g_count] = sockServer;//װ������
	g_alllap[g_count].hEvent = WSACreateEvent();
	g_count++;
	//�����˿�
	hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (0 == hPort)
	{
		int a = GetLastError();
		printf("%d", a);
		closesocket(sockServer);
		WSACleanup();
		return 0;
	}
	//��
	HANDLE hPort1 = CreateIoCompletionPort((HANDLE)sockServer, hPort, 0, 0);
	if (hPort1 != hPort)
	{
		int a = GetLastError();
		printf("%d", a);
		CloseHandle(hPort);
		closesocket(sockServer);
		WSACleanup();
		return 0;
	}
	if (SOCKET_ERROR == listen(sockServer, 20))
	{
		return SocketError(sockServer);
	}
	return sockServer;
}
int SocketError(SOCKET sock)
{
	closesocket(sock);
	WSACleanup();
	return 0;
}
int PostAccept()
{
	//�����ͻ��������������ߴ˺���
	g_allsock[g_count] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_alllap[g_count].hEvent = WSACreateEvent();
	char str[1024] = { 0 };
	DWORD dwRecvcount;
	BOOL bRes = AcceptEx(g_allsock[0], g_allsock[g_count], str, 0, sizeof(struct sockaddr_in) + 16,
		sizeof(struct sockaddr_in) + 16, &dwRecvcount, &g_alllap[0]);
	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//��������
		return 1;
	}
	return 0;
}
int PostRecv(int iIndex)
{
	WSABUF wsabuf = { 0 };
	wsabuf.buf = g_strbuf;
	wsabuf.len = MAX_RECV_COUNT;
	DWORD dwRecvCount;
	DWORD flags = 0;
	int nRes = WSARecv(g_allsock[iIndex], &wsabuf, 1, &dwRecvCount, &flags, &g_alllap[iIndex], NULL);
	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//��������
		return 1;
	}
	return 0;
}
int PostSend(int iIndex)
{
	WSABUF wsabuf = { 0 };
	wsabuf.buf = g_strbuf;
	wsabuf.len = MAX_RECV_COUNT;
	DWORD  dwBuffcount;
	DWORD dwFlags = 0;
	int nRes = WSASend(g_allsock[iIndex], &wsabuf, 1, &dwBuffcount, dwFlags, &g_alllap[iIndex], NULL);
	int a = WSAGetLastError();
	if (ERROR_IO_PENDING != a)
	{
		//����ִ�г���
		return 1;
	}
	return 0;
}
void Clear(void)
{
	for (int i = 0; i < g_count; i++)
	{
		if (g_allsock[i] == 0)
			continue;
		closesocket(g_allsock[i]);
		WSACloseEvent(g_alllap[i].hEvent);

	}
	CloseHandle(hPort);
}
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE port = (HANDLE)lpParameter;
	DWORD NumberOfBytesTransferred;
	ULONG_PTR index;
	LPOVERLAPPED lpOverlapped;
	while (g_flag)
	{
		BOOL bFlag = GetQueuedCompletionStatus(port, &NumberOfBytesTransferred, &index, &lpOverlapped, INFINITE);
		if (FALSE == bFlag)
		{
			int a = GetLastError();
			if (64 == a)
				printf("Client Force Close\n");
			continue;
		}
		//�����ź�
		//accept
		if (0 == index)
		{
			printf("accept success\n");
			HANDLE hPort1 = CreateIoCompletionPort((HANDLE)g_allsock[g_count], hPort, g_count, 0);
			if (hPort1 != hPort)
			{
				int a = GetLastError();
				printf("accept error:%d", a);
				closesocket(g_allsock[g_count]);
				continue;
			}
			//�¿ͻ���Ͷ��recv
			PostRecv(g_count);
			g_count++;
			PostAccept(g_count);
		}
		else//�ɶ���д
		{
			if (0 == NumberOfBytesTransferred)
			{
				//client����
				printf("Client Close\n");
				closesocket(g_allsock[index]);
				if (g_alllap[index].hEvent == 0)
					continue;//ֻ��Ϊ�˽������ӵ�ѭ��
				WSACloseEvent(g_alllap[index].hEvent);
				g_allsock[index] = 0;
				g_alllap[index].hEvent = NULL;
			}
			else if (0 != g_strbuf[0])
			{
				//recv
				if (g_strbuf[0] == '&')//�ж�����
				{
					strcpy(g_name[i], g_strbuf);//������������
					i++;
					memset(g_strbuf, 0, sizeof(g_strbuf));
					PostRecv(index);
					continue;
				}
				for (int i = 0; i < g_count; i++)//��ÿ���ͻ��˷���
				{
					if (i == index)
						continue;
					send(g_allsock[i], g_name[index], 20, 0);//�û���
					send(g_allsock[i], g_strbuf, 1024, 0);//����
					printf("%s\n", g_strbuf);
				}
				memset(g_strbuf, 0, sizeof(g_strbuf));
				PostRecv(index);
			}
			else
			{
				//send
				//PostSend(index);
			}
		}
	}
	return 0;
}