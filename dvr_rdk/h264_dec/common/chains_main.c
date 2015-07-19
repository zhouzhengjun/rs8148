/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2014 Ruisu Xintong Technologies Co.Ltd                        *
 *					- http://www.ruisutech.com/      					       *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include <avcap/common/chains.h>
#include "ti_vdis_common_def.h"
#include <mcfw/interfaces/common_def/ti_vsys_common_def.h>

#define MAX_INPUT_STR_SIZE  (80)

Chains_Ctrl gChains_ctrl;

UInt32 gChains_enabledProcs[] = {
    SYSTEM_LINK_ID_M3VPSS,
    SYSTEM_LINK_ID_M3VIDEO,
    SYSTEM_LINK_ID_DSP,
};

Void Chains_setDefaultCfg()
{
    UInt32 displayResDefault[SYSTEM_DC_MAX_VENC] =
        {VSYS_STD_1080P_60,   //SYSTEM_DC_VENC_HDMI,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_HDCOMP,
         VSYS_STD_1080P_60,    //SYSTEM_DC_VENC_DVO2
         VSYS_STD_NTSC        //SYSTEM_DC_VENC_SD,
        };

    gChains_ctrl.enableNsfLink = TRUE;
    gChains_ctrl.enableOsdAlgLink = FALSE;
    gChains_ctrl.enableVidFrameExport = FALSE;
    gChains_ctrl.bypassNsf = FALSE;
    memcpy(gChains_ctrl.displayRes,displayResDefault,sizeof(gChains_ctrl.displayRes));
}

char *gChains_cpuName[SYSTEM_PLATFORM_CPU_REV_MAX] = {
    "ES1.0",
    "ES1.1",
    "ES2.0",
    "ES2.1",
    "UNKNOWN",
};

char *gChains_boardName[SYSTEM_PLATFORM_BOARD_MAX] = {
    "UNKNOWN",
    "4x TVP5158 VS",
    "2x SII9135, 1x TVP7002 VC",
    "2x SIL1161A, 2x TVP7002 Catalog",
    "2x SIL1161A, 2x TVP7002 DVR",
	"1x VC0356(ISS), 1x SII9135 BELL"
};

char *gChains_boardRev[SYSTEM_PLATFORM_BOARD_REV_MAX] = {
    "UNKNOWN",
    "REV A",
    "REV B",
    "REV C",
    "DVR"
};

Int32 Chains_detectBoard()
{
    Int32 status;
    UInt32 boardRev, boardId, cpuRev;

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
    printf(" %u: CHAINS  : CPU Revision [%s] !!! \r\n",
        OSA_getCurTimeInMsec(), gChains_cpuName[cpuRev]);

    /* Detect board */
    boardId = platformInfo.boardId;
    if (boardId >= SYSTEM_PLATFORM_BOARD_MAX)
    {
        boardId = SYSTEM_PLATFORM_BOARD_UNKNOWN;
    }
    printf(" %u: CHAINS  : Detected [%s] Board !!! \r\n",
        OSA_getCurTimeInMsec(), gChains_boardName[boardId]);

    /* Get base board revision */
    boardRev = platformInfo.baseBoardRev;
    if (boardRev >= SYSTEM_PLATFORM_BOARD_REV_MAX)
    {
        boardRev = SYSTEM_PLATFORM_BOARD_REV_UNKNOWN;
    }
    printf(" %u: CHAINS  : Base Board Revision [%s] !!! \r\n",
        OSA_getCurTimeInMsec(), gChains_boardRev[boardRev]);

    if (boardId != SYSTEM_PLATFORM_BOARD_UNKNOWN)
    {
        /* Get daughter card revision */
        boardRev = platformInfo.dcBoardRev;
        if (boardRev >= SYSTEM_PLATFORM_BOARD_REV_MAX)
        {
            boardRev = SYSTEM_PLATFORM_BOARD_REV_UNKNOWN;
        }
        printf(" %u: CHAINS  : Daughter Card Revision [%s] !!! \r\n",
            OSA_getCurTimeInMsec(), gChains_boardRev[boardRev]);
    }

    return 0;
}

