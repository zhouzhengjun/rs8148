/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/
/****************************************************************
**	
**	modify mark: VPS_SII9135_USE_REG_CACHE -> DEVICE_SII9135_USE_REG_CACHE
**  	VPS_SII9135_USE_AVI_FRAME_INFO  -> DEVICE_SII9135_USE_AVI_FRAME_INFO
**   this macro can be used in sii9135_priv.h.
*****************************************************************/
#include "ti_media_std.h"
#include "ti_vsys_common_def.h"
#include <device.h>
#include <device_videoDecoder.h>
//#include <device_tvp5158.h> mark
#include <sii9135_priv.h>
#include <osa_i2c.h>

/** \brief Timeout wait forever. */
#define VSYS_TIMEOUT_FOREVER           (~(0u))

/** \brief FVID2 API call returned with error as bad arguments.
 *  Typically, NULL pointer passed to the FVID2 API where its not expected. */
#define VSYS_EBADARGS                  ((Int32) -2)


/** \brief FVID2 API call returned with error as timed out. Typically API is
 *  waiting for some condition and returned as condition not happened
 *  in the timeout period. */
#define VSYS_ETIMEOUT                  ((Int32) -5)

typedef enum
{
    VSYS_DF_YUV422I_UYVY = 0x0000,
    /**< YUV 422 Interleaved format - UYVY. */
    VSYS_DF_YUV422I_YUYV,
    /**< YUV 422 Interleaved format - YUYV. */
    VSYS_DF_YUV422I_YVYU,
    /**< YUV 422 Interleaved format - YVYU. */
    VSYS_DF_YUV422I_VYUY,
    /**< YUV 422 Interleaved format - VYUY. */
    VSYS_DF_YUV422SP_UV,
    /**< YUV 422 Semi-Planar - Y separate, UV interleaved. */
    VSYS_DF_YUV422SP_VU,
    /**< YUV 422 Semi-Planar - Y separate, VU interleaved. */
    VSYS_DF_YUV422P,
    /**< YUV 422 Planar - Y, U and V separate. */
    VSYS_DF_YUV420SP_UV,
    /**< YUV 420 Semi-Planar - Y separate, UV interleaved. */
    VSYS_DF_YUV420SP_VU,
    /**< YUV 420 Semi-Planar - Y separate, VU interleaved. */
    VSYS_DF_YUV420P,
    /**< YUV 420 Planar - Y, U and V separate. */
    VSYS_DF_YUV444P,
    /**< YUV 444 Planar - Y, U and V separate. */
    VSYS_DF_YUV444I,
    /**< YUV 444 interleaved - YUVYUV... */
    VSYS_DF_RGB16_565 = 0x1000,
    /**< RGB565 16-bit - 5-bits R, 6-bits G, 5-bits B. */
    VSYS_DF_ARGB16_1555,
    /**< ARGB1555 16-bit - 5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (MSB). */
    VSYS_DF_RGBA16_5551,
    /**< RGBA5551 16-bit - 5-bits R, 5-bits G, 5-bits B, 1-bit Alpha (LSB). */
    VSYS_DF_ARGB16_4444,
    /**< ARGB4444 16-bit - 4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (MSB). */
    VSYS_DF_RGBA16_4444,
    /**< RGBA4444 16-bit - 4-bits R, 4-bits G, 4-bits B, 4-bit Alpha (LSB). */
    VSYS_DF_ARGB24_6666,
    /**< ARGB6666 24-bit - 6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (MSB). */
    VSYS_DF_RGBA24_6666,
    /**< RGBA6666 24-bit - 6-bits R, 6-bits G, 6-bits B, 6-bit Alpha (LSB). */
    VSYS_DF_RGB24_888,
    /**< RGB24 24-bit - 8-bits R, 8-bits G, 8-bits B. */
    VSYS_DF_ARGB32_8888,
    /**< ARGB32 32-bit - 8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (MSB). */
    VSYS_DF_RGBA32_8888,
    /**< RGBA32 32-bit - 8-bits R, 8-bits G, 8-bits B, 8-bit Alpha (LSB). */
    VSYS_DF_BGR16_565,
    /**< BGR565 16-bit -   5-bits B, 6-bits G, 5-bits R. */
    VSYS_DF_ABGR16_1555,
    /**< ABGR1555 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit Alpha (MSB). */
    VSYS_DF_ABGR16_4444,
    /**< ABGR4444 16-bit - 4-bits B, 4-bits G, 4-bits R, 4-bit Alpha (MSB). */
    VSYS_DF_BGRA16_5551,
    /**< BGRA5551 16-bit - 5-bits B, 5-bits G, 5-bits R, 1-bit Alpha (LSB). */
    VSYS_DF_BGRA16_4444,
    /**< BGRA4444 16-bit - 4-bits B, 4-bits G, 4-bits R, 4-bit Alpha (LSB). */
    VSYS_DF_ABGR24_6666,
    /**< ABGR6666 24-bit - 6-bits B, 6-bits G, 6-bits R, 6-bit Alpha (MSB). */
    VSYS_DF_BGR24_888,
    /**< BGR888 24-bit - 8-bits B, 8-bits G, 8-bits R. */
    VSYS_DF_ABGR32_8888,
    /**< ABGR8888 32-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit Alpha (MSB). */
    VSYS_DF_BGRA24_6666,
    /**< BGRA6666 24-bit - 6-bits B, 6-bits G, 6-bits R, 6-bit Alpha (LSB). */
    VSYS_DF_BGRA32_8888,
    /**< BGRA8888 32-bit - 8-bits B, 8-bits G, 8-bits R, 8-bit Alpha (LSB). */
    VSYS_DF_BITMAP8 = 0x2000,
    /**< BITMAP 8bpp. */
    VSYS_DF_BITMAP4_LOWER,
    /**< BITMAP 4bpp lower address in CLUT. */
    VSYS_DF_BITMAP4_UPPER,
    /**< BITMAP 4bpp upper address in CLUT. */
    VSYS_DF_BITMAP2_OFFSET0,
    /**< BITMAP 2bpp offset 0 in CLUT. */
    VSYS_DF_BITMAP2_OFFSET1,
    /**< BITMAP 2bpp offset 1 in CLUT. */
    VSYS_DF_BITMAP2_OFFSET2,
    /**< BITMAP 2bpp offset 2 in CLUT. */
    VSYS_DF_BITMAP2_OFFSET3,
    /**< BITMAP 2bpp offset 3 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET0,
    /**< BITMAP 1bpp offset 0 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET1,
    /**< BITMAP 1bpp offset 1 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET2,
    /**< BITMAP 1bpp offset 2 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET3,
    /**< BITMAP 1bpp offset 3 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET4,
    /**< BITMAP 1bpp offset 4 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET5,
    /**< BITMAP 1bpp offset 5 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET6,
    /**< BITMAP 1bpp offset 6 in CLUT. */
    VSYS_DF_BITMAP1_OFFSET7,
    /**< BITMAP 1bpp offset 7 in CLUT. */
    VSYS_DF_BITMAP8_BGRA32,
    /**< BITMAP 8bpp BGRA32. */
    VSYS_DF_BITMAP4_BGRA32_LOWER,
    /**< BITMAP 4bpp BGRA32 lower address in CLUT. */
    VSYS_DF_BITMAP4_BGRA32_UPPER,
    /**< BITMAP 4bpp BGRA32 upper address in CLUT. */
    VSYS_DF_BITMAP2_BGRA32_OFFSET0,
    /**< BITMAP 2bpp BGRA32 offset 0 in CLUT. */
    VSYS_DF_BITMAP2_BGRA32_OFFSET1,
    /**< BITMAP 2bpp BGRA32 offset 1 in CLUT. */
    VSYS_DF_BITMAP2_BGRA32_OFFSET2,
    /**< BITMAP 2bpp BGRA32 offset 2 in CLUT. */
    VSYS_DF_BITMAP2_BGRA32_OFFSET3,
    /**< BITMAP 2bpp BGRA32 offset 3 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET0,
    /**< BITMAP 1bpp BGRA32 offset 0 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET1,
    /**< BITMAP 1bpp BGRA32 offset 1 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET2,
    /**< BITMAP 1bpp BGRA32 offset 2 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET3,
    /**< BITMAP 1bpp BGRA32 offset 3 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET4,
    /**< BITMAP 1bpp BGRA32 offset 4 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET5,
    /**< BITMAP 1bpp BGRA32 offset 5 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET6,
    /**< BITMAP 1bpp BGRA32 offset 6 in CLUT. */
    VSYS_DF_BITMAP1_BGRA32_OFFSET7,
    /**< BITMAP 1bpp BGRA32 offset 7 in CLUT. */
    VSYS_DF_BAYER_RAW = 0x3000,
    /**< Bayer pattern. */
    VSYS_DF_RAW_VBI,
    /**< Raw VBI data. */
    VSYS_DF_RAW,
    /**< Raw data - Format not interpreted. */
    VSYS_DF_MISC,
    /**< For future purpose. */
    VSYS_DF_INVALID
    /**< Invalid data format. Could be used to initialize variables. */
} VSYS_DataFormat;

