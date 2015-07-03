/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/**
    \ingroup LINK_API
    \defgroup SCLR_LINK_API Scalar 5 Link API

    SCLR Link can be used to take input from a link and after Scaling output
    the frames to output queue.

    SCLR has one output
    - Output from SC5 scaler


    @{
*/

/**
    \file sclrLink.h
    \brief Scalar (SCLR) Link API
*/

#ifndef _SCLR_LINK_H_
#define _SCLR_LINK_H_

#include <mcfw/interfaces/link_api/system.h>
#include <mcfw/interfaces/link_api/vidframe.h>

/** \brief SCLR Scaler output ID */
#define SCLR_LINK_MAX_OUT_QUE  (1)

/** \SCLR command to get processed data */
#define SCLR_LINK_CMD_GET_PROCESSED_DATA      (0x7000)
/** \SCLR command to enable channel */
#define SCLR_LINK_CMD_ENABLE_CHANNEL          (0x7001)
/** \SCLR command to enable channel */
#define SCLR_LINK_CMD_DISABLE_CHANNEL         (0x7002)
/** \SCLR command to set Frame Rate */
#define SCLR_LINK_CMD_SET_FRAME_RATE          (0x7003)
/** \SCLR command to print stats */
#define SCLR_LINK_CMD_PRINT_STATISTICS        (0x7004)
/** \SCLR command to set resolution */
#define SCLR_LINK_CMD_SET_OUTPUTRESOLUTION    (0x7005)
/** \SCLR command to get resolution */
#define SCLR_LINK_CMD_GET_OUTPUTRESOLUTION    (0x7006)
/** \SCLR command to skip one specific FID type */
#define SCLR_LINK_CMD_SKIP_FID_TYPE           (0x7007)

#define SCLR_LINK_SC5               0
#define SCLR_LINK_SEC0_SC3          1

#define SCLR_LINK_NUM_BUFS_PER_CH_DEFAULT (0)

/**
    \brief SCLR link scale mode
*/
typedef enum
{
	/**< Absolute scale mode, provide width & height */
    SCLR_SCALE_MODE_ABSOLUTE,

	/**< Scale based on ratio */
    SCLR_SCALE_MODE_RATIO
} SCLR_SCALE_MODE;

/**
    \brief SCLR link scale ratio parameters
*/
typedef struct
{
	/**< numerator of the ratio */
	UInt32	numerator;
	/**< denominiator of the ratio, for a scaling of 1/2, numerator will be 1, denominator will be 2 */
	UInt32	denominator;
} SclrLink_OutputScaleRatio;

/**
    \brief SCLR link set ouput resolution
*/
typedef struct
{
    /**< output channel Id */
    UInt32 chId;
    /**< ouput width */
    UInt32 width;
    /**< ouput height */
    UInt32 height;
    /**< ouput pitch */
    UInt32 pitch[VIDFRAME_MAX_PLANES];
} SclrLink_chDynamicSetOutRes;

/**
    \brief SCLR link to skip one specific FID type
*/
typedef struct
{
    /**< output channel Id */
    UInt32 chId;
    /**< FID type, TOP or BOTTOM */
    UInt32 fidType;
} SclrLink_chDynamicSkipFidType;

/**
    \brief SCLR link scale factor 
*/
typedef union
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
        SclrLink_OutputScaleRatio    widthRatio;    
        /**< Required scaled output ratio, output height is relative to input height */
        SclrLink_OutputScaleRatio    heightRatio;    
    } ratio;
} SclrLink_OutputScaleFactor;

/**
    \brief Sclr link create parameters
*/
typedef struct
{
    /**< System configuration ID. */
    System_LinkInQueParams      inQueParams;
    /**< Input queue information. */
    System_LinkOutQueParams     outQueParams;
    /**< Output queue information. */

    UInt32                      pathId;
    UInt32                      tilerEnable;
	/**< Scale mode for outputs, see SCLR_SCALE_MODE for valid values */
    UInt32                      scaleMode;

	SclrLink_OutputScaleFactor  outScaleFactor;

    UInt32                     enableLineSkipSc;

    UInt32                      numBufsPerCh;
    /**< Number of output buffers per channel in capture */

    UInt32                     inputFrameRate;
    UInt32                     outputFrameRate;

    UInt32                      outDataFormat;
} SclrLink_CreateParams;

/**
    \brief SCLR link channel info
*/
typedef struct
{
	UInt32 channelId;
	UInt32 enable;
} SclrLink_ChannelInfo;

/**
    \brief SCLR link channel dynamic set config params

    Defines SCLR FPS parameters that can be changed dynamically
    on a per channel basis 
*/
typedef struct SclrLink_ChFpsParams
{
    UInt32 chId;
    /**< Sclr channel number */
    UInt32 inputFrameRate;
    /**< input frame rate - 60 or 50 fps if interlaced */
    UInt32 outputFrameRate;
    /**< Expected output frame rate */
} SclrLink_ChFpsParams;

/**
    \brief SCLR link register and init

    - Creates link task
    - Registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 SclrLink_init();

/**
    \brief Sclr link de-register and de-init

    - Deletes link task
    - De-registers as a link with the system API

    \return FVID2_SOK on success
*/
Int32 SclrLink_deInit();

static inline void SclrLink_CreateParams_Init(SclrLink_CreateParams *pPrm)
{

    memset(pPrm, 0, sizeof(*pPrm));

    pPrm->inQueParams.prevLinkId = SYSTEM_LINK_ID_INVALID;
    pPrm->inQueParams.prevLinkQueId = 0;

    pPrm->outQueParams.nextLink = SYSTEM_LINK_ID_INVALID;
    pPrm->outScaleFactor.ratio.widthRatio.numerator = 1;
    pPrm->outScaleFactor.ratio.widthRatio.denominator = 1;
    pPrm->outScaleFactor.ratio.heightRatio.numerator = 1;
    pPrm->outScaleFactor.ratio.heightRatio.denominator = 1;
    pPrm->inputFrameRate = 30;
    pPrm->outputFrameRate = 30;
    pPrm->tilerEnable = FALSE;
    pPrm->scaleMode = SCLR_SCALE_MODE_RATIO;
    pPrm->enableLineSkipSc = FALSE;

    pPrm->numBufsPerCh  = SCLR_LINK_NUM_BUFS_PER_CH_DEFAULT;
    pPrm->outDataFormat = SYSTEM_DF_YUV422I_YUYV;

    pPrm->pathId = SCLR_LINK_SC5;
}

#endif

/*@}*/

