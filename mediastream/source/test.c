#include "mediastream.h"
#include "isoformat.h"

#pragma comment( lib, "Ws2_32.lib" )

ifly_mediarcv_t *g_pMediaRcv[16];

void DealMediaFrame(PFRAMEHDR pFrmHdr, u32 dwContext)
{
	static int i=0;
	
	if(pFrmHdr->m_tVideoParam.m_bKeyFrame)
		printf("******i=%d,media type=%d,width=%d,height=%d,IKey=%d,frame id=%d\n",i++,pFrmHdr->m_byMediaType,pFrmHdr->m_tVideoParam.m_wVideoWidth,pFrmHdr->m_tVideoParam.m_wVideoHeight,pFrmHdr->m_tVideoParam.m_bKeyFrame,pFrmHdr->m_dwFrameID);
}

int main()
{
	int i=0;
	u16 wRet;
	
	u8 byBuf[256<<10];
	
	TLocalNetParam tLocalNetParam;
	TRSParam tRSParam;
	
	memset(byBuf,0,sizeof(byBuf));
	
	mediastreamStartup();
	
	for(i=0;i<16;i++)
	{
		g_pMediaRcv[i] = NULL;
	}
	
	//for(i=0;i<16;i++)
	for(i=0;i<1;i++)
	{
		tLocalNetParam.m_tLocalNet.m_wRTPPort   = 64000+i*4;
		tLocalNetParam.m_tLocalNet.m_dwRTPAddr  = 0;
		
		tLocalNetParam.m_tLocalNet.m_wRTCPPort  = 64001+i*4;
		tLocalNetParam.m_tLocalNet.m_dwRTCPAddr	= 0;
		
		tLocalNetParam.m_dwRtcpBackAddr			= inet_addr("192.168.1.32");//inet_addr("118.26.246.127");//inet_addr("192.168.1.32");//inet_addr("118.26.245.168");
		tLocalNetParam.m_wRtcpBackPort			= 4009+i*4;
		
		g_pMediaRcv[i] = CreateMediaRcv(MAX_FRAME_SIZE,DealMediaFrame,i,0);
		wRet = SetMediaRcvLocalParam(g_pMediaRcv[i],tLocalNetParam);
		printf("###wRet1=%d\n",wRet);
		
		tRSParam.m_wFirstTimeSpan = 40;
		tRSParam.m_wSecondTimeSpan = 80;
		tRSParam.m_wThirdTimeSpan = 120;
		tRSParam.m_wRejectTimeSpan = 160;//160;
		//wRet = ResetMediaRcvRSFlag(g_pMediaRcv[i],tRSParam,TRUE);
		printf("###wRet2=%d\n",wRet);
		
		wRet = StartMediaRcv(g_pMediaRcv[i]);
		printf("###wRet3=%d\n",wRet);
	}
	
	while(1)
	{
		Sleep(10);
	}
	
	for(i=0;i<16;i++)
	{
		if(g_pMediaRcv[i])
		{
			DestroyMediaRcv(g_pMediaRcv[i]);
			g_pMediaRcv[i] = NULL;
		}
	}
	
	mediastreamCleanup();
	
	return 0;
}