Int32 Device_sii9135I2cRead8 ( Device_Sii9135Obj * pObj,
                        UInt32 i2cInstId, UInt32 i2cDevAddr,
                        const UInt8 * regAddr,
                        UInt8 * regValue, UInt32 numRegs )
{
    Int32 status = 0;
	UInt32 i;
	static Uint8 r[I2C_TRANSFER_SIZE_MAX], v[I2C_TRANSFER_SIZE_MAX];	
	Uint8 devAddr;
    #ifdef DEVICE_SII9135_USE_REG_CACHE
    UInt32 reg, port;

    if(pObj->createArgs.deviceI2cAddr[0]==i2cDevAddr)
        port = 0;
    else
        port = 1;

    for(reg=0; reg<numRegs; reg++)
    {
        regValue[reg] = pObj->regCache[port][regAddr[reg]];
    }
    #else
	
	#if 1
	r[0]=0x40;
      for(i=0; i<19; i++)
        v[i] = 0;

      for(i=1; i<19; i++)
      {
        r[i] = r[0]+i;
      }
	devAddr=0x34;
	OSA_i2cRead8 (&gDevice_sii9135CommonObj.i2cHandle, devAddr, r, v, 19);
	
		printf("Device_sii9135I2cRead8 handle=0x%x, devAddr=%x\r\n",gDevice_sii9135CommonObj.i2cHandle, devAddr);
	
	    for (i = 0;i < 19; i++ )
    {
		printf("%x,%x ",r[i],v[i]);	

        
    }	
		printf(" \r\n");
		#endif
	status = OSA_i2cRead8 (&gDevice_sii9135CommonObj.i2cHandle, i2cDevAddr, regAddr, regValue, numRegs);

    if ( status < 0 )
       return -1;
	
	#endif

    return status;
}

