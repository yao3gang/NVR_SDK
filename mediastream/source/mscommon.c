#include "mscommon.h"

#ifndef WIN32
#include <sys/time.h>		/* struct timeval */
#include <sys/param.h>
#endif

BOOL SemBCreate(SEMHANDLE *phSema)
{
	if(phSema == NULL)
    {
		return FALSE;
	}
	
#ifdef WIN32
    *phSema = CreateSemaphore(NULL, 1, 1, NULL);
    return (*phSema != NULL);
#else
	int nRet;
	
	SEMHANDLE hSema = (SEMHANDLE)malloc(sizeof(sem_t));
	assert(hSema != NULL);
	
	nRet = sem_init(hSema, 0, 1);
	if(nRet != 0)
	{
		free(hSema);
		return FALSE;
	}
	
	*phSema = hSema;
	return TRUE;
#endif
}

BOOL SemCCreate(SEMHANDLE *phSema, u32 dwInitCount, u32 dwMaxCount)
{
	if(phSema == NULL)
	{
		return FALSE;
	}
	
#ifdef WIN32
    *phSema = CreateSemaphore(NULL, dwInitCount, dwMaxCount, NULL);
    return (*phSema != NULL);
#else
	int nRet;
	
	SEMHANDLE hSema = (SEMHANDLE)malloc(sizeof(sem_t));
	assert(hSema != NULL);
	
	nRet = sem_init(hSema, 0, dwInitCount);
	if(nRet != 0)
	{
		free(hSema);
		return FALSE;
	}
	
	*phSema = hSema;
	return TRUE;
#endif
}

BOOL SemDelete(SEMHANDLE hSema)
{
#ifdef WIN32    
    return CloseHandle(hSema);
#else
	int nRet;
	
	nRet = sem_destroy(hSema);
	free(hSema);
	return nRet == 0;
#endif
}

BOOL SemTake(SEMHANDLE hSema)
{
#ifdef WIN32
    return ( WAIT_FAILED != WaitForSingleObject(hSema, INFINITE) );
#else
	int nRet;
	
	nRet = sem_wait(hSema);
	return (nRet==0);
#endif
}

BOOL SemTakeByTime(SEMHANDLE hSema, u32 dwTimeout)
{
#ifdef WIN32
    return ( WAIT_OBJECT_0 == WaitForSingleObject(hSema, dwTimeout));
#else
	int nRet;
	
	nRet = sem_wait(hSema);
	return (nRet==0);
#endif
}

BOOL SemGive(SEMHANDLE hSema)
{
#ifdef WIN32
    u32 previous;
	
    return ReleaseSemaphore(hSema, 1, (LPLONG)&previous);
#else
	int nRet;
	
	nRet = sem_post(hSema);
	
	return nRet == 0;
#endif
}

u32 TickGet()
{
#ifdef WIN32
    return GetTickCount();
#else
	struct timeval tv;
	
	//printf("HZ=%d\n",HZ);//HZ=100
	
	gettimeofday(&tv, NULL);
	
	#ifdef RS_ENABLE
	return tv.tv_sec*HZ + (tv.tv_usec*HZ)/1000000;
	#else
	return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	#endif
#endif
}

u32 GetSystemTick()
{
#ifdef WIN32
    return GetTickCount();
#else
	struct timeval tv;
	
	//printf("HZ=%d\n",HZ);//HZ=100
	
	gettimeofday(&tv, NULL);
	
	#ifdef RS_ENABLE
	return tv.tv_sec*HZ + (tv.tv_usec*HZ)/1000000;
	#else
	return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	#endif
#endif
}

u32 SystemClkRateGet()
{
#ifdef WIN32
    return 1000;
#else
	#ifdef RS_ENABLE
	return HZ;
	#else
	return 1000;
	#endif
#endif
}

BOOL SockInit()
{
#ifdef WIN32
    WSADATA wsaData;
    u32 err;
	
    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(err != 0)
	{
		return FALSE;
	}
#endif
	
	return TRUE;
}

BOOL SockCleanup()
{
#ifdef WIN32
    u32 err;
    err = WSACleanup();
	if(err != 0) return FALSE;	
#endif
	
	return TRUE;
}

BOOL SockClose(SOCKHANDLE hSock)
{
	if(hSock == INVALID_SOCKET)
	{
		return FALSE;
	}
	
#ifdef WIN32
    return ( 0 == closesocket(hSock) ); 
#else
	/* 不准用户关闭标准输入\输出 */
	if(hSock < 3)
	{
		return FALSE;
	}
    
    return ( 0 == close(hSock) );	
#endif
}

#ifdef WIN32

