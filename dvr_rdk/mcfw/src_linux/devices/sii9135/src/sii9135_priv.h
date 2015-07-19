/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _DEVICE_SII9135_PRIV_H_
#define _DEVICE_SII9135_PRIV_H_

#include <osa_sem.h>
#include <osa_i2c.h>

#include "ti_vcap_common_def.h"

//#define DEVICE_SII9135_USE_REG_CACHE
#define DEVICE_SII9135_USE_AVI_FRAME_INFO

/* Driver object state - NOT IN USE */
#define DEVICE_SII9135_OBJ_STATE_UNUSED   (0)

/* Driver object state - IN USE and IDLE */
#define DEVICE_SII9135_OBJ_STATE_IDLE     (1)

/* SII9135 I2C Port1 offset from Port0  */
#define DEVICE_SII9135_I2C_PORT1_OFFSET   (4)

/* SII9135 Default TMDS Core ID to use  */
#define DEVICE_SII9135_CORE_ID_DEFAULT    (0)

/* SII9135 Registers - I2C Port 0 */
#define DEVICE_SII9135_REG_VND_IDL         (0x00)
#define DEVICE_SII9135_REG_VND_IDH         (0x01)
#define DEVICE_SII9135_REG_DEV_IDL         (0x02)
#define DEVICE_SII9135_REG_DEV_IDH         (0x03)
#define DEVICE_SII9135_REG_DEV_REV         (0x04)
#define DEVICE_SII9135_REG_SW_RST_0        (0x05)
#define DEVICE_SII9135_REG_STATE           (0x06)
#define DEVICE_SII9135_REG_SW_RST_1        (0x07)
#define DEVICE_SII9135_REG_SYS_CTRL_1      (0x08)
#define DEVICE_SII9135_REG_SYS_SWTCHC      (0x09)
#define DEVICE_SII9135_REG_H_RESL          (0x3A)
#define DEVICE_SII9135_REG_H_RESH          (0x3B)
#define DEVICE_SII9135_REG_V_RESL          (0x3C)
#define DEVICE_SII9135_REG_V_RESH          (0x3D)
#define DEVICE_SII9135_REG_VID_CTRL        (0x48)
#define DEVICE_SII9135_REG_VID_MODE_2      (0x49)
#define DEVICE_SII9135_REG_VID_MODE_1      (0x4A)
#define DEVICE_SII9135_REG_VID_BLANK1      (0x4B)
#define DEVICE_SII9135_REG_VID_BLANK2      (0x4C)
#define DEVICE_SII9135_REG_VID_BLANK3      (0x4D)
#define DEVICE_SII9135_REG_DE_PIXL         (0x4E)
#define DEVICE_SII9135_REG_DE_PIXH         (0x4F)
#define DEVICE_SII9135_REG_DE_LINL         (0x50)
#define DEVICE_SII9135_REG_DE_LINH         (0x51)
#define DEVICE_SII9135_REG_VID_STAT        (0x55)
#define DEVICE_SII9135_REG_VID_CH_MAP      (0x56)
#define DEVICE_SII9135_REG_VID_XPCNTL      (0x6E)
#define DEVICE_SII9135_REG_VID_XPCNTH      (0x6F)

/* SII9135 Registers - I2C Port 1 */
#define DEVICE_SII9135_REG_SYS_PWR_DWN_2   (0x3E)
#define DEVICE_SII9135_REG_SYS_PWR_DWN     (0x3F)
#define DEVICE_SII9135_REG_AVI_TYPE        (0x40)
#define DEVICE_SII9135_REG_AVI_DBYTE15     (0x52)

/* SII9135 Registers values */

/* PCLK stable, Clock detect, Sync Detect */
#define DEVICE_SII9135_VID_DETECT          ((1<<0) | (1<<1) | (1<<4))

/* Video interlace status */
#define DEVICE_SII9135_VID_INTERLACE       (1<<2)

/* color format */
#define DEVICE_SII9135_COLOR_FORMAT_RGB    (0)
#define DEVICE_SII9135_COLOR_FORMAT_YUV    (1)

/* Select TMDS core */
#define DEVICE_SII9135_SEL_TMDS_CORE_0     ((1<<0)|(1<<4))
#define DEVICE_SII9135_SEL_TMDS_CORE_1     ((1<<1)|(1<<5))

/* F_xtal frequency in Khz  */
#define DEVICE_SII9135_FXTAL_KHZ           (27000)

/* HDMI AVI InfoFrame Packet info  */

/* color space  */
#define DEVICE_SII9135_AVI_INFO_COLOR_RGB444       (0)
#define DEVICE_SII9135_AVI_INFO_COLOR_YUV444       (1)
#define DEVICE_SII9135_AVI_INFO_COLOR_YUV422       (2)