Int32 Device_sii9135I2cWrite8 ( Device_Sii9135Obj * pObj,
                        UInt32 i2cInstId, UInt32 i2cDevAddr,
                        const UInt8 * regAddr,
                        UInt8 * regValue, UInt32 numRegs )
{
    Int32 status = 0;

    #ifdef DEVICE_SII9135_USE_REG_CACHE
    UInt32 reg, port;

    if(pObj->createArgs.deviceI2cAddr[0]==i2cDevAddr)
        port = 0;
    else
        port = 1;

    for(reg=0; reg<numRegs; reg++)
    {
        pObj->regCache[port][regAddr[reg]] = regValue[reg];
    }
    #endif

	status = OSA_i2cWrite8(&gDevice_sii9135CommonObj.i2cHandle, 
								i2cDevAddr, regAddr,
								regValue, numRegs);
	if ( status < 0 )
       return -1;
	
    return status;
}

Int32 Device_sii9135ResetRegCache(Device_Sii9135Obj * pObj)
{
    #ifdef DEVICE_SII9135_USE_REG_CACHE

    memset(pObj->regCache, 0, sizeof(pObj->regCache));

    // for now hard code default values in registers 0x0 to 0x7
    pObj->regCache[0][0] = 0x01;
    pObj->regCache[0][1] = 0x00;
    pObj->regCache[0][2] = 0x35;
    pObj->regCache[0][3] = 0x91;
    pObj->regCache[0][4] = 0x03;
    pObj->regCache[0][5] = 0x00;
    pObj->regCache[0][6] = 0x1F; // assume signal is detected
    pObj->regCache[0][7] = 0x00;

    #endif

    return 0;
}

Int32 Device_sii9135UpdateRegCache(Device_Sii9135Obj * pObj)
{
    Int32 status = 0;

    #ifdef DEVICE_SII9135_USE_REG_CACHE

    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 numRegs = 0xf8;
    UInt8 devAddr[2];
    UInt8 regAddr[8];
    UInt8 regValue[8];

    pCreateArgs = &pObj->createArgs;

    devAddr[0] = pCreateArgs->deviceI2cAddr[0];
    devAddr[1] = pCreateArgs->deviceI2cAddr[0] + DEVICE_SII9135_I2C_PORT1_OFFSET;

    Device_sii9135ResetRegCache(pObj);

    regAddr[0] = 0;
    regAddr[1] = 1;

    // dummy read to make next read io work
    status = OSA_i2cRead8(
                &gDevice_sii9135CommonObj.i2cHandle,
                devAddr[0],
                regAddr,
                regValue,
                2
                );
     if ( status < 0 )
        return status;

    status = OSA_i2cRawRead8(
                &gDevice_sii9135CommonObj.i2cHandle,
                devAddr[0],
                &pObj->regCache[0][8],
                numRegs
                );
    if ( status < 0 )
        return status;

    status = OSA_i2cRawRead8(
                &gDevice_sii9135CommonObj.i2cHandle,
                devAddr[1],
                &pObj->regCache[1][8],
                numRegs
                );

    if ( status < 0 )
        return status;

    #endif

    return status;
}

