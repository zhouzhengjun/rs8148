/*******************************************************************************
 *                                                                             *
 *      Copyright (c) 2013 BroadEng Beijing - http://www.broadeng.net/         *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             * 
 *******************************************************************************
 *                                                                             *
 * Author:      Zhang Tao <zhangtao@broadeng.net>                              *
 * Version:     v1.0.0                                                         *
 *                                                                             *
 ******************************************************************************/
#include <bell_osd.h>
#include <ti_swosd_logo_224x30_yuv420sp.h>
#include <ti_swosd_logo_224x30_yuv422i.h>

#define OSD_BUF_HEAP_SR_ID          (0)

static UInt8   *osdBufBaseVirtAddr = NULL;
static UInt32   osdTotalBufSize = 0;
AlgLink_OsdChWinParams g_osdChParam[ALG_LINK_OSD_MAX_CH];
AlgLink_OsdChBlindWinParams g_osdChBlindParam[ALG_LINK_OSD_MAX_CH];

Int32 Bell_osdInit(UInt32 numCh, UInt8 *osdFormat)
{
    int chId, winId, status;

    Vsys_AllocBufInfo bufInfo;
    UInt32 osdBufSize, osdBufSizeY, bufAlign;

    UInt32 bufOffset;
    UInt8 *curVirtAddr;

	assert(numCh <= ALG_LINK_OSD_MAX_CH);

	osdBufSizeY = BELL_OSD_WIN_PITCH_H*BELL_OSD_WIN_PITCH_V;

	osdBufSize = osdBufSizeY * 2 ;

    /* All channels share the same OSD window buffers, this is just for demo, actually each CH
        can have different OSD buffers
    */
    osdTotalBufSize = osdBufSize * BELL_OSD_NUM_WINDOWS;
    bufAlign = 128;

    status = Vsys_allocBuf(OSD_BUF_HEAP_SR_ID, osdTotalBufSize, bufAlign, &bufInfo);
    OSA_assert(status==OSA_SOK);

    osdBufBaseVirtAddr = bufInfo.virtAddr;

    for(chId = 0; chId < numCh; chId++)
    {
        AlgLink_OsdChWinParams * chWinPrm = &g_osdChParam[chId];

        AlgLink_OsdChBlindWinParams *chBlindWinPrm = &g_osdChBlindParam[chId];

        chWinPrm->chId = chId;
        chWinPrm->numWindows = BELL_OSD_NUM_WINDOWS;

        chWinPrm->colorKey[0] = 0xfa; /* Y */
        chWinPrm->colorKey[1] = 0x7d; /* U */
        chWinPrm->colorKey[2] = 0x7e; /* V */

        chBlindWinPrm->chId = chId;
        chBlindWinPrm->numWindows = BELL_OSD_NUM_BLIND_WINDOWS;

        bufInfo.virtAddr = osdBufBaseVirtAddr;

        for(winId=0; winId < chWinPrm->numWindows; winId++)
        {
            chWinPrm->winPrm[winId].startX             = BELL_OSD_WIN0_STARTX ;
            chWinPrm->winPrm[winId].startY             = BELL_OSD_WIN0_STARTY + (BELL_OSD_WIN_HEIGHT+BELL_OSD_WIN0_STARTY)*winId;
            chWinPrm->winPrm[winId].width              = BELL_OSD_WIN_WIDTH;
            chWinPrm->winPrm[winId].height             = BELL_OSD_WIN_HEIGHT;
            chWinPrm->winPrm[winId].lineOffset         = BELL_OSD_WIN_PITCH_H;
            chWinPrm->winPrm[winId].globalAlpha        = BELL_OSD_GLOBAL_ALPHA/(winId+1);
            chWinPrm->winPrm[winId].transperencyEnable = BELL_OSD_TRANSPARENCY;
            chWinPrm->winPrm[winId].enableWin          = BELL_OSD_ENABLE_WIN;

            chBlindWinPrm->winPrm[winId].startX        = BELL_OSD_WIN_WIDTH + 4 + BELL_OSD_WIN0_STARTX ;
            chBlindWinPrm->winPrm[winId].startY        = BELL_OSD_WIN0_STARTY + (BELL_OSD_WIN_HEIGHT+BELL_OSD_WIN0_STARTY)*winId;
            chBlindWinPrm->winPrm[winId].width         = 40;
            chBlindWinPrm->winPrm[winId].height        = BELL_OSD_WIN_HEIGHT;
            chBlindWinPrm->winPrm[winId].fillColorYUYV = 0x80008000;
            chBlindWinPrm->winPrm[winId].enableWin     = BELL_OSD_ENABLE_WIN;

            bufOffset = osdBufSize * winId;

            chWinPrm->winPrm[winId].addr[0][0] = (bufInfo.physAddr + bufOffset);

            curVirtAddr = bufInfo.virtAddr + bufOffset;

            /* copy logo to buffer  */
            if(osdFormat[chId] == SYSTEM_DF_YUV422I_YUYV)
            {
                chWinPrm->winPrm[winId].format     = SYSTEM_DF_YUV422I_YUYV;
                chWinPrm->winPrm[winId].addr[0][1] = NULL;
                OSA_assert(sizeof(gMCFW_swosdTiLogoYuv422i)<=osdBufSize);
                memcpy(curVirtAddr, gMCFW_swosdTiLogoYuv422i, sizeof(gMCFW_swosdTiLogoYuv422i));
            }
            else
            {
                chWinPrm->winPrm[winId].format     = SYSTEM_DF_YUV420SP_UV;
                chWinPrm->winPrm[winId].addr[0][1] =  chWinPrm->winPrm[winId].addr[0][0] + osdBufSizeY;
                OSA_assert(sizeof(gMCFW_swosdTiLogoYuv420sp)<= osdBufSize);
                memcpy(curVirtAddr, gMCFW_swosdTiLogoYuv420sp, sizeof(gMCFW_swosdTiLogoYuv420sp));
            }
        }
    }
    return status;
}

Void Bell_osdDeinit()
{
	if(osdBufBaseVirtAddr != NULL)
	{
		Vsys_freeBuf(OSD_BUF_HEAP_SR_ID, osdBufBaseVirtAddr, osdTotalBufSize);
	}
}
