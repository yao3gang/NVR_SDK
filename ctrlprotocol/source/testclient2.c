#include "ctrlprotocol.h"
#include <stdio.h>

#pragma comment( lib, "Ws2_32.lib" )

int main()
{
	ifly_monitor_param_t mp;
	u32 dwServerIp;
	u16 wServerPort;

	char szHostName[64];
	struct hostent* pHostLocal;
	int  count=0;
	
	u16 wRet;
	DWORD start,end;

	int i;
	
	CPLibInit(TRUE);

	//WSANOTINITIALISED
	mp.byChn = 0;
	//mp.dwIp  = inet_addr("192.168.1.120");
	gethostname(szHostName, sizeof(szHostName));
	pHostLocal = gethostbyname(szHostName);
	
	//count = pHostLocal->h_length/sizeof(struct in_addr);
	for(i=0;;i++)
	{
		count++;
		if(pHostLocal->h_addr_list[i] + pHostLocal->h_length >= pHostLocal->h_name)
		{
			break;
		}	
	}

	mp.dwIp = inet_addr(inet_ntoa(*(struct in_addr *)pHostLocal->h_addr_list[0]));
	printf("szHostName=%s,mp.dwIp=0x%08x,ip count=%d,all_addr len=%d,in_addr len=%d\n",szHostName,mp.dwIp,count,pHostLocal->h_length,sizeof(struct in_addr));
	
	mp.wPort = 64000;
	
	//dwServerIp = inet_addr("192.168.1.168");
	dwServerIp = inet_addr("127.0.0.1");
	wServerPort = CTRL_PROTOCOL_SERVERPORT;

	start = GetTickCount();
	wRet = StartNetMonitor(mp,dwServerIp,wServerPort,20*CTRL_PROTOCOL_CONNECT_DEFAULT);
	end = GetTickCount();
	printf("StartNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);

	mp.byChn = 0;
	start = GetTickCount();
	//wRet = StopNetMonitor(mp,dwServerIp,wServerPort,CTRL_PROTOCOL_CONNECT_BLOCK);
	end = GetTickCount();
	printf("StopNetMonitor:wRet=%d,start=%d,end=%d,span=%d\n",wRet,start,end,end-start);

	CPLibCleanup(TRUE);
	
	return 0;
}