Void Chains_run(Chains_RunFunc chainsRunFunc)
{
    Chains_Ctrl chainsCtrl;

    Chains_detectBoard();

    System_linkControl(
        SYSTEM_LINK_ID_M3VPSS,
        SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES,
        NULL,
        0,
        TRUE
        );

    memcpy(&chainsCtrl, &gChains_ctrl, sizeof(gChains_ctrl));

    Chains_memPrintHeapStatus();

    UTILS_assert(  chainsRunFunc!=NULL);
    chainsRunFunc(&chainsCtrl);

    Chains_memPrintHeapStatus();
}

char gChains_menuMain0[] = {
    "\r\n ============"
    "\r\n Chain Select"
    "\r\n ============"
    "\r\n"
};

char gChains_menuMainVs[] = {
    "\r\n"
    "\r\n 1: Multi  CH Capture + NSF     + DEI     + Display           (SI9135, NTSC, YUV422I )"
    "\r\n "
    "\r\n s: System Settings "
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
    "\r\n Enter Choice: "
};

Void Chains_menuMainShow()
{
    printf(gChains_menuMain0);

    Chains_menuCurrentSettingsShow();

    printf(gChains_menuMainVs);
}

Void Chains_menuMainRunVs(char ch)
{
    gChains_ctrl.enableNsfLink = FALSE;

    switch(ch)
    {
        case '1':
            gChains_ctrl.enableNsfLink = TRUE;
            Chains_run(Chains_multiChCaptureNsfDei);
            break;
        default:
            break;
    }
}

#if 0
int main ( int argc, char **argv )
{
    System_init();

    Chains_main();

    System_deInit();

    return (0);
}
#endif
Void Chains_main()
{
    char ch[MAX_INPUT_STR_SIZE];
    Bool done;

    done = FALSE;

    Chains_setDefaultCfg();

    while(!done)
    {
        Chains_menuMainShow();

        fgets(ch, MAX_INPUT_STR_SIZE, stdin);
        if(ch[1] != '\n' || ch[0] == '\n')
            continue;

        printf(" \r\n");

        Chains_menuMainRunVs(ch[0]);

        switch(ch[0])
        {
            case 's':
                Chains_menuSettings();
                break;
            case 'x':
                done = TRUE;
                break;
            case 'd':
                Chains_detectBoard();
                break;
        }

    }
}

Void Chains_menuCurrentSettingsShow()
{
    static char *nsfModeName[] =
        { "SNF + TNF", "CHR DS ONLY" };

    printf("\r\n Current System Settings,");
    printf("\r\n NSF Mode              : %s", nsfModeName[gChains_ctrl.bypassNsf]);
}

char gChains_menuSettings0[] = {
    "\r\n ==============="
    "\r\n System Settings"
    "\r\n ==============="
    "\r\n"
};

char gChains_menuSettings1[] = {
    "\r\n"
    "\r\n 1: NSF Bypass Mode"
    "\r\n "
    "\r\n x: Exit "
    "\r\n "
};

Void Chains_menuSettingsShow()
{
    printf(gChains_menuSettings0);

    Chains_menuCurrentSettingsShow();

    printf(gChains_menuSettings1);
}

Void Chains_menuSettings()
{
    char ch[MAX_INPUT_STR_SIZE];
    Bool done = FALSE;
    char inputStr[MAX_INPUT_STR_SIZE];
    Int32 value;

    Chains_menuSettingsShow();

    while(!done)
    {
        printf("\r\n Enter Choice: ");

    fgets(ch, MAX_INPUT_STR_SIZE, stdin);
    if(ch[1] != '\n' || ch[0] == '\n')
        continue;
        printf(" \r\n");

        switch(ch[0])
        {
            case '1':
                printf(" \r\n Enter NSF Mode [1: CHR DS ONLY, 2: SNF + TNF] : ");
                fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
                value = atoi(inputStr);

                if(value==1)
                    gChains_ctrl.bypassNsf = TRUE;
                if(value==2)
                    gChains_ctrl.bypassNsf = FALSE;

                break;

            case 'x':
                done = TRUE;
                break;
        }
    }
}

