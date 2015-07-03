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

#include <bell_blindArea.h>

CaptureLink_BlindInfo g_blindAreaChParam[CAPTURE_LINK_MAX_CH_PER_OUT_QUE];

Int32 Bell_blindAreaInit(UInt32 numCh)
{
    int chId, winId, numChanPerQueue = numCh;

    for(chId = 0; chId < numCh; chId++)
    {
        VCAP_CHN_DYNAMIC_PARAM_S params = { 0 };
        CaptureLink_BlindInfo * blindInfo;
        blindInfo = &g_blindAreaChParam[chId];

        numChanPerQueue = numCh;

        if(chId < numChanPerQueue)
        {
          blindInfo->queId     = 0;
          blindInfo->channelId = chId;
        }
        else
        {
          blindInfo->queId     = 1;
          blindInfo->channelId = chId - numChanPerQueue;
        }

        blindInfo->numBlindArea = BELL_BLIND_AREA_NUM_WINDOWS;
        for(winId=0; winId < blindInfo->numBlindArea; winId++)
        {
            blindInfo->win[winId].enableWin = FALSE;
            blindInfo->win[winId].fillColorYUYV= 0x80108010;
            blindInfo->win[winId].startX = BELL_BLIND_AREA_WIN0_STARTX;
            blindInfo->win[winId].startY = BELL_BLIND_AREA_WIN0_STARTY + BELL_BLIND_AREA_WIN0_STARTY * winId +( BELL_BLIND_AREA_WIN_HEIGHT * winId);
            blindInfo->win[winId].width  = BELL_BLIND_AREA_WIN_WIDTH;
            blindInfo->win[winId].height = BELL_BLIND_AREA_WIN_HEIGHT;
        }
        memcpy(&params.captureBlindInfo,blindInfo,sizeof(CaptureLink_BlindInfo));
        Vcap_setDynamicParamChn(chId, &params, VCAP_BLINDAREACONFIG);
    }
    return 0;
}
