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


#ifndef _BELL_API_H_
#define _BELL_API_H_

#include "ti_media_common_def.h"
#include "videoServer.h"

typedef enum
{
	BELL_USECASE_PREVIEW,
	
    BELL_USECASE_IPNCDEMO,  //VSYS_USECASE_ENC_A8_DEC

    BELL_USECASE_BELLLITE,

    BELL_USECASE_BELL_BASIC,
    
    BELL_USECASE_BELL_BASIC_HDMI_IN,

	BELL_USECASE_MODE0,
	BELL_USECASE_MODE1,
	BELL_USECASE_MODE2,
	BELL_USECASE_MODE3,
	BELL_USECASE_MODE4,
	BELL_USECASE_MODE5_0,
	BELL_USECASE_MODE5_1,
	BELL_USECASE_MODE6,
	BELL_USECASE_MODE7,
	BELL_USECASE_MODE8,

    BELL_USECASE_MAX
    /**< Maximum use-case ID */

} BELL_USECASES_E;

/**********************************************************************************************************
 *
 *  Following code is import from mcfw/interfaces/ti_venc.h & mcfw/interfaces/ti_vdec.h
 *  Make sure they are up-to-date with MCFW version.
 *
 **********************************************************************************************************/

/** Maximum Video Encode channels */
#define BELL_VENC_PRIMARY_CHANNELS   (16)

#define BELL_VENC_CHN_MAX			(3*BELL_VENC_PRIMARY_CHANNELS)    /* If secondary output is enabled, there will be 16+16+16 channels */

/**
    \brief Encoder Rate-control type
*/
typedef enum BELL_RC_CTRL_E
{
    BELL_VENC_RATE_CTRL_VBR = 0,
    /**< [IH264_RATECONTROL_PRC] Variable Bitrate: For Local Storage*/

    BELL_VENC_RATE_CTRL_CBR = 1,
    /**< [IH264_RATECONTROL_PRC_LOW_DELAY]Constant Bitrate: For Video Conferencing*/

} BELL_VENC_RATE_CTRL_E;

typedef enum {
    BELL_VENC_XDM_DEFAULT = 0,                
    /**< Default setting of encoder.  See
     *   codec specific documentation for its
     *   encoding behaviour.
     */

    BELL_VENC_XDM_HIGH_QUALITY = 1,           
    /**< High quality encoding. */

    BELL_VENC_XDM_HIGH_SPEED = 2,             
    /**< High speed encoding. */

    BELL_VENC_XDM_USER_DEFINED = 3,           
    /**< User defined configuration, using
          *   advanced parameters.
          */

    BELL_VENC_XDM_HIGH_SPEED_MED_QUALITY = 4, 
    /**< High speed, medium quality
          *   encoding.
          */

    BELL_VENC_XDM_MED_SPEED_MED_QUALITY = 5,  
    /**< Medium speed, medium quality
          *   encoding.
                                     */
    BELL_VENC_XDM_MED_SPEED_HIGH_QUALITY = 6, 
    /**< Medium speed, high quality
          *   encoding.
          */

    BELL_VENC_XDM_ENCODING_PRESET_MAX  = 7,   
    /**< @todo need to add documentation */

	BELL_VENC_XDM_PRESET_DEFAULT = BELL_VENC_XDM_MED_SPEED_MED_QUALITY 
    /**< Default setting of
     *   encoder.  See codec specific
     *   documentation for its encoding
     *   behaviour.
     */
} BELL_VENC_XDM_EncodingPreset;

/**
  @enum   BELL_VENC_IH264ENC_SvcExtensionFlag
  @brief  Define SVC Extension Flag
*/
typedef enum
{
 /*Svc Extension Flag Disabled*/
  BELL_VENC_IH264_SVC_EXTENSION_FLAG_DISABLE               = 0 ,
  /*Svc Extension Flag Enabled*/
  BELL_VENC_IH264_SVC_EXTENSION_FLAG_ENABLE                = 1 ,
  /*Svc Extension Flag Enabled with EC Flexibility*/
  BELL_VENC_IH264_SVC_EXTENSION_FLAG_ENABLE_WITH_EC_FLEXIBILITY = 2

} BELL_VENC_IH264ENC_SvcExtensionFlag ;


