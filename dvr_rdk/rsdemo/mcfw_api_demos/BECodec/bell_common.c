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

#include "bell_common.h"

#define SYSTEM_PLATFORM_BOARD             SYSTEM_PLATFORM_BOARD_VS

UInt32 gBell_enabledProcs[] = {
    SYSTEM_LINK_ID_M3VPSS,
    SYSTEM_LINK_ID_M3VIDEO,
};

char *gBell_cpuName[SYSTEM_PLATFORM_CPU_REV_MAX] = {
    "ES1.0",
    "ES1.1",
    "ES2.0",
    "ES2.1",
    "UNKNOWN",
};

char *gBell_boardName[SYSTEM_PLATFORM_BOARD_MAX] = {
    "UNKNOWN",
    "4x TVP5158 VS",
    "2x SII9135, 1x TVP7002 VC",
    "2x SIL1161A, 2x TVP7002 Catalog",
    "2x SIL1161A, 2x TVP7002 DVR",
	"1x VC0356(ISS), 1x SII9135 BELL"
};

char *gBell_boardRev[SYSTEM_PLATFORM_BOARD_REV_MAX] = {
    "UNKNOWN",
    "REV A",
    "REV B",
    "REV C",
    "DVR"
};

Int32 Bell_detectBoard()
{
    Int32 status;
    UInt32 boardRev;
    UInt32 cpuRev;
    UInt32 boardId;


    SystemVpss_PlatformInfo  platformInfo;

    status = System_linkControl(
                SYSTEM_LINK_ID_M3VPSS,
                SYSTEM_M3VPSS_CMD_GET_PLATFORM_INFO,
                &platformInfo,
                sizeof(platformInfo),
                TRUE
                );

    UTILS_assert(status==OSA_SOK);

    /* Get CPU version */
    cpuRev = platformInfo.cpuRev;
    if (cpuRev >= SYSTEM_PLATFORM_CPU_REV_MAX)
    {
        cpuRev = SYSTEM_PLATFORM_CPU_REV_UNKNOWN;
    }
    printf(" %u: MCFW  : CPU Revision [%s] !!! \r\n",
        OSA_getCurTimeInMsec(), gBell_cpuName[cpuRev]);

    /* Detect board */
    boardId = platformInfo.boardId;
    if (boardId >= SYSTEM_PLATFORM_BOARD_MAX)
    {
        boardId = SYSTEM_PLATFORM_BOARD_UNKNOWN;
    }
    printf(" %u: MCFW  : Detected [%s] Board !!! \r\n",
        OSA_getCurTimeInMsec(), gBell_boardName[boardId]);

    /* Get base board revision */
    boardRev = platformInfo.baseBoardRev;
    if (boardRev >= SYSTEM_PLATFORM_BOARD_REV_MAX)
    {
        boardRev = SYSTEM_PLATFORM_BOARD_REV_UNKNOWN;
    }
    printf(" %u: MCFW  : Base Board Revision [%s] !!! \r\n",
        OSA_getCurTimeInMsec(), gBell_boardRev[boardRev]);

    if (boardId != SYSTEM_PLATFORM_BOARD_UNKNOWN)
    {
        /* Get daughter card revision */
        boardRev = platformInfo.dcBoardRev;
        if (boardRev >= SYSTEM_PLATFORM_BOARD_REV_MAX)
        {
            boardRev = SYSTEM_PLATFORM_BOARD_REV_UNKNOWN;
        }
        printf(" %u: MCFW  : Daughter Card Revision [%s] !!! \r\n",
            OSA_getCurTimeInMsec(), gBell_boardRev[boardRev]);
    }

    return 0;
}

Int32 Bell_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad)
{
    UInt32 numProcs, procId;

    numProcs = sizeof(gBell_enabledProcs)/sizeof(gBell_enabledProcs[0]);

    for(procId=0; procId<numProcs; procId++)
    {
        if(enable)
        {
            System_linkControl(
                gBell_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START,
                NULL,
                0,
                TRUE
            );
        }
        else
        {
            System_linkControl(
                gBell_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_STOP,
                NULL,
                0,
                TRUE
            );
            if(printStatus)
            {
                SystemCommon_PrintStatus printStatus;

                memset(&printStatus, 0, sizeof(printStatus));

                printStatus.printCpuLoad = TRUE;
                printStatus.printTskLoad = printTskLoad;
                System_linkControl(
                    gBell_enabledProcs[procId],
                    SYSTEM_COMMON_CMD_PRINT_STATUS,
                    &printStatus,
                    sizeof(printStatus),
                    TRUE
                );
            }
            System_linkControl(
                gBell_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET,
                NULL,
                0,
                TRUE
            );
        }
    }

    return 0;
}

Int32 Bell_prfLoadPrint(Bool printTskLoad,Bool resetTskLoad)
{
    UInt32 numProcs, procId;

    numProcs = sizeof(gBell_enabledProcs)/sizeof(gBell_enabledProcs[0]);

    for(procId=0; procId<numProcs; procId++)
    {
        SystemCommon_PrintStatus printStatus;

        memset(&printStatus, 0, sizeof(printStatus));

        printStatus.printCpuLoad = TRUE;
        printStatus.printTskLoad = printTskLoad;
        System_linkControl(
                   gBell_enabledProcs[procId],
                   SYSTEM_COMMON_CMD_PRINT_STATUS,
                   &printStatus,
                   sizeof(printStatus),
                   TRUE);
        if (resetTskLoad)
        {
            System_linkControl(
               gBell_enabledProcs[procId],
               SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET,
               NULL,
               0,
               TRUE);
        }
        OSA_waitMsecs(500); // allow for print to complete
    }
    return 0;
}

