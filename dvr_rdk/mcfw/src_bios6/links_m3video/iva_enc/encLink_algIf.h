/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _ENC_LINK_ALG_IF_H_
#define _ENC_LINK_ALG_IF_H_

#include <mcfw/interfaces/link_api/encLink.h>
#include <ih264enc.h>
#include <ijpegenc.h>

#define ENC_LINK_ALG_VERSION_STRING_MAX_LEN                               (255)

typedef struct EncLink_AlgCreateParams {
    IVIDEO_Format format;
    IVIDEO_VideoLayout dataLayout;
    Bool singleBuf;
    Int32 maxWidth;
    Int32 maxHeight;
    Int32 maxInterFrameInterval;
    Int32 inputContentType;
    Int32 inputChromaFormat;
    Int32 profile;
    Int32 level;
    Bool  tilerEnable;
    Int32 enableAnalyticinfo;
    Int32 enableSVCExtensionFlag;
    Int32 enableWaterMarking;
    Int32 mvDataSize;
    Int32 maxBitRate;
    Int32 encodingPreset;
    Int32 rateControlPreset;
    Bool enableHighSpeed;
    Int32 numTemporalLayer;
} EncLink_AlgCreateParams;

typedef struct EncLink_AlgDynamicParams {
    Int32 startX;
    Int32 startY;
    Int32 inputWidth;
    Int32 inputHeight;
    Int32 inputPitch;
    Int32 targetBitRate;
    Int32 targetFrameRate;
    Int32 intraFrameInterval;
    Int32 interFrameInterval;
    Int32 rcAlg;
    Int32 qpMinI;
    Int32 qpMaxI;
    Int32 qpInitI;
    Int32 qpMinP;
    Int32 qpMaxP;
    Int32 qpInitP;
    Int32 forceFrame;
    Int32 vbrDuration;
    Int32 vbrSensitivity;
    Bool  forceFrameStatus;
    Int32 mvAccuracy;
    Int32 refFrameRate;
} EncLink_AlgDynamicParams;

#endif                                                     /* _ENC_LINK_ALG_IF_H_
                                                            */