/**
    \brief Profile Identifier for H.264 Encoder
*/
typedef enum
{
    BELL_VENC_CHN_BASELINE_PROFILE = 66,                     
    /**< BaseLine Profile   */

    BELL_VENC_CHN_MAIN_PROFILE     = 77,                     
    /**< Main Profile       */

    BELL_VENC_CHN_EXTENDED_PROFILE = 88,                     
    /**< Extended Profile   */

    BELL_VENC_CHN_HIGH_PROFILE     = 100,                    
    /**< High Profile       */

    BELL_VENC_CHN_DEFAULT_PROFILE  = BELL_VENC_CHN_HIGH_PROFILE,  
    /**< Default Profile    */

    BELL_VENC_CHN_HIGH10_PROFILE   = 110,                    
    /**< High 10 Profile    */

    BELL_VENC_CHN_HIGH422_PROFILE  = 122,
    /**< High 4:2:2 Profile */
    
    BELL_VENC_CHN_SVC_BASELINE_PROFILE = 83,
    /**< SVC Baseline Profile */

    BELL_VENC_CHN_SVC_HIGH_PROFILE      = 86
    /**< SVC High Profile */

} BELL_VENC_CHN_H264_Profile_E;


/**
    \brief Indicates number of temporal layers in bitstrea, for H.264 Encoder
*/
typedef enum
{
    BELL_VENC_TEMPORAL_LAYERS_1 = 1,
    /**< Only Base Layer */

    BELL_VENC_TEMPORAL_LAYERS_2 = 2,
    /**< Base Layer + Temporal Layer */

    BELL_VENC_TEMPORAL_LAYERS_3 = 3,
    /**< Base Layer + 2 Temporal Layers */

    BELL_VENC_TEMPORAL_LAYERS_4 = 4,
    /**< Base Layer + 3 Temporal Layers */

    BELL_VENC_TEMPORAL_LAYERS_MAX = BELL_VENC_TEMPORAL_LAYERS_4
    /**< Max number of layers supported */


} BELL_VENC_CHN_H264_NumTemporalLayer_E;

typedef enum
{
    BELL_VENC_BITRATE,
    /**< Set Bitrate in bits/second */

    BELL_VENC_FRAMERATE,
    /**< Set frame-rate */

    BELL_VENC_IPRATIO,
    /**< Set Intra-frame interval */

    BELL_VENC_RCALG,
    /**< Set Rate Control Algo */

    BELL_VENC_QPVAL_I,
    /**< Set QP min/max/init for I-frame */

    BELL_VENC_QPVAL_P,
    /**< Set QP min/max/init for I-frame */

    BELL_VENC_IN_FRAMERATE,
    /**< Set Input frame rate */

    BELL_VENC_VBRDURATION,
    /**< Set VBRDuration param of CVBR */

    BELL_VENC_VBRSENSITIVITY,
    /**< Set VBRSensitivity param of CVBR */

    BELL_VENC_ALL
    /**< Get/Set All parameters */

} BELL_VENC_PARAM_E;

/**
    \brief Encode Channel Specific Dynamic Parameters
*/
typedef struct
{
    Int32 frameRate;
    /**< Frame-rate */

    Int32 targetBitRate;
    /**< required bitrate in bits/second */

    Int32 intraFrameInterval;
    /**< I to P ratio or inter-frame interval */

    Int32 inputFrameRate;
    /**< Frame rate of the incoming content */

    Int32 rcAlg;
    /**< Rate-control Algorithm type */

    Int32 qpMin;
    /**< QP Min value */

    Int32 qpMax;
    /**< QP Max value */

    Int32 qpInit;
    /**< QP Init value */

    Int32 vbrDuration;
    /**< VBRDuration for CVBR */

    Int32 vbrSensitivity;
    /**< VBRSensitivity for CVBR */

} BELL_VENC_CHN_DYNAMIC_PARAM_S;

/**
    \brief Encode Channel Specific Static Parameters
*/
typedef struct
{
    Int32 videoWidth;
    /**< Read only: Encoded Video Width */

    Int32 videoHeight;
    /**< Read only: Encoded Video Height */

    Int32 enableAnalyticinfo;
    /**< Enable/Disable flag for MV Data generation  */
    
    Int32 enableWaterMarking;
    /**< Enable this option to Insert Water Mark SEI message in the bit stream */

    BELL_VENC_XDM_EncodingPreset encodingPreset;
    /**< Enum identifying predefined encoding presets @sa XDM_EncodingPreset  */

    Int32 maxBitRate;
   /**< Maximum Bit Rate for Encoder*/

    BELL_VENC_IH264ENC_SvcExtensionFlag enableSVCExtensionFlag;
    /**< Enable/Disable flag for SVC extension headers in h.264 Enc  */   

    BELL_VENC_RATE_CTRL_E rcType;
    /**< Rate-control type */

    BELL_VENC_CHN_H264_NumTemporalLayer_E numTemporalLayer;
     /**< Number of temporal layers the output bitstream should have */
 
    BELL_VENC_CHN_DYNAMIC_PARAM_S dynamicParam;
    /**< Initial values for dynamic parameters of encode channel */

} BELL_VENC_CHN_PARAMS_S;