/* color imetric  */
#define DEVICE_SII9135_AVI_INFO_CMETRIC_NO_DATA    (0)
#define DEVICE_SII9135_AVI_INFO_CMETRIC_ITU601     (1)
#define DEVICE_SII9135_AVI_INFO_CMETRIC_ITU709     (2)

/* pixel repition */
#define DEVICE_SII9135_AVI_INFO_PIXREP_NONE        (0)
#define DEVICE_SII9135_AVI_INFO_PIXREP_2X          (1)
#define DEVICE_SII9135_AVI_INFO_PIXREP_4X          (3)

/* AVI packet info values */
#define DEVICE_SII9135_AVI_INFO_PACKET_CODE        (0x82)
#define DEVICE_SII9135_AVI_INFO_VERSION_CODE       (0x02)
#define DEVICE_SII9135_AVI_INFO_PACKET_LENGTH      (0x0D)

typedef struct
{
    Bool   isPalMode;
	#if 0
    /*palMode to be set based on detect status */
    Device_VideoDecoderVideoModeParams videoModeParams;
	#endif
	Device_VideoDecoderCreateParams createArgs;
    /* create time arguments */
    /* video mode params */
    UInt8 regCache[2][256]; /* register read cache */
} Device_Sii9135Obj;

typedef Device_Sii9135Obj * Device_Sii9135Handle;

typedef struct
{

    OSA_I2cHndl i2cHandle;
    /* i2cHandle for i2c read write operations */
    Device_Sii9135Handle sii9135handle[DEVICE_MAX_HANDLES];
    /*
     * Sii9135 handle objects
     */

} Device_Sii9135CommonObj;

/*
  Information that is parsed from HDMI AVIInfoFrame packet
*/
typedef struct
{
    UInt32 colorSpace;          /* RGB444 ot YUV422 or YUV422 */
    UInt32 colorImetric;        /* BT709 or BT601 */
    UInt32 pixelRepeat;         /* 1x, 2x, 4x */

} Device_Sii9135AviInfoFrame;

Device_Sii9135CommonObj gDevice_sii9135CommonObj;

Int32 Device_sii9135GetChipId ( Device_Sii9135Obj * pObj,
                             Device_VideoDecoderChipIdParams * pPrm,
                             Device_VideoDecoderChipIdStatus * pStatus );


Int32 Device_sii9135GetVideoStatus ( Device_Sii9135Obj * pObj,
                                  VCAP_VIDEO_SOURCE_STATUS_PARAMS_S * pPrm,
                                  VCAP_VIDEO_SOURCE_CH_STATUS_S * pStatus );


Int32 Device_sii9135Reset ( Device_Sii9135Obj * pObj );

Int32 Device_sii9135SetVideoMode ( Device_Sii9135Obj * pObj,
                                Device_VideoDecoderVideoModeParams * pPrm );

Int32 Device_sii9135Start ( Device_Sii9135Obj * pObj );

Int32 Device_sii9135Stop ( Device_Sii9135Obj * pObj );

Int32 Device_sii9135RegWrite ( Device_Sii9135Obj * pObj,
                            Device_VideoDecoderRegRdWrParams * pPrm );

Int32 Device_sii9135RegRead ( Device_Sii9135Obj * pObj,
                           Device_VideoDecoderRegRdWrParams * pPrm );

Int32 Device_sii9135LockObj ( Device_Sii9135Obj * pObj );
Int32 Device_sii9135UnlockObj ( Device_Sii9135Obj * pObj );
Int32 Device_sii9135Lock (  );
Int32 Device_sii9135Unlock (  );
Device_Sii9135Obj *Device_sii9135AllocObj (  );
Int32 Device_sii9135FreeObj ( Device_Sii9135Obj * pObj );

Int32 Device_sii9135SetupVideo ( Device_Sii9135Obj * pObj,
                              Device_Sii9135AviInfoFrame * pAviInfo,
                              Device_VideoDecoderVideoModeParams * pPrm );
Int32 Device_sii9135ReadAviInfo ( Device_Sii9135Obj * pObj,
                               Device_Sii9135AviInfoFrame * pAviInfo );
Int32 Device_sii9135DetectVideo ( Device_Sii9135Obj * pObj,
                               Device_Sii9135AviInfoFrame * pAviInfo,
                               UInt32 timeout );
Int32 Device_sii9135SelectTmdsCore ( Device_Sii9135Obj * pObj, UInt32 coreId );
Int32 Device_sii9135PowerDown ( Device_Sii9135Obj * pObj, UInt32 powerDown );
Int32 Device_sii9135OutputEnable ( Device_Sii9135Obj * pObj, UInt32 enable );

Int32 Device_sii9135ResetRegCache(Device_Sii9135Obj * pObj);


#endif /*  _DEVICE_SII9135_PRIV_H_  */