Int32 Chains_displayCtrlInit(UInt32 displayRes[])
{
    Int32   status;
    UInt16 i, j, width, height;

    VDIS_PARAMS_S prm;
	VDIS_PARAMS_S * pContext =&prm;
    for (i = 0; i < SYSTEM_DC_MAX_VENC; i++)
    {
        prm.deviceParams[i].resolution = displayRes[i];
    }

    memset(pContext, 0, sizeof(VDIS_PARAMS_S));
	
    for(i = 0; i < VDIS_DEV_MAX; i++)
    {
        pContext->deviceParams[i].backGroundColor = 0;

        pContext->deviceParams[i].resolution  = VSYS_STD_1080P_60;

        width  = 1920;
        height = 1080;

        if(i == VDIS_DEV_SD)
        {
            pContext->deviceParams[i].resolution  = VSYS_STD_NTSC;

            width  = 720;
            height = 480;

        }

        pContext->mosaicParams[i].displayWindow.start_X = 0;
        pContext->mosaicParams[i].displayWindow.start_Y = 0;
        pContext->mosaicParams[i].displayWindow.width   = width;
        pContext->mosaicParams[i].displayWindow.height  = height;

        pContext->mosaicParams[i].numberOfWindows       = 1;
        pContext->mosaicParams[i].outputFPS             = 30;

        for(j=0;j<pContext->mosaicParams[i].numberOfWindows;j++)
        {
            pContext->mosaicParams[i].winList[j].start_X    = 0;
            pContext->mosaicParams[i].winList[j].start_Y    = 0;
            pContext->mosaicParams[i].winList[j].width      = width;
            pContext->mosaicParams[i].winList[j].height     = height;

            pContext->mosaicParams[i].chnMap[j]             = j;
        }
        pContext->mosaicParams[i].userSetDefaultSWMLayout = TRUE;
    }

    /* Configure output Info for vencs */

    pContext->tiedDevicesMask = VDIS_VENC_HDMI | VDIS_VENC_DVO2;
    pContext->enableConfigExtVideoEncoder = FALSE;
    pContext->enableLayoutGridDraw = FALSE;

   /* Setting SD resolution for SD VENC */
   //prm.deviceParams[SYSTEM_DC_VENC_SD].resolution = VSYS_STD_NTSC;
   prm.deviceParams[VDIS_DEV_DVO2].enable = FALSE;
   prm.deviceParams[VDIS_DEV_SD].enable = FALSE;
   prm.deviceParams[VDIS_DEV_HDCOMP].enable = FALSE;
   prm.deviceParams[VDIS_DEV_HDMI].enable = TRUE;
   prm.deviceParams[VDIS_DEV_HDMI].outputInfo.vencNodeNum = VDIS_VENC_HDMI;
   prm.deviceParams[VDIS_DEV_HDMI].outputInfo.aFmt = VDIS_A_OUTPUT_COMPOSITE;
   prm.deviceParams[VDIS_DEV_HDMI].outputInfo.dvoFmt = VDIS_DVOFMT_TRIPLECHAN_EMBSYNC;
   prm.deviceParams[VDIS_DEV_HDMI].outputInfo.dataFormat = SYSTEM_DF_RGB24_888;
   prm.deviceParams[SYSTEM_DC_VENC_HDMI].colorSpaceMode = VDIS_CSC_MODE_SDTV_GRAPHICS_Y2R;
   
    status = System_linkControl(
        SYSTEM_LINK_ID_M3VPSS,
        SYSTEM_M3VPSS_CMD_GET_DISPLAYCTRL_INIT,
        &prm,
        sizeof(prm),
        TRUE
        );
    UTILS_assert(status==OSA_SOK);

    return status;
}

Int32 Chains_displayCtrlDeInit()
{
    Int32 status;

    status = System_linkControl(
        SYSTEM_LINK_ID_M3VPSS,
        SYSTEM_M3VPSS_CMD_GET_DISPLAYCTRL_DEINIT,
        NULL,
        0,
        TRUE
        );
    UTILS_assert(status==OSA_SOK);

    return status;
}