/*
  For SII9135 below parameters in Vps_VideoDecoderVideoModeParams are ignored
  and any value set by user for these parameters is not effective.

  videoIfMode
  standard
  videoCaptureMode
  videoSystem
  videoCropEnable

  Depending on video data format SII9135 is configured as below

  videoDataFormat =
   FVID2_DF_YUV422P   : 16-bit YUV422 single CH embedded sync auto-detect mode
   FVID2_DF_YUV444P   : 24-bit YUV444 single CH discrete sync auto-detect mode
   FVID2_DF_RGB24_888 : 24-bit RGB24  single CH discrete sync auto-detect mode
*/
Int32 Device_sii9135SetVideoMode ( Device_Sii9135Obj * pObj,
                                Device_VideoDecoderVideoModeParams * pPrm )
{
    Int32 status = 0;
    Device_Sii9135AviInfoFrame aviInfo;

    /*
     * reset device
     */
//    status = Device_sii9135Reset ( pObj );
//    if ( status < 0 )
//        return status;

    /*
     * detect video source properties
     */
    status = Device_sii9135DetectVideo ( pObj,
                                      &aviInfo, pPrm->videoAutoDetectTimeout );

    if ( status < 0 )
        return status;

    if(pPrm->standard==VSYS_STD_480I
        ||
        pPrm->standard==VSYS_STD_576I
        ||
        pPrm->standard==VSYS_STD_D1
        ||
        pPrm->standard==VSYS_STD_NTSC
        ||
        pPrm->standard==VSYS_STD_PAL
    )
    {
        aviInfo.pixelRepeat = DEVICE_SII9135_AVI_INFO_PIXREP_2X;
    }

    /*
     * setup video processing path based on detected source
     */
    status = Device_sii9135SetupVideo ( pObj, &aviInfo, pPrm );
    if ( status < 0 )
        return status;

    return status;
}