Int32 Bell_memPrintHeapStatus()
{
    UInt32 numProcs, procId;
    SystemCommon_PrintStatus printStatus;

    memset(&printStatus, 0, sizeof(printStatus));
    numProcs = sizeof(gBell_enabledProcs)/sizeof(gBell_enabledProcs[0]);

    printStatus.printHeapStatus = TRUE;

    for(procId=0; procId<numProcs; procId++)
    {
        System_linkControl(
                gBell_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START,
                &printStatus,
                sizeof(printStatus),
                TRUE
            );
    }

    return 0;
}

Int32 Bell_GetSize(UInt32 outRes, UInt32 * width, UInt32 * height)
{
    switch (outRes)
    {
        case VSYS_STD_MAX:
            *width = 1920;
            *height = 1200;
            break;

        case VSYS_STD_720P_60:
            *width = 1280;
            *height = 720;
            break;
        case VSYS_STD_NTSC:
             *width = 720;
             *height = 480;
             break;
        case VSYS_STD_VGA_60:
            *width = 640;
            *height = 480;
            break;
        case VSYS_STD_CIF:
            *width = 352;
            *height = 288;
            break;
        case VSYS_STD_PAL :
           *width = 720;
           *height = 576;
           break;

        case VSYS_STD_XGA_60:
           *width = 1024;
           *height = 768;
           break;

        case VSYS_STD_SXGA_60:
           *width = 1280;
           *height = 1024;
           break;

        default:
        case VSYS_STD_1080I_60:
        case VSYS_STD_1080P_60:
        case VSYS_STD_1080P_50:
        case VSYS_STD_1080P_30:
            *width = 1920;
            *height = 1080;
            break;


    }
    return 0;
}

/* forceLowCostScaling set to TRUE will take effect only when lineSkipMode is set to TRUE */
Void Bell_swMsGetDefaultLayoutPrm(UInt32 devId, SwMsLink_CreateParams *swMsCreateArgs, Bool forceLowCostScaling)
{
    SwMsLink_LayoutPrm *layoutInfo;
    SwMsLink_LayoutWinInfo *winInfo;
    UInt32 outWidth, outHeight, row, col, winId, widthAlign, heightAlign;
    VDIS_MOSAIC_S *mosaicParam;
    WINDOW_S *windowInfo;

    Bell_GetSize(swMsCreateArgs->maxOutRes, &outWidth, &outHeight);

    widthAlign = 8;
    heightAlign = 1;

    if(devId>=VDIS_DEV_MAX)
        devId = VDIS_DEV_HDMI;

    layoutInfo = &swMsCreateArgs->layoutPrm;

    /* init to known default */
    memset(layoutInfo, 0, sizeof(*layoutInfo));

    mosaicParam = &(gVdisModuleContext.vdisConfig.mosaicParams[devId]);
    if (mosaicParam->userSetDefaultSWMLayout == TRUE)
    {
      /* get default layout info from user define */
      layoutInfo->onlyCh2WinMapChanged = mosaicParam->onlyCh2WinMapChanged;
      layoutInfo->outputFPS = mosaicParam->outputFPS;
      layoutInfo->numWin = mosaicParam->numberOfWindows;
      for (winId = 0; winId < layoutInfo->numWin; winId++)
      {
        winInfo = &layoutInfo->winInfo[winId];
        windowInfo = &mosaicParam->winList[winId];
        winInfo->width  = windowInfo->width;
        winInfo->height = windowInfo->height;
        winInfo->startX = windowInfo->start_X;
        winInfo->startY = windowInfo->start_Y;
        winInfo->channelNum = mosaicParam->chnMap[winId];
        winInfo->bypass = mosaicParam->useLowCostScaling[winId];
      }
    }
    else /* using MCFW default layout info */
    {
      layoutInfo->onlyCh2WinMapChanged = FALSE;
      layoutInfo->outputFPS = mosaicParam->outputFPS;
      layoutInfo->numWin = 8;
      for(row=0; row<2; row++)
      {
          for(col=0; col<2; col++)
          {
              winId = row*2+col;

              winInfo = &layoutInfo->winInfo[winId];

              winInfo->width  = SystemUtils_floor((outWidth*2)/5, widthAlign);
              winInfo->height = SystemUtils_floor(outHeight/2, heightAlign);
              winInfo->startX = winInfo->width*col;
              winInfo->startY = winInfo->height*row;
              if (forceLowCostScaling == TRUE)
                winInfo->bypass = TRUE;
              else
                winInfo->bypass = FALSE;
              winInfo->channelNum = devId*(gVdisModuleContext.vdisConfig.numChannels/2) + winId;
          }
      }

      for(row=0; row<4; row++)
      {
          winId = 4 + row;

          winInfo = &layoutInfo->winInfo[winId];

          winInfo->width  = layoutInfo->winInfo[0].width/2;
          winInfo->height = layoutInfo->winInfo[0].height/2;
          winInfo->startX = layoutInfo->winInfo[0].width*2;
          winInfo->startY = winInfo->height*row;
          if (forceLowCostScaling == TRUE)
            winInfo->bypass = TRUE;
          else
            winInfo->bypass = FALSE;

          winInfo->channelNum = devId*(gVdisModuleContext.vdisConfig.numChannels/2) + winId;
      }
    }
}