Int32 Chains_grpxEnable(UInt32 grpxId, Bool enable)
{
    return 0;
}

Int32 Chains_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad)
{
    UInt32 numProcs, procId;

    numProcs = sizeof(gChains_enabledProcs)/sizeof(gChains_enabledProcs[0]);

    for(procId=0; procId<numProcs; procId++)
    {
        if(enable)
        {
            System_linkControl(
                gChains_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START,
                NULL,
                0,
                TRUE
            );
        }
        else
        {
            System_linkControl(
                gChains_enabledProcs[procId],
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
                    gChains_enabledProcs[procId],
                    SYSTEM_COMMON_CMD_PRINT_STATUS,
                    &printStatus,
                    sizeof(printStatus),
                    TRUE
                );
            }
            System_linkControl(
                gChains_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_RESET,
                NULL,
                0,
                TRUE
            );
        }
    }

    return 0;
}

Int32 Chains_memPrintHeapStatus()
{
    UInt32 numProcs, procId;
    SystemCommon_PrintStatus printStatus;

    memset(&printStatus, 0, sizeof(printStatus));
    numProcs = sizeof(gChains_enabledProcs)/sizeof(gChains_enabledProcs[0]);

    printStatus.printHeapStatus = TRUE;

    for(procId=0; procId<numProcs; procId++)
    {
        System_linkControl(
                gChains_enabledProcs[procId],
                SYSTEM_COMMON_CMD_CPU_LOAD_CALC_START,
                &printStatus,
                sizeof(printStatus),
                TRUE
            );
    }

    return 0;
}

char gChains_runTimeMenu[] = {
    "\r\n ===================="
    "\r\n Chains Run-time Menu"
    "\r\n ===================="
    "\r\n"
    "\r\n 0: Stop Chain"
    "\r\n"
    "\r\n s: Switch Display Layout  (Sequential change of layout)"
    "\r\n c: Switch Display Channel (Sequential: Increments by 1)"
    "\r\n p: Print Capture Statistics "
    "\r\n b: Modify Encoder Bit Rate "
    "\r\n f: Modify Encoder Frame Rate "
    "\r\n r: Modify Intra Frame Interval(GOP) "
    "\r\n t: Toggle force IDR frame "
    "\r\n g: Print Encoder Dynamic Parameters"
    "\r\n i: Print IVA-HD Statistics "
    "\r\n m: Print SwMs Statistics "
#ifdef  SYSTEM_ENABLE_AUDIO
     "\r\n a: Audio Capture / Playback"
#endif
    "\r\n "
    "\r\n Enter Choice: "
};

char Chains_menuRunTime()
{
    char ch[MAX_INPUT_STR_SIZE];
    printf(gChains_runTimeMenu);
    fgets(ch, MAX_INPUT_STR_SIZE, stdin);
    if(ch[1] != '\n' || ch[0] == '\n')
    ch[0] = '\n';
    return ch[0];
}

#ifdef  SYSTEM_ENABLE_AUDIO
char gChains_runTimeAudioMenu[] = {
    "\r\n ===================="
    "\r\n Audio Run-time Menu"
    "\r\n ===================="
    "\r\n"
    "\r\n 0: Exit Audio Menu"
    "\r\n"
    "\r\n f: Setup Audio Storage Path <set this first>"
    "\r\n s: Start Audio Capture"
    "\r\n t: Stop Audio Capture"
    "\r\n p: Start Audio Playback"
    "\r\n b: Stop Audio Playback"
    "\r\n d: Audio Statistics"
    "\r\n "
    "\r\n Enter Choice: "
} ;

char Chains_audioMenuRunTime()
{
    char buffer[MAX_INPUT_STR_SIZE];

    printf(gChains_runTimeAudioMenu);
    fgets(buffer, MAX_INPUT_STR_SIZE, stdin);
    if (buffer[0] == '\n')
    {
        fgets(buffer, sizeof(buffer), stdin);
    }
    buffer[1] = '\0';
    return buffer[0];
}