/*
  Setup video processing path based on detected source

  pAviInfoFrame - input source properties
  pPrm - user required parameters
*/
Int32 Device_sii9135SetupVideo ( Device_Sii9135Obj * pObj,
                              Device_Sii9135AviInfoFrame * pAviInfo,
                              Device_VideoDecoderVideoModeParams * pPrm )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[32];
    UInt8 regValue[32];
    UInt8 numRegs;
    UInt8 devAddr;
    UInt32 insSavEav, cscR2Y, upSmp, downSmp, chMap;
    UInt32 cscY2R, outColorSpace, isBT709, inPixRep;

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0];

    outColorSpace = DEVICE_SII9135_COLOR_FORMAT_YUV;
    if ( pPrm->videoDataFormat == VSYS_DF_RGB24_888 )
        outColorSpace = DEVICE_SII9135_COLOR_FORMAT_RGB;

    inPixRep = pAviInfo->pixelRepeat;
    if ( inPixRep > DEVICE_SII9135_AVI_INFO_PIXREP_4X )
    {
        inPixRep = 0;
        status = -1;
    }

    isBT709 = 0;
    if ( pAviInfo->colorImetric == DEVICE_SII9135_AVI_INFO_CMETRIC_ITU601 )
        isBT709 = 0;
    if ( pAviInfo->colorImetric == DEVICE_SII9135_AVI_INFO_CMETRIC_ITU709 )
        isBT709 = 1;
    cscR2Y = 0;
    upSmp = 0;
    downSmp = 0;
    cscY2R = 0;
    insSavEav = 0;

    switch ( pAviInfo->colorSpace )
    {
        case DEVICE_SII9135_AVI_INFO_COLOR_RGB444:
            switch ( pPrm->videoDataFormat )
            {
                case VSYS_DF_YUV422P:
                    cscR2Y = 1;
                    downSmp = 1;
                    insSavEav = 1;
                    break;

                case VSYS_DF_YUV444P:
                    cscR2Y = 1;
                    break;

                case VSYS_DF_RGB24_888:
                    break;

                default:
                    status = -2;
                    break;
            }
            break;

        case DEVICE_SII9135_AVI_INFO_COLOR_YUV444:
            switch ( pPrm->videoDataFormat )
            {
                case VSYS_DF_YUV422P:
                    downSmp = 1;
                    insSavEav = 1;
                    break;

                case VSYS_DF_YUV444P:
                    break;

                case VSYS_DF_RGB24_888:
                    cscY2R = 1;
                    break;

                default:
                    status = -2;
                    break;
            }
            break;

        case DEVICE_SII9135_AVI_INFO_COLOR_YUV422:
            switch ( pPrm->videoDataFormat )
            {
                case VSYS_DF_YUV422P:
                    insSavEav = 1;
                    downSmp = 1;					
                    break;

                case VSYS_DF_YUV444P:
                    upSmp = 1;
                    break;

                case VSYS_DF_RGB24_888:
                    upSmp = 1;
                    cscY2R = 1;
                    break;

                default:
                    status = -2;
                    break;
            }
            break;

        default:
            status = -1;
            break;
    }
	
    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_MODE_1;
    regValue[numRegs] = ( insSavEav << 7 )  /* 1: SAV2EAV enable   , 0: disable */
        | ( 0 << 6 )    /* 1: Mux Y/C          , 0: No MUX  */
        | ( 1 << 5 )    /* 1: Dither enable    , 0: disable */
        | ( 1 << 4 )    /* 1: R2Y compress     , 0: bypass  */
        | ( cscR2Y << 3 )   /* 1: Enable R2Y CSC   , 0: bypass  */
        | ( upSmp << 2 )    /* 1: YUV422 to YUV444 , 0: bypass  */
        | ( downSmp << 1 )  /* 1: YUV444 to YUV422 , 0: bypass  */
        ;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_MODE_2;
    regValue[numRegs] = ( 0 << 6 )  /* 0: Dither to 8bits, 1: 10bits, 2: 12bits  */
        | ( 0 << 5 )    /* 0: EVNODD LOW if field 0 is even, 1: HIGH */
        | ( 1 << 3 )    /* 1: Y2R compress     , 0: bypass           */
        | ( cscY2R << 2 )   /* 1: Y2R CSC          , 0: bypass           */
        | ( outColorSpace << 1 )    /* 0: Output format RGB, 1: YUV           */
        | ( 1 << 0 )    /* 1: Range clip enable, 0: disable          */
        ;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_CTRL;
    regValue[numRegs] = ( 0 << 7 )  /* 0: Do not invert VSYNC, 1: invert */
        | ( 0 << 6 )    /* 0: Do not invert HSYNC, 1: invert */
        | ( isBT709 << 2 )  /* 0: Y2R BT601          , 1: BT709  */
        | ( 0 << 1 )    /* 0: 8bits RGB or YUV   , 1: YUV422 > 8 bits */
        | ( isBT709 << 0 )  /* 0: R2Y BT601          , 1: BT709  */
        ;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_SYS_CTRL_1;
    regValue[numRegs] = ( inPixRep << 6 )  /* 0: Output pixel clock divided 1x, 1: 1/2x, 3: 1/4x */
        | ( inPixRep << 4 ) /* 0: Input pixel replicate 1x, 1:2x, 3:4x            */
        | ( 1 << 2 )    /* 0: 12-bit mode         , 1: 24-bit mode      */
        | ( 0 << 1 )    /* 0: Normal output clock , 1: Invert clock     */
        | ( 1 << 0 )    /* 0: Power down          , 1: Normal operation */
        ;
    numRegs++;

    if((devAddr & 0x1)==0)
    {
        /* this is device connected to VIP1 and is connected in
            16-bit mode */
        chMap = 0x0;
    }
    else
    {
        /* this is device connected to VIP0 and is connected in
            24-bit mode */
        chMap = 0x5;
    }

#if defined(TI_814X_BUILD) || defined(TI_8107_BUILD)

    /* On VC Card
        Q4-Q10  =>  D0 D7   ->  Red / CbCr
        Q16-Q23 =>  D8 D16  ->  Green / Y
        Q28-Q35 =>  D17 D23 ->  Blue */
 //   chMap = 0x05;
#endif /* TI_814X_BUILD || TI_8107_BUILD */
    /*     Q[23:16]  Q[15:8]  Q[7:0]
        0:     R        G       B
        1:     R        B       G
        2:     G        R       B
        3:     G        B       R
        4:     B        R       G
        5:     B        G       R
    */
    regAddr[numRegs] = DEVICE_SII9135_REG_VID_CH_MAP;
    regValue[numRegs] = chMap;
        ;
    numRegs++;
	
	printf("set video mode reg isBT709=%d, is cscR2Y=%d, upSmp=%d, downSmp=%d, cscY2R=%d, insSavEav=%d, chMap=%d\r\n", isBT709, cscR2Y,upSmp,downSmp,cscY2R, insSavEav,chMap);	

    status = Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );
    if ( status < 0 )
       return -1;

    return status;
}

