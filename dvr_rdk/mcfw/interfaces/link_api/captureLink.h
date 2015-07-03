/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup CAPTURE_LINK_API Capture Link API

    Capture Link can be used to instantiate capture upto
    SYSTEM_CAPTURE_INST_MAX Video input port instances.

    Each instance can have upto two outputs.

    The frames from these capture outputs can be put in upto
    four output queues.

    Each output queue can inturn to be connected to a link like
    Display or DEI or NSF.

    @{
*/

/**
    \file captureLink.h
    \brief Capture Link API
*/

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <mcfw/interfaces/link_api/system.h>

/** \brief Max outputs per VIP instance */
#define CAPTURE_LINK_MAX_OUTPUT_PER_INST  (2)

/** \brief Max output queues in the capture link */
#define CAPTURE_LINK_MAX_OUT_QUE          (4)

/** \brief Max Channels per output queue */
#define CAPTURE_LINK_MAX_CH_PER_OUT_QUE   (16)

/** \brief Indicates number of output buffers to be set to default
 *         value by the capture link
 */
#define CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT (0)

/** \brief Max blind area supported for each channel */
#define CAPTURE_LINK_MAX_BLIND_AREA_PER_CHNL (16)


/**
    \brief Link CMD: Detect capture video source format

    This command can make the capture link wait until video
    source is detect on all the expected input video sources

    \param UInt32 timeout  [IN] BIOS_WAIT_FOREVER or BIOS_NO_WAIT
*/
#define CAPTURE_LINK_CMD_DETECT_VIDEO            (0x1000)
#define CAPTURE_LINK_CMD_FORCE_RESET             (0x1001)
#define CAPTURE_LINK_CMD_PRINT_ADV_STATISTICS    (0x1002)
#define CAPTURE_LINK_CMD_CHANGE_BRIGHTNESS       (0x1003)
#define CAPTURE_LINK_CMD_CHANGE_CONTRAST         (0x1004)
#define CAPTURE_LINK_CMD_CHANGE_SATURATION       (0x1005)
#define CAPTURE_LINK_CMD_CHANGE_HUE              (0x1006)
#define CAPTURE_LINK_CMD_HALT_EXECUTION          (0x1007)
#define CAPTURE_LINK_CMD_CONFIGURE_VIP_DECODERS  (0x1008)
#define CAPTURE_LINK_CMD_GET_VIDEO_STATUS        (0x1009)
#define CAPTURE_LINK_CMD_SET_AUDIO_CODEC_PARAMS  (0x100A)
#define CAPTURE_LINK_CMD_PRINT_BUFFER_STATISTICS (0x100B)
#define CAPTURE_LINK_CMD_SET_EXTRA_FRAMES_CH_ID  (0x100C)
#define CAPTURE_LINK_CMD_CONFIGURE_BLIND_AREA    (0x100D)

/*
    Only applicable for interlaced input source
*/
#define CAPTURE_LINK_CMD_SKIP_ALTERNATE_FRAMES   (0x100E)

/**
    \brief Parameters used to specify or change at run-time
        the channel for which extra buffers are needed

    This is applicable only when CaptureLink_CreateParams.numExtraBuf > 0
*/
typedef struct
{
    UInt32 queId;
    /**< Queue for which the extra channel buffers are needed */

    UInt32 chId;
    /**< Channel in the queue for whom extra channel buffers are needed */

} CaptureLink_ExtraFramesChId;

/**
    \brief Capture Color Specific Parameters
*/

typedef struct
{
    UInt32 brightness;
    /*< Brightness: 0-255 */

    UInt32 contrast;
    /*< contrast: 0-255 */

    UInt32 satauration;
    /*< satauration: 0-255 */

    UInt32 hue;
    /*< hue: 0-255 */

    UInt32 chId;
    /*< chId: 0-15 */

}CaptureLink_ColorParams;


/**
    \brief Capture output parameters
*/
typedef struct
{
    UInt32              dataFormat;
    /**< output data format, YUV422, YUV420, RGB, see System_VideoDataFormat */

    UInt32              scEnable;
    /**< TRUE: enable scaling, FALSE: disable scaling */

    UInt32              scOutWidth;
    /**< Scaler output width */

    UInt32              scOutHeight;
    /**< Scaler output height */

    UInt32              outQueId;
    /**< Link output que ID to which this VIP instance output frames are put */

} CaptureLink_OutParams;

/**
    \brief VIP instance information
*/
typedef struct
{
    UInt32                        vipInstId;
    /**< VIP capture driver instance ID, see SYSTEM_CAPTURE_INST_VIPx_PORTy */

    UInt32                        videoDecoderId;
    /**< Video decoder instance ID, see SYSTEM_DEVICE_VID_DEC_xxx_DRV */

    UInt32                        inDataFormat;
    /**< Input source data format, RGB or YUV422, see System_VideoDataFormat */

    UInt32                        standard;
    /**< Required video standard, see System_VideoStandard */

    UInt32                        numOutput;
    /**< Number of outputs per VIP */

    CaptureLink_OutParams         outParams[CAPTURE_LINK_MAX_OUTPUT_PER_INST];
    /**< Information about each output */

    UInt32                        numChPerOutput;
    /**< Number of individual channels per outputs */

} CaptureLink_VipInstParams;

typedef struct
{
    UInt32 startX;        /* in pixels, MUST be multiple of 2 of YUV422I and multiple of 4 for YUV420SP */
    UInt32 startY;        /* in lines, MUST be multiple of 2 for YUV420SP  */
    UInt32 width;         /* in pixels, MUST be multiple of 2 of YUV422I and multiple of 4 for YUV420SP */
    UInt32 height;        /* in lines, MUST be multiple of 2 for YUV420SP  */
    UInt32 fillColorYUYV; /* Color in YUYV format .*/
    UInt32 enableWin;     /* TRUE: Draw this blind area, FALSE: Do not draw this area */
} CaptureLink_BlindWin;

typedef struct
{
    UInt32                  channelId;
    UInt32                  queId;

    UInt32                  numBlindArea;
    /**< number of valid blind area, 0 means disable bland area of this channel */

    CaptureLink_BlindWin    win[CAPTURE_LINK_MAX_BLIND_AREA_PER_CHNL];

} CaptureLink_BlindInfo;


/**
    \brief Params for CAPTURE_LINK_CMD_SKIP_ALTERNATE_FRAMES
*/
typedef struct
{
    UInt32 queId;   /**< Output Que ID */
    UInt32 skipAlternateFramesChBitMask;
    /**< Channels for which odd fields should be skipped

        bit0 = CH0
        bit1 = CH1
        ...
        bitN = CHN
    */

} CaptureLink_SkipAlternateFrames;

/**
    \brief Capture Link create parameters
*/
typedef struct
{
    Bool                      isPalMode;
    /**< pal mode based on usecase */

    UInt16                    numVipInst;
    /**< Number of VIP instances in this link */

    CaptureLink_VipInstParams vipInst[SYSTEM_CAPTURE_INST_MAX];
    /**< VIP instance information */

    System_LinkOutQueParams   outQueParams[CAPTURE_LINK_MAX_OUT_QUE];
    /**< Output queue information */

    UInt32                    tilerEnable;
    /**< TRUE/FALSE: Enable/disable tiler */


    UInt32                    fakeHdMode;
    /**< Capture in D1 but tells link size is 1080p, useful to test HD data flow on DVR HW or VS EVM */

    UInt32                    enableSdCrop;
    /**< Applicable only for D1/CIF capture input, crops 720 to 704 and 360 to 352 at video decoder.
        */
    UInt32                    doCropInCapture;
    /**< Applicable only for D1/CIF capture input, crops 720 to 704 and 360 to 352 at video decoder.
         Input is still 720/360. Crop is done in capture link by adjusting capture frame buffer pointer
         */
    UInt32                    numBufsPerCh;
    /**< Number of output buffers per channel in capture */

    UInt32                    numExtraBufs;
    /**<
        Number of extra buffers allocated and used by capture across channels.
        Buffers are allocated using WxH of CH0

        This is useful in some cases where the next link may hold the buffers
        of a given channel for a longer duration.

        In such cases the extra buffers will used for capture. This will ensure
        capture does not drop frames due to next link holding onto the
        buffer for a longer time

        Example, when capture is connect to SD display link directly

        This should be used typically when all CHs have the same properties like WxH
    */

    UInt32                    maxBlindAreasPerCh;

} CaptureLink_CreateParams;

/**
    \brief Audio Codec parameters
*/
typedef struct
{
    UInt32 deviceNum;
  /**< Device number for which to apply the audio parameters */

    UInt32 numAudioChannels;
  /**< Number of audio channels */
    UInt32 samplingHz;
  /**< Audio sampling rate in Hz, Valid values: 8000, 16000 */

    UInt32 masterModeEnable;
  /**< TRUE: Master mode of operation, FALSE: Slave mode of operation */

    UInt32 dspModeEnable;
  /**< TRUE: DSP data format mode, FALSE: I2S data format mode */

    UInt32 ulawEnable;
  /**< TRUE: 8-bit ulaw data format mode, FALSE: 16-bit PCM data format mode */

    UInt32 cascadeStage;
  /**< Cascade stage number, Valid values: 0..3 */

    UInt32 audioVolume;
  /**< Audio volume, Valid values: 0..8. Refer to TVP5158 datasheet for details
   */

    UInt32 tdmChannelNum;
    /**< Number of TDM channels: 0: 2CH, 1: 4CH, 2: 8CH, 3: 12CH 4: 16CH */

} Capture_AudioModeParams;

/**
    \brief Capture link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 CaptureLink_init();

/**
    \brief Capture link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 CaptureLink_deInit();


static inline void CaptureLink_CreateParams_Init(CaptureLink_CreateParams *pPrm)
{
    UInt32 i;
    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->numVipInst = 0;
    pPrm->tilerEnable = FALSE;
    pPrm->fakeHdMode = FALSE;
    pPrm->enableSdCrop = TRUE;
    pPrm->doCropInCapture = TRUE;
    pPrm->numBufsPerCh = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT;
    pPrm->numExtraBufs = 0;
    
    for (i=0; i<SYSTEM_CAPTURE_INST_MAX; i++)
    {
        pPrm->vipInst[i].numChPerOutput = 0;
    }
}

#endif

/*@}*/