/**
    \brief Encode Sub-system initialization parameters
*/
typedef struct {

    BELL_VENC_CHN_PARAMS_S encChannelParams[BELL_VENC_CHN_MAX];
    /**< Channel Specific Parameters */

    BELL_VENC_CHN_H264_Profile_E h264Profile[BELL_VENC_CHN_MAX];
    /**< Codec profile */

    UInt32 numPrimaryChn;
    /**< Number of encoder primary channels <D1> */

    UInt32 numSecondaryChn;
    /**< Number of encoder secondary channels <CIF> */

} BELL_VENC_PARAMS_S;


/** Maximum Video Decode channels */
#define BELL_VDEC_CHN_MAX			(64)

/**
    \brief Decode Channel Specific Dynamic Parameters
*/
typedef struct
{
    Int32 frameRate;
    /**< frame rate */

    Int32 targetBitRate;
    /**< target bit-rate in bits/second */

} BELL_VDEC_CHN_DYNAMIC_PARAM_S;

/**
    \brief Decode Channel Specific Static Parameters
*/
typedef struct
{
    Int32 maxVideoWidth;
    /**< Decoded frame width */

    Int32 maxVideoHeight;
    /**< Decoded frame height */

    Int32 numBufPerCh;
    /**< Number of output buffers per channel required based on input */

    Int32 displayDelay;
    /**< Number of output buffers per channel required based on input */

    BELL_VDEC_CHN_CODEC_E isCodec;
    /**< Codec type */

    BELL_VDEC_CHN_DYNAMIC_PARAM_S dynamicParam;
    /**< Initial values for dynamic parameters */

} BELL_VDEC_CHN_PARAMS_S;

/**
    \brief Decode Sub-system initialization parameters
*/
typedef struct {

    UInt32 numChn;
    /**< Number of decoder channels - USED only if BELL_VDEC_PARAMS_S.forceUseDecChannelParams = TRUE */

    Bool   forceUseDecChannelParams;
    /**< TRUE: Decoder width and height decided by BELL_VDEC_PARAMS_S.decChannelParams[] \n
         FALSE: Decoder width and height decided based on encoder width and height (Default)
    */

    BELL_VDEC_CHN_PARAMS_S decChannelParams[BELL_VDEC_CHN_MAX];
    /**< Channel Specific Parameters */

} BELL_VDEC_PARAMS_S;

/**********************************************************************************************************
 *
 * Above code is import from mcfw/interfaces/ti_venc.h & mcfw/interfaces/ti_vdec.h
 * Make sure they are up-to-date with MCFW version.
 *
 **********************************************************************************************************/

typedef struct CodecPrm
{
    BELL_VENC_PARAMS_S   vencParams;
    BELL_VDEC_PARAMS_S   vdecParams;
    BELL_USECASES_E usecase;
 
}CodecPrm;

Int32 Bell_printStatistics(Bool resetStats, Bool allChs);

Void Bell_api_param_init(CodecPrm *prm);

void* Bell_api_create();
int Bell_api_start(void *h, CodecPrm *prm);
int Bell_api_stop(void *h);
void Bell_api_delete(void* h);
int Bell_api_force_idr();

const BELL_VENC_PARAMS_S const* Bell_api_get_enc_param(Int32 vencChnId);

Int32 Bell_api_set_enc_dynamic_param(Int32 ChId, BELL_VENC_CHN_DYNAMIC_PARAM_S *dyn_prm, BELL_VENC_PARAM_E paramId);

int Bell_apiBitsGet(void *h, VCODEC_BITSBUF_LIST_S *buflist);
int Bell_apiBitsPut(VCODEC_BITSBUF_LIST_S *buflist);


VCODEC_BITSBUF_S* Bell_apiGetEmptyBuf(void *h, int size);

int Bell_apiBitsFeed(void *h, VCODEC_BITSBUF_S *pFullBufInfo);

Int32 Bell_api_disp_layout(int winId0, int winId1, int numWin);

#endif