/*
  Read AVI packet info and parse information from it

  pAviInfo - parsed information returned by this API
*/
Int32 Device_sii9135ReadAviInfo ( Device_Sii9135Obj * pObj,
                               Device_Sii9135AviInfoFrame * pAviInfo )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[32];
    UInt8 regValue[32];
    UInt8 numRegs;
    UInt8 devAddr;
    UInt8 aviAddr;
    UInt8 *aviData;

    /*
     * read AVI Info Frame
     */

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0] + DEVICE_SII9135_I2C_PORT1_OFFSET;
	
	printf("read AVI info devAddr=%x\r\n", devAddr);
	
    numRegs = 0;

    for ( aviAddr = DEVICE_SII9135_REG_AVI_TYPE;
          aviAddr <= DEVICE_SII9135_REG_AVI_DBYTE15; aviAddr++ )
    {
        regAddr[numRegs] = aviAddr;
        regValue[numRegs] = 0;
        numRegs++;
    }

    status = Device_sii9135I2cRead8 ( pObj,
                               pCreateArgs->deviceI2cInstId,
                               devAddr, regAddr, regValue, numRegs );

    if ( status < 0 ){

		printf("read AVI info failed\r\n");
       return -1;
	   }
	    for ( aviAddr = 0;
          aviAddr < numRegs; aviAddr++ )
    {
		printf("%x ",regValue[aviAddr]);	
				printf(" \r\n");
        
    }
    status = -1;

    if ( regValue[0] == DEVICE_SII9135_AVI_INFO_PACKET_CODE
         && regValue[1] == DEVICE_SII9135_AVI_INFO_VERSION_CODE
         && regValue[2] == DEVICE_SII9135_AVI_INFO_PACKET_LENGTH )
    {
        /*
         * Valid AVI packet recevied
         */

        status = 0;

        aviData = &regValue[3]; /* point to start of AVI data checksum */

        /*
         * parse information
         */
        pAviInfo->colorSpace = ( ( aviData[1] >> 5 ) & 0x3 );
        pAviInfo->colorImetric = ( ( aviData[2] >> 6 ) & 0x3 );
        pAviInfo->pixelRepeat = ( ( aviData[5] >> 0 ) & 0xF );
		printf("AVI info valid colorSpace=%d, colorImetric=%d, pixelRepeat=%d \r\n", pAviInfo->colorSpace,pAviInfo->colorImetric,pAviInfo->pixelRepeat);		
    }

    return status;
}

/*
  Wait until video is detected and the get video properties
*/
Int32 Device_sii9135DetectVideo ( Device_Sii9135Obj * pObj,
                               Device_Sii9135AviInfoFrame * pAviInfo,
                               UInt32 timeout )
{
    Int32 status = 0;
    VCAP_VIDEO_SOURCE_CH_STATUS_S vidStatus;
    VCAP_VIDEO_SOURCE_STATUS_PARAMS_S vidStatusPrm;
    UInt32 loopTimeout, curTimeout = 0;

    vidStatusPrm.channelNum = 0;

    timeout = 1000; /* hard code for now */
    loopTimeout = 250;   /* check video status every 50 OS ticks */

//    Task_sleep(100);
	usleep(100000);

    while ( 1 )
    {
        status = Device_sii9135GetVideoStatus ( pObj, &vidStatusPrm, &vidStatus );
        if ( status != 0 )
            return status;

        if ( vidStatus.isVideoDetect )
        {
            break;
        }
        usleep ( loopTimeout*1000 );

        if ( timeout != VSYS_TIMEOUT_FOREVER )
        {
            curTimeout += loopTimeout;

            if ( curTimeout >= timeout )
            {
                status = VSYS_ETIMEOUT;
                break;
            }
        }
    }

    /*
     * video detected, read AVI info
     */
    status |= Device_sii9135ReadAviInfo ( pObj, pAviInfo );
	printf("**USE_AVI_FRAME_INFO colorSpace=%d, colorImetric=%d, pixelRepeat=%d \r\n", pAviInfo->colorSpace,pAviInfo->colorImetric,pAviInfo->pixelRepeat);
    #ifndef DEVICE_SII9135_USE_AVI_FRAME_INFO
    // Hardcode AVI frame info
    pAviInfo->colorSpace = DEVICE_SII9135_AVI_INFO_COLOR_YUV444;
    pAviInfo->colorImetric = DEVICE_SII9135_AVI_INFO_CMETRIC_ITU709;
    pAviInfo->pixelRepeat = DEVICE_SII9135_AVI_INFO_PIXREP_NONE;
	printf("**hardcode AVI info colorSpace=%d, colorImetric=%d, pixelRepeat=%d \r\n", pAviInfo->colorSpace,pAviInfo->colorImetric,pAviInfo->pixelRepeat);
    status = 0;
    #endif

    return status;
}