#ifdef WIN32
THREADHANDLE IFly_CreateThread(void* pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID)
#else
THREADHANDLE IFly_CreateThread(LINUXFUNC pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID)
#endif
{
	THREADHANDLE  hTask;
	
#ifdef WIN32
	u32 dwTaskID;
	int Priority;

	if(szName == NULL){} /* 用于避免告警 */
	wFlag = 0; /* 用于避免告警 */

	if(byPriority < 50)
	{
		Priority = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else if(byPriority < 100)
	{
		Priority = THREAD_PRIORITY_HIGHEST;
	}
	else if(byPriority < 120)
	{
		Priority = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else if(byPriority < 150)
	{
		Priority = THREAD_PRIORITY_NORMAL;
	}
	else if(byPriority < 200)
	{
		Priority = THREAD_PRIORITY_BELOW_NORMAL;		
	}	
	else
	{
		Priority = THREAD_PRIORITY_LOWEST;
	}
	
    hTask = CreateThread(NULL, dwStacksize, (LPTHREAD_START_ROUTINE)pvTaskEntry, (char * )dwParam, 0, &dwTaskID);
	if(hTask != NULL)
    {
		SetThreadAffinityMask(hTask, THREADAFFMASK); // 设置线程的处理器姻亲掩码

		if(SetThreadPriority(hTask, Priority) != 0)  
		{
			if(pdwTaskID != NULL) 
			{
				*pdwTaskID = dwTaskID;
			}
			return hTask;
		}
    }
#else
	
	int nRet = 0;
	struct sched_param tSchParam;	
	pthread_attr_t tThreadAttr;
	int nSchPolicy;

	pthread_attr_init(&tThreadAttr);

	// 设置调度策略
	pthread_attr_getschedpolicy(&tThreadAttr, &nSchPolicy);
	nSchPolicy = SCHED_FIFO;
	pthread_attr_setschedpolicy(&tThreadAttr, nSchPolicy);

	// 设置优先级
	pthread_attr_getschedparam(&tThreadAttr, &tSchParam);
	byPriority = 255-byPriority;
	if(byPriority < 60)
	{
		byPriority = 60;
	}
	tSchParam.sched_priority = byPriority;
	pthread_attr_setschedparam(&tThreadAttr, &tSchParam);

	pthread_attr_setstacksize(&tThreadAttr, (size_t)dwStacksize);

	nRet = pthread_create(&hTask, &tThreadAttr, pvTaskEntry, (void *)dwParam);
	if(nRet == 0)
	{
		if(pdwTaskID != NULL)
		{
			*pdwTaskID = (u32)hTask;
		}
		return hTask;
	}
#endif

    return 0;
}

BOOL IFly_ThreadExit()
{
#ifdef WIN32
	ExitThread(0);
	return TRUE;
#else
	static int nRetCode = 0;
	pthread_exit(&nRetCode);
	return TRUE;
#endif
}

BOOL IFly_ThreadTerminate(THREADHANDLE hTask)
{
#ifdef WIN32
	return TerminateThread(hTask, 0);
#else
	void *temp;
	
	pthread_cancel(hTask);
	return ( 0 == pthread_join(hTask, &temp) );
#endif
}

#endif

void InitLoopBuf(TLoopBuf *lp)
{
	if(lp)
	{
		lp->m_nBufLen     = 0;
		lp->m_nReadPos    = 0;
		lp->m_nSubLen     = 0;
		lp->m_nUintBufLen = 0;
		lp->m_nWritePos   = 0;
		lp->m_pBuf        =	NULL;
	}
}

TLoopBuf* CreateLoopBuf(INT32 nUnitBufLen, INT32 nUnitBufNum)
{
	TLoopBuf *lp;
	if(nUnitBufLen<=0 || nUnitBufNum<=0)
	{
		return NULL;
	}
	lp = (TLoopBuf *)malloc(sizeof(TLoopBuf));
	if(lp == NULL)
	{
		return NULL;
	}
	InitLoopBuf(lp);
	
	lp->m_nBufLen = (nUnitBufLen+sizeof(INT32))*nUnitBufNum;//加上长度单元
	lp->m_nUintBufLen = nUnitBufLen+sizeof(INT32);
	lp->m_pBuf = (u8 *)malloc(lp->m_nBufLen);
	if(lp->m_pBuf == NULL)
	{
		free(lp);
		return NULL;
	}
	return lp;
}

u16 ReadLoopBuf(TLoopBuf *lp,u8 *pBuf, INT32 *pnBufSize)
{
	INT32 nRealLen;

	if(lp==NULL||pBuf==NULL||pnBufSize==NULL)
	{
		return ERROR_LOOP_BUF_PARAM;
	}

	if(lp->m_pBuf == NULL)
    {
        return ERROR_LOOP_BUF_NOCREATE;
    }
    
    if(lp->m_nSubLen < lp->m_nUintBufLen)
    {
        return ERROR_LOOP_BUF_NULL;
    }

	memcpy(&nRealLen,lp->m_pBuf+lp->m_nReadPos,sizeof(nRealLen));//取出数据长度
    
    if(nRealLen < 0 || nRealLen > (lp->m_nUintBufLen - (INT32)sizeof(INT32)))
    {
        return ERROR_LOOP_BUF_SIZE;
    }
    
	memcpy(pBuf, lp->m_pBuf + lp->m_nReadPos + sizeof(INT32), nRealLen);
    *pnBufSize = nRealLen;
	
	//修改标识位
    lp->m_nReadPos	+= lp->m_nUintBufLen;
    lp->m_nSubLen	-= lp->m_nUintBufLen;
    if(lp->m_nReadPos >= lp->m_nBufLen)
    {
        lp->m_nReadPos = 0;
    }
    return LOOPBUF_NO_ERROR;
}

u16 WriteLoopBuf(TLoopBuf *lp,u8 *pBuf, INT32 nBufSize)
{
	if( lp == NULL || pBuf == NULL || nBufSize > ( lp->m_nUintBufLen-(INT32)sizeof(INT32) ) )
	{
		return ERROR_LOOP_BUF_PARAM;
	}
	
	if(lp->m_pBuf == NULL)
    {
        return ERROR_LOOP_BUF_NOCREATE;
    }
    
    if(lp->m_nSubLen >= lp->m_nBufLen)
    {
        return ERROR_LOOP_BUF_FULL;
    }
	
	memcpy(lp->m_pBuf+lp->m_nWritePos,&nBufSize,sizeof(nBufSize));//写入数据长度
	
	memcpy(lp->m_pBuf + lp->m_nWritePos + sizeof(INT32), pBuf, nBufSize);
    	
	//修改标识位
	lp->m_nWritePos += lp->m_nUintBufLen;
	lp->m_nSubLen += lp->m_nUintBufLen;
    if(lp->m_nWritePos >= lp->m_nBufLen)
    {
        lp->m_nWritePos = 0;
    }
    
	return LOOPBUF_NO_ERROR;
}

u16 DestroyLoopBuf(TLoopBuf *lp)
{
	if(lp)
	{
		if(lp->m_pBuf)
		{
			free(lp->m_pBuf);
			lp->m_pBuf = NULL;
		}
		free(lp);
	}
	return LOOPBUF_NO_ERROR;
}

u32 SetBitField(u32 dwValue, u32 dwBitField, INT32 nStartBit, INT32 nBits)
{
	INT32 nMask = (1 << nBits) - 1;
    
    return (dwValue & ~(nMask << nStartBit)) + 
		((dwBitField & nMask) << nStartBit);
}

u32 GetBitField(u32 dwValue, INT32 nStartBit, INT32 nBits)
{
	INT32  nMask = (1 << nBits) - 1;
	
    return (dwValue >> nStartBit) & nMask; 
}

void ConvertH2N(u8 *pBuf, INT32 nStartIndex, INT32 nSize)
{
	INT32 i;
	
	if(((u32)pBuf % 4) == 0)
	{
		for(i=nStartIndex; i<(nStartIndex+nSize); i++) 
		{
			((u32 *)pBuf)[i] = htonl(((u32 *)pBuf)[i]);
		}
	}
	else
	{
		u32 value = 0;
		
		printf("########################ConvertH2N:addr is not align\n");
		fflush(stdout);
		
		for(i=nStartIndex; i<(nStartIndex+nSize); i++) 
		{
			memcpy(&value, pBuf + i * 4, sizeof(u32));
			value = htonl(value);
			memcpy(pBuf + i * 4, &value, sizeof(u32));
		}
	}
	
    return;
}

void ConvertN2H(u8 *pBuf, INT32 nStartIndex, INT32 nSize)
{
	INT32 i;
	
	if(((u32)pBuf % 4) == 0)
	{
		for(i=nStartIndex; i<(nStartIndex+nSize); i++) 
		{
			((u32 *)pBuf)[i] = ntohl(((u32 *)pBuf)[i]);
		}
	}
	else
	{
		u32 value = 0;
		
		printf("########################ConvertN2H:addr is not align\n");
		fflush(stdout);
		
		for(i=nStartIndex; i<(nStartIndex+nSize); i++) 
		{
			memcpy(&value, pBuf + i * 4, sizeof(u32));
			value = ntohl(value);
			memcpy(pBuf + i * 4, &value, sizeof(u32));
		}
	}
	
    return;
}