unsigned int Chains_AudioStorageInputRunTime(char *path)
{
    printf ("\r\n Provide Audio storage path <e.g. %s or %s> : ", "/audio", "/media/sda2/audio");
    fgets(path, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(path, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(path, MAX_INPUT_STR_SIZE, stdin);
    }
    return 0;
}

unsigned int Chains_AudioCaptureInputRunTime(Bool captureActiveFlag, Int8 maxAudChannels)
{
    UInt32 value;
    char inputStr[MAX_INPUT_STR_SIZE];

    if (captureActiveFlag == TRUE)
    {
        printf ("Audio capture already active... stopping current capture...\n");
    }

    printf("\r\n For which channel would you like to capture audio? Enter (1-%d): ", maxAudChannels);
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(inputStr, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    }

    value = atoi(inputStr);

    if(!(value>=1 && value<=maxAudChannels))
    {
        printf("\r\n Wrong Channel number entered ? Defaulting to 1");
        value = 1;
    }
   return value;
}

unsigned int Chains_AudioPlaybackInputRunTime(Bool playbackActiveFlag, Int8 maxAudChannels)
{
    UInt32 value;
    char inputStr[MAX_INPUT_STR_SIZE];

    if (playbackActiveFlag == TRUE)
    {
        printf ("Audio playback already active... stopping current playback...\n");
    }

    printf("\r\n Which channel audio would you like to play? Enter (1-%d): ", maxAudChannels);
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(inputStr, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    }

    value = atoi(inputStr);

    if(!(value>=1 && value<=maxAudChannels))
    {
        printf("\r\n Wrong Channel number entered ? Defaulting to 1");
        value = 1;
    }
   return value;
}

#endif

unsigned int Chains_ChanInputRunTime()
{
    UInt32 value = 128;
    char inputStr[MAX_INPUT_STR_SIZE];

        printf("\r\n For which channel would you like to apply this change? Enter (0-15): ");
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
        if (strcmp(inputStr, "\n") == 0) {
                /** printf("\r\n New Line read"); **/
                fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
        }

    if (strcmp(inputStr, "\n") != 0)
        value = atoi(inputStr);

        if(!(value>=0 && value<=15))
        {
                printf("\r\n Wrong Channel number entered. ");
                printf("Setting/Query will not be successfull");
                value = 128;
        }

    return value;
}

unsigned int Chains_BitRateInputRunTime()
{
    UInt32 value;
    char inputStr[MAX_INPUT_STR_SIZE];

    printf("\r\n Required bitrate? Enter(Range 64-4000 kbps): ");

    fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(inputStr, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    }

    value = atoi(inputStr);

    if(!(value>=64 && value<=4000))
    {
        printf("\r\n Erroreous Bitrate Value Entered, Not doing anything ");
        value = 0;
    }

    return (value  * 1000);
}

unsigned int Chains_FrameRateInputRunTime()
{
    UInt32 value;
    char inputStr[MAX_INPUT_STR_SIZE];

    printf("\r\n Required framerate? Enter(Choose 8,15,26,30): ");

    fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(inputStr, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    }

    value = atoi(inputStr);

    if(!(value==8 || value==15 || value==26 || value==30 ))
    {
        printf("\r\n Erroreous FPS Value Chosen, Not doing anything ");
        value = 0;
    }

    return value;
}

unsigned int Chains_IntraFrRateInputRunTime()
{
    UInt32 value;
    char inputStr[MAX_INPUT_STR_SIZE];

    printf("\r\n Required Intra-framerate(GOP) interval? Enter[Range 1 - 30]: ");

    fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    if (strcmp(inputStr, "\n") == 0) {
        /** printf("\r\n New Line read"); **/
        fgets(inputStr, MAX_INPUT_STR_SIZE, stdin);
    }

    value = atoi(inputStr);

    if(!(value>=1 && value<=30))
    {
        printf("\r\n Erroreous GOP Value Entered, Not doing anything ");
        value = 0;
    }

    return value;
}