/*
  Select TMDS core
*/
Int32 Device_sii9135SelectTmdsCore ( Device_Sii9135Obj * pObj, UInt32 coreId )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[8];
    UInt8 regValue[8];
    UInt8 numRegs;
    UInt8 devAddr;

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0];

    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_SYS_SWTCHC;
    regValue[numRegs] = 0x80;    /* DDCDLY_EN = 1, default value */
    if ( coreId == 0 )
        regValue[numRegs] |= DEVICE_SII9135_SEL_TMDS_CORE_0;    /* select core 0 */
    else
        regValue[numRegs] |= DEVICE_SII9135_SEL_TMDS_CORE_1;    /* select core 1 */
    numRegs++;

    status = Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    return status;
}

/*
  This API
  - Disable output
  - Power-ON of all module
  - Manual reset of SII9135 and then setup in auto reset mode
  - Select default TMDS core
*/
Int32 Device_sii9135Reset ( Device_Sii9135Obj * pObj )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[8];
    UInt8 regValue[8];
    UInt8 numRegs;
    UInt8 devAddr;

    Device_sii9135ResetRegCache(pObj);

    /*
     * disable outputs
     */
    status = Device_sii9135Stop ( pObj );
    if ( status != 0 )
        return status;

    /*
     * Normal operation
     */
    status = Device_sii9135PowerDown ( pObj, FALSE );
    if ( status != 0 )
        return status;

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0];

    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_SW_RST_0;
    regValue[numRegs] = 0x2F;   /* reset AAC, HDCP, ACR, audio FIFO, SW */
    numRegs++;

    status = Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    /*
     * wait for reset to be effective
     */
    usleep ( 50000 );

    /*
     * remove reset and setup in auto-reset mode
     */
    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_SW_RST_0;
    regValue[numRegs] = 0x10;   /* auto-reset SW */
    numRegs++;

    status = Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    /*
     * select TDMS core
     */
    status = Device_sii9135SelectTmdsCore ( pObj, DEVICE_SII9135_CORE_ID_DEFAULT );
    if ( status != 0 )
        return status;

    status = Device_sii9135UpdateRegCache(pObj);
    if ( status != 0 )
        return status;

    return status;
}

/*
  Power-ON All modules
*/
Int32 Device_sii9135PowerDown ( Device_Sii9135Obj * pObj, UInt32 powerDown )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[8];
    UInt8 regValue[8];
    UInt8 numRegs;
    UInt8 devAddr;

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0];

    numRegs = 0;
    regAddr[numRegs] = DEVICE_SII9135_REG_SYS_CTRL_1;
    regValue[numRegs] = 0;
    numRegs++;

    status = Device_sii9135I2cRead8 ( pObj,
                                pCreateArgs->deviceI2cInstId,
                               devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    if ( powerDown )
        regValue[0] = 0x04;  /* power down  */
    else
        regValue[0] = 0x05;  /* normal operation */

    status = Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    return status;
}

/*
  Enable output port
*/
Int32 Device_sii9135OutputEnable ( Device_Sii9135Obj * pObj, UInt32 enable )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[8];
    UInt8 regValue[8];
    UInt8 numRegs;
    UInt8 devAddr;

    pCreateArgs = &pObj->createArgs;

    devAddr = pCreateArgs->deviceI2cAddr[0] + DEVICE_SII9135_I2C_PORT1_OFFSET;

    numRegs = 0;
    regAddr[numRegs] = DEVICE_SII9135_REG_SYS_PWR_DWN_2;
    regValue[numRegs] = 0xC3;   /* Enable all expect outputs  */
    if ( enable )
        regValue[numRegs] |= ( 1 << 2 );    /* Enable outputs  */

    regAddr[numRegs] = DEVICE_SII9135_REG_SYS_PWR_DWN;
    regValue[numRegs] = 0xAD;   /* Enable all expect outputs  */
    if ( enable )
        regValue[numRegs] |= ( 1 << 6 );    /* Enable outputs  */

    numRegs++;

    status =Device_sii9135I2cWrite8 ( pObj, pCreateArgs->deviceI2cInstId,
                                devAddr, regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    return status;
}

/*
  Stop output port
*/
Int32 Device_sii9135Stop ( Device_Sii9135Obj * pObj )
{
    Int32 status = 0;

    status = Device_sii9135OutputEnable ( pObj, FALSE );

    return status;
}

/*
  Start output port
*/
Int32 Device_sii9135Start ( Device_Sii9135Obj * pObj )
{
    Int32 status = 0;

    status = Device_sii9135OutputEnable ( pObj, TRUE );

    return status;
}

