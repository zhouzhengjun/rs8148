/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _CHAINS_H_
#define _CHAINS_H_


#include <osa.h>

#include <link_api/system.h>
#include <link_api/captureLink.h>
#include <link_api/deiLink.h>
#include <link_api/nsfLink.h>
#include <link_api/algLink.h>
#include <link_api/displayLink.h>
#include <link_api/nullLink.h>
#include <link_api/grpxLink.h>
#include <link_api/dupLink.h>
#include <link_api/swMsLink.h>
#include <link_api/mergeLink.h>
#include <link_api/nullSrcLink.h>
#include <link_api/ipcLink.h>
#include <link_api/systemLink_m3vpss.h>
#include <link_api/systemLink_m3video.h>
#include <link_api/encLink.h>
#include <link_api/decLink.h>


#include <avcap/common/chains_swMs.h>
#include <avcap/common/chains_ipcBits.h>

#include <rsdemo/graphic/graphic.h>

#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/ivideo.h>
#include <ih264enc.h>
#include <ih264vdec.h>
#include <device.h>
#include <device_videoDecoder.h>
#include <mcfw/src_linux/devices/sii9135/src/sii9135_priv.h>

#include <sii9135.h>

#define CHAINS_OSD_NUM_WINDOWS      (1)
#define CHAINS_OSD_WIN_MAX_WIDTH    (320)
#define CHAINS_OSD_WIN_MAX_HEIGHT   (64)
#define CHAINS_OSD_WIN0_STARTX      (16)
#define CHAINS_OSD_WIN0_STARTY      (32)
#define CHAINS_OSD_WIN1_STARTX      (500)
#define CHAINS_OSD_WIN1_STARTY      (150)
#define CHAINS_OSD_WIN_WIDTH       (160)
#define CHAINS_OSD_WIN_HEIGHT      (32)
#define CHAINS_OSD_WIN_PITCH       (CHAINS_OSD_WIN_WIDTH)
#define CHAINS_OSD_TRANSPARENCY    (1)
#define CHAINS_OSD_GLOBAL_ALPHA    (0x80)
#define CHAINS_OSD_ENABLE_WIN      (1)
#define CHAINS_OSD_MAX_FILE_NAME_SIZE (128)
#define CHAINS_OSD_WIN0_FILE_NAME   "tiLogoYUV422i.jpeg"

#define CHAINS_INIT_STRUCT(structName,structObj)  structName##_Init(&structObj)

typedef struct {

    /* Enable NSF during chain */
    Bool   enableNsfLink;

    Bool   enableOsdAlgLink;

    Bool   enableVidFrameExport;
    /* NSF mode when NSF Link is in chain

        TRUE: NSF is bypass and is in CHR DS mode
        FALSE: NSF is bypass and is in CHR DS mode
    */
    Bool bypassNsf;
	
	UInt32  captureId ;
    UInt32  swMsId;
    UInt32  displayId;
	UInt32  dupId;
	UInt32  m3out;
	UInt32  a8in;

    UInt32 displayRes[SYSTEM_DC_MAX_VENC];

} Chains_Ctrl;

typedef Void (*Chains_RunFunc)();

Chains_Ctrl * Chains_Create();
Void Chains_Delete(Chains_Ctrl *);
Void Chains_Start(Chains_Ctrl *);
Void Chains_Stop(Chains_Ctrl *);

Void Chains_main();

Void Chains_multiChCaptureNsfDei(Chains_Ctrl *chainsCfg);
Void Chains_multiChSystemUseCaseSwMsTriDisplay(Chains_Ctrl *chainsCfg);
Void Chains_multiChSystemUseCaseSwMsTriDisplay2(Chains_Ctrl *chainsCfg);
Void Chains_tvp5158NonMuxCapture(Chains_Ctrl *chainsCfg);

Void Chains_multiChCaptureDeiIpcOutIn(Chains_Ctrl *chainsCfg);
Void Chains_multiChEncDecLoopBack(Chains_Ctrl *chainsCfg);
Void chains_multiChDucatiSystemUseCaseSwMsTriDisplay1(Chains_Ctrl *chainsCfg);
Void chains_multiChDucatiSystemUseCaseSwMsTriDisplay2(Chains_Ctrl *chainsCfg);

Void Chains_menuSettings();
Void Chains_menuCurrentSettingsShow();
char Chains_menuRunTime();

unsigned int Chains_ChanInputRunTime();
unsigned int Chains_BitRateInputRunTime();
unsigned int Chains_FrameRateInputRunTime();
unsigned int Chains_IntraFrRateInputRunTime();
#ifdef	SYSTEM_ENABLE_AUDIO
char Chains_audioMenuRunTime(void);
unsigned int Chains_AudioStorageInputRunTime(char *path);
unsigned int Chains_AudioCaptureInputRunTime(Bool captureActiveFlag, Int8 maxAudChannels);
unsigned int Chains_AudioPlaybackInputRunTime(Bool playbackActiveFlag, Int8 maxAudChannels);
#endif


Int32 Chains_detectBoard();

Int32 Chains_displayCtrlInit(UInt32 displayRes[]);
Int32 Chains_displayCtrlDeInit();
Int32 Chains_grpxEnable(UInt32 grpxId, Bool enable);
Int32 Chains_prfLoadCalcEnable(Bool enable, Bool printStatus, Bool printTskLoad);
Int32 Chains_memPrintHeapStatus();

#endif

