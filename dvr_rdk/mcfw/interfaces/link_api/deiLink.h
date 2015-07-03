/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup DEI_LINK_API De-interlacer (DEI) Link API

    Dei Link can be used to take input from a link and after doing DEI output
    the frames to output queue.

    DEI can have upto two outputs
    - Output from DEI scaler
    - Output from VIP scaler

    Each can be individually enabled/disabled during link create.

    Each frame output is put in its corresponding output queue.

    @{
*/

/**
    \file deiLink.h
    \brief De-interlacer (DEI) Link API
*/

#ifndef _DEI_LINK_H_
#define _DEI_LINK_H_

#include <mcfw/interfaces/link_api/system.h>


/** \brief DEI Scaler output ID */
#define DEI_LINK_OUT_QUE_DEI_SC  (0)

/** \brief VIP Scaler output ID */
#define DEI_LINK_OUT_QUE_VIP_SC  (1)

/** \brief DEI Scaler output ID */
#define DEI_LINK_OUT_QUE_VIP_SC_SECONDARY_OUT  (2)

/** \brief Max DEI outputs/output queues */
#define DEI_LINK_MAX_OUT_QUE     (3)

/** \brief Max DEI outputs/output queues */
#define DEI_LINK_MAX_DRIVER_OUT_QUE     (2)

/** \brief Max No. Of channels */
#if defined(TI_814X_BUILD) || defined(TI_8107_BUILD)
#define DEI_LINK_MAX_CH                      (16)
#else
#define DEI_LINK_MAX_CH                      (8)
#endif
/** \Maximum number of video planes (Y/U/V or R/G/B) */
#define DEI_LINK_MAX_VIDFRAME_PLANES    (3u)

/** \DEI command to get processed data */
#define DEI_LINK_CMD_GET_PROCESSED_DATA      (0x3000)
/** \DEI command to enable channel */
#define DEI_LINK_CMD_ENABLE_CHANNEL          (0x3001)
/** \DEI command to enable channel */
#define DEI_LINK_CMD_DISABLE_CHANNEL         (0x3002)
/** \DEI command to set Frame Rate */
#define DEI_LINK_CMD_SET_FRAME_RATE          (0x3003)
/** \DEI command to print stats */
#define DEI_LINK_CMD_PRINT_STATISTICS        (0x3004)
/** \DEI command to set resolution */
#define DEI_LINK_CMD_SET_OUTPUTRESOLUTION    (0x3005)
/** \DEI command to get resolution */
#define DEI_LINK_CMD_GET_OUTPUTRESOLUTION    (0x3006)
/** \DEI command to print buffer status */
#define DEI_LINK_CMD_PRINT_BUFFER_STATISTICS (0x3007)
/** DEI command to flush previous input fields from DEI */
#define DEI_LINK_CMD_FLUSH_CHANNEL_INPUT     (0x3008)

/**
    \brief Dei link scale mode
*/
typedef enum
{
	/**< Absolute scale mode, provide width & height */
    DEI_SCALE_MODE_ABSOLUTE,

	/**< Scale based on ratio */
    DEI_SCALE_MODE_RATIO
} DEI_SCALE_MODE;

/**
    \brief Dei link scale ratio parameters
*/
typedef struct
{
	/**< numerator of the ratio */
	UInt32	numerator;
	/**< denominiator of the ratio, for a scaling of 1/2, numerator will be 1, denominator will be 2 */
	UInt32	denominator;
} DeiLink_OutputScaleRatio;

/**
    \brief Dei link set ouput resolution
*/
typedef struct
{
    /**< output queue Id */
    UInt32 queId;
    /**< output channel Id */
    UInt32 chId;
    /**< ouput width */
    UInt32 width;
    /**< ouput height */
    UInt32 height;
    /**< ouput pitch */
    UInt32 pitch[DEI_LINK_MAX_VIDFRAME_PLANES];
} DeiLink_chDynamicSetOutRes;

/**
    \brief Dei link scale factor
*/
typedef struct
{
    UInt32                     scaleMode;

    /**< Scale factor for outputs */
    union
    {
        struct
        {
        	/**< Required scaled output width */
            UInt32 outWidth;
            /**< Required scaled output height */
            UInt32 outHeight;
        } absoluteResolution;
        struct
        {
            /**< Required scaled output ratio, output width is relative to input width */
            DeiLink_OutputScaleRatio    widthRatio;
            /**< Required scaled output ratio, output height is relative to input height */
            DeiLink_OutputScaleRatio    heightRatio;
        } ratio;
    };
} DeiLink_OutputScaleFactor;

/**
    \brief Dei link create parameters
*/
typedef struct
{
    /**< System configuration ID. */
    System_LinkInQueParams     inQueParams;
    /**< Input queue information. */
    System_LinkOutQueParams    outQueParams[DEI_LINK_MAX_OUT_QUE];
    /**< Output queue information. */
    UInt32                     enableOut[DEI_LINK_MAX_OUT_QUE];
    /**< enableOut[x] = TRUE, enable the corresponding output
         enableOut[x] = FALSE, disable the corresponding output. */

    UInt32                     tilerEnable;
    UInt32                     comprEnable;
    UInt32                     setVipScYuv422Format;
    /**< Scale mode for outputs, see DEI_SCALE_MODE for valid values */
    DeiLink_OutputScaleFactor  outScaleFactor[DEI_LINK_MAX_OUT_QUE][DEI_LINK_MAX_CH];
    UInt32                     enableDeiForceBypass;
    UInt32                     enableLineSkipSc;
    /** < applicable for progressive input OR when enableDeiForceBypass = TRUE */
    /** < inputFrameRate + outputFrameRate should  set to control required fps */
    UInt32                     inputFrameRate[DEI_LINK_MAX_OUT_QUE];
    UInt32                     outputFrameRate[DEI_LINK_MAX_OUT_QUE];

    /** < applicable for de-interlace module of DEI link */
    /** < the inputDeiFrameRate + outputDeiFrameRate will do fps control first */
    /** < the output of de-interlace module will go to scalar module of DEI link */
    /** < then, the inputFrameRate + outputFrameRate above will be used to fps control */
    UInt32                     inputDeiFrameRate;
    UInt32                     outputDeiFrameRate;

    UInt32                     numBufsPerCh[DEI_LINK_MAX_OUT_QUE];

    Bool                       generateBlankOut[DEI_LINK_MAX_OUT_QUE];
} DeiLink_CreateParams;

/**
    \brief Dei link channel info
*/
typedef struct
{
	UInt32 channelId;
	UInt32 streamId;
	UInt32 enable;
} DeiLink_ChannelInfo;

/**
    \brief Dei link channel dynamic set config params

    Defines Dei FPS parameters that can be changed dynamically
    on a per channel basis
*/
typedef struct DeiLink_ChFpsParams
{
    UInt32 chId;
    /**< Dei channel number */
	UInt32 streamId;
    /**< Dei steamId, maps to Queue No */
    UInt32 inputFrameRate;
    /**< input frame rate - 60 or 50 fps if interlaced */
    UInt32 outputFrameRate;
    /**< Expected output frame rate */
} DeiLink_ChFpsParams;

/**
    \brief Dei link channel flush params

    Flush or release previous input fields.
    Only applicable when DEI is ON.

    Only applicable for TI814x and TI810x
*/
typedef struct DeiLink_ChFlushParams
{
    UInt32 chId;
    /**< Dei channel number */

} DeiLink_ChFlushParams;

/**
    \brief Dei link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 DeiLink_init();

/**
    \brief Dei link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 DeiLink_deInit();

static inline void DeiLink_CreateParams_Init(DeiLink_CreateParams *pPrm)
{
    UInt32 outId, chId;

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;

    for(outId=0; outId<DEI_LINK_MAX_OUT_QUE; outId++)
    {
#if defined(TI_814X_BUILD) || defined(TI_8107_BUILD)
        pPrm->numBufsPerCh[outId] = 4;
#else
        pPrm->numBufsPerCh[outId] = 8;
#endif
        pPrm->outQueParams[outId].nextLink = SYSTEM_LINK_ID_INVALID;
        pPrm->enableOut[outId] = FALSE;
        for(chId=0; chId<DEI_LINK_MAX_CH; chId++)
        {
            pPrm->outScaleFactor[outId][chId].scaleMode = DEI_SCALE_MODE_RATIO;
            pPrm->outScaleFactor[outId][chId].ratio.widthRatio.numerator = 1;
            pPrm->outScaleFactor[outId][chId].ratio.widthRatio.denominator = 1;
            pPrm->outScaleFactor[outId][chId].ratio.heightRatio.numerator = 1;
            pPrm->outScaleFactor[outId][chId].ratio.heightRatio.denominator = 1;
        }
        pPrm->generateBlankOut[outId]   = FALSE;
    }

    pPrm->tilerEnable = FALSE;
    pPrm->comprEnable = FALSE;

    pPrm->setVipScYuv422Format = FALSE;
    pPrm->inputFrameRate[DEI_LINK_OUT_QUE_DEI_SC] = 60;
    pPrm->outputFrameRate[DEI_LINK_OUT_QUE_DEI_SC] = 30;
    pPrm->inputFrameRate[DEI_LINK_OUT_QUE_VIP_SC] = 30;
    pPrm->outputFrameRate[DEI_LINK_OUT_QUE_VIP_SC] = 30;
    pPrm->inputFrameRate[DEI_LINK_OUT_QUE_VIP_SC_SECONDARY_OUT] = 30;
    pPrm->outputFrameRate[DEI_LINK_OUT_QUE_VIP_SC_SECONDARY_OUT] = 30;

    pPrm->enableDeiForceBypass = FALSE;
    pPrm->enableLineSkipSc = FALSE;

    pPrm->inputDeiFrameRate  = 30;
    pPrm->outputDeiFrameRate = 30;
}

#endif

/*@}*/