/*
  Get Chip ID and revision ID
*/
Int32 Device_sii9135GetChipId ( Device_Sii9135Obj * pObj,
                             Device_VideoDecoderChipIdParams * pPrm,
                             Device_VideoDecoderChipIdStatus * pStatus )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[8];
    UInt8 regValue[8];
    UInt8 numRegs;

    if ( pStatus == NULL || pPrm == NULL )
        return -2;

    memset ( pStatus, 0, sizeof ( *pStatus ) );

    pCreateArgs = &pObj->createArgs;

    if ( pPrm->deviceNum >= pCreateArgs->numDevicesAtPort )
        return -2;

    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_VND_IDL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VND_IDH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DEV_IDL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DEV_IDH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DEV_REV;
    regValue[numRegs] = 0;
    numRegs++;

    status = Device_sii9135I2cRead8 (
                               pObj,
                               pCreateArgs->deviceI2cInstId,
                               pCreateArgs->deviceI2cAddr[pPrm->deviceNum],
                               regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    pStatus->chipId = ( ( UInt32 ) regValue[3] << 8 ) | regValue[2];
    pStatus->chipRevision = regValue[4];
    pStatus->firmwareVersion = ( ( UInt32 ) regValue[1] << 8 ) | regValue[0];

    return status;
}

/*
  Get and detect video status
*/

Int32 Device_sii9135GetVideoStatus ( Device_Sii9135Obj * pObj,
                                  VCAP_VIDEO_SOURCE_STATUS_PARAMS_S * pPrm,
                                  VCAP_VIDEO_SOURCE_CH_STATUS_S * pStatus )
{
    Int32 status = 0;
    Device_VideoDecoderCreateParams *pCreateArgs;
    UInt8 regAddr[16];
    UInt8 regValue[16];
    UInt8 numRegs;
    UInt32 hRes, vRes, dePix, deLin, xclkInPclk;
    UInt32 sysStat, vidStat;
    UInt32 t_line;

    if ( pStatus == NULL || pPrm == NULL )
        return VSYS_EBADARGS;

    memset ( pStatus, 0, sizeof ( *pStatus ) );

    pCreateArgs = &pObj->createArgs;

    numRegs = 0;

    regAddr[numRegs] = DEVICE_SII9135_REG_H_RESL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_H_RESH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_V_RESL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_V_RESH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DE_PIXL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DE_PIXH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DE_LINL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_DE_LINH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_XPCNTL;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_XPCNTH;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_VID_STAT;
    regValue[numRegs] = 0;
    numRegs++;

    regAddr[numRegs] = DEVICE_SII9135_REG_STATE;
    regValue[numRegs] = 0;
    numRegs++;

    status = Device_sii9135I2cRead8 (
                               pObj,
                               pCreateArgs->deviceI2cInstId,
                               pCreateArgs->deviceI2cAddr[0],
                               regAddr, regValue, numRegs );

    if ( status != 0 )
        return VSYS_ETIMEOUT;

    /*
     * horizontal resolution
     */
    hRes = ( ( UInt32 ) regValue[1] << 8 ) | regValue[0];

    /*
     * vertical resolution
     */
    vRes = ( ( UInt32 ) regValue[3] << 8 ) | regValue[2];

    /*
     * horizontal active data resolution
     */
    dePix = ( ( UInt32 ) regValue[5] << 8 ) | regValue[4];

    /*
     * vertical active data resolution
     */
    deLin = ( ( UInt32 ) regValue[7] << 8 ) | regValue[6];

    /*
     * number of xclks per 2048 video clocks
     */
    xclkInPclk = ( ( UInt32 ) regValue[9] << 8 ) | regValue[8];

    /*
     * video status
     */
    vidStat = regValue[10];

    /*
     * system status
     */
    sysStat = regValue[11];

    if ( sysStat & DEVICE_SII9135_VID_DETECT )
    {
        pStatus->isVideoDetect = TRUE;

        if ( vidStat & DEVICE_SII9135_VID_INTERLACE )
        {
            pStatus->isInterlaced = TRUE;
        }

        pStatus->frameWidth = dePix;
        pStatus->frameHeight = deLin;

        /*
         * time interval in usecs for each line
         */
        t_line = ( UInt32 ) ( ( ( UInt32 ) hRes * xclkInPclk * 1000 ) / ( DEVICE_SII9135_FXTAL_KHZ * 2048 ) ); /* in usecs */
		
        printf(" SII9135: tline is %d, xclkInPclk is %d, DEVICE_SII9135_FXTAL_KHZ is %d\n",  t_line,   xclkInPclk, DEVICE_SII9135_FXTAL_KHZ);

        /*
         * time interval in usecs for each frame/field
         */
        pStatus->frameInterval = t_line * vRes;
        if(pStatus->frameInterval==0) pStatus->frameInterval=1;
        printf(" SII9135: %dx%d, %dx%d@%dHz, %d\n", hRes, vRes,
                pStatus->frameWidth,
                pStatus->frameHeight,
                1000000/pStatus->frameInterval,
                pStatus->isInterlaced
            );

    }

    return status;
}

