/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/


#include "ti_media_std.h"
#include <device.h>
#include <device_videoDecoder.h>
//#include <device_tvp5158.h>  not used. mark
#include <sii9135_priv.h>

/* Global object storing all information related to all
  SII9135 driver handles */

/* Control API that gets called when Device_control is called

  This API does handle level semaphore locking

  handle - SII9135 driver handle
  cmd - command
  cmdArgs - command arguments
  cmdStatusArgs - command status

  returns error in case of
  - illegal parameters
  - I2C command RX/TX error
*/
Int32 Device_sii9135Control ( Device_Sii9135Handle handle,
                           UInt32 cmd, Ptr cmdArgs, Ptr cmdStatusArgs )
{
    Device_Sii9135Obj *pObj = ( Device_Sii9135Obj * ) handle;
    Int32 status;

    if ( pObj == NULL )
        return -1;

    switch ( cmd )
    {
        case DEVICE_CMD_START:
            status = Device_sii9135Start ( pObj );
            break;

        case DEVICE_CMD_STOP:
            status = Device_sii9135Stop ( pObj );
            break;

        case IOCTL_DEVICE_VIDEO_DECODER_GET_CHIP_ID:
            status = Device_sii9135GetChipId ( pObj, cmdArgs, cmdStatusArgs );
            break;

        case IOCTL_DEVICE_VIDEO_DECODER_RESET:
            status = Device_sii9135Reset ( pObj );
            break;

        case IOCTL_DEVICE_VIDEO_DECODER_SET_VIDEO_MODE:
            status = Device_sii9135SetVideoMode ( pObj, cmdArgs );
            break;

        case IOCTL_DEVICE_VIDEO_DECODER_GET_VIDEO_STATUS:
            status = Device_sii9135GetVideoStatus ( pObj, cmdArgs, cmdStatusArgs );
            break;

   //     case IOCTL_DEVICE_VIDEO_DECODER_REG_WRITE:
    //        status = Device_sii9135RegWrite ( pObj, cmdArgs );
     //       break;

      //  case IOCTL_DEVICE_VIDEO_DECODER_REG_READ:
      //      status = Device_sii9135RegRead ( pObj, cmdArgs );
       //     break;
			

        default:
            status = -1;
            break;
    }

    return status;
}

/*
  Create API that gets called when Device_create is called

  This API does not configure the SII9135 is any way.

  This API
  - validates parameters
  - allocates driver handle
  - stores create arguments in its internal data structure.

  Later the create arguments will be used when doing I2C communcation with
  SII9135

  drvId - driver ID, must be DEVICE_VID_DEC_SII9135_DRV
  instanceId - must be 0
  createArgs - create arguments
  createStatusArgs - create status
  fdmCbParams  - NOT USED, set to NULL

  returns NULL in case of any error
*/
Device_Sii9135Handle Device_sii9135Create ( UInt8 drvId,
                                            UInt8 instId,
                                            Ptr createArgs,
                                            Ptr createStatusArgs)
{
    Int32 status = 0;
    Device_Sii9135Obj *pObj;
    Device_VideoDecoderCreateParams *vidDecCreateArgs
        = ( Device_VideoDecoderCreateParams * ) createArgs;

    Device_VideoDecoderCreateStatus *vidDecCreateStatus
        = ( Device_VideoDecoderCreateStatus * ) createStatusArgs;

    /*
     * check parameters
     */
    if ( vidDecCreateStatus == NULL )
        return NULL;

    vidDecCreateStatus->retVal = 0;

    if ( vidDecCreateArgs->deviceI2cInstId >= DEVICE_I2C_INST_ID_MAX)
    {
        vidDecCreateStatus->retVal = -1;
        return NULL;
    }

    pObj = (Device_Sii9135Obj *)malloc(sizeof(Device_Sii9135Obj));

    if ( pObj == NULL )
    {
        vidDecCreateStatus->retVal = -1;
        return NULL;
    }
    else
    {
        memset(pObj, 0, sizeof(Device_Sii9135Obj));

        gDevice_sii9135CommonObj.sii9135handle[instId] = pObj;

        /*
         * copy parameters to allocate driver handle
         */
        memcpy ( &pObj->createArgs, vidDecCreateArgs,
                 sizeof ( *vidDecCreateArgs ) );

        status = OSA_i2cOpen(&(gDevice_sii9135CommonObj.i2cHandle), I2C_DEFAULT_INST_ID);

        if ( status < 0)
        {
            vidDecCreateStatus->retVal = -1;
            return NULL;
        }
		Device_sii9135Reset (pObj);
		//Device_sii9135ResetRegCache(pObj);
    }
    /*
     * return driver handle object pointer
     */
    return pObj;
}

/*
  Delete function that is called when Device_delete is called

  This API
  - free's driver handle object

  handle - driver handle
  deleteArgs - NOT USED, set to NULL

*/
Int32 Device_sii9135Delete ( Device_Sii9135Handle handle, Ptr deleteArgs )
{
    Device_Sii9135Obj *pObj = ( Device_Sii9135Obj * ) handle;

    if ( pObj == NULL )
        return -1;


    OSA_i2cClose(&(gDevice_sii9135CommonObj.i2cHandle));


    /*
     * free driver handle object
     */
    free(pObj);

    return 0;
}

/*
  System init for SII9135 driver

  This API
  - create semaphore locks needed
  - registers driver to FVID2 sub-system
  - gets called as part of Device_deviceInit()

*/
Int32 Device_sii9135Init (  )
{
    /*
     * Set to 0's for global object, descriptor memory
     */
    memset ( &gDevice_sii9135CommonObj, 0, sizeof ( gDevice_sii9135CommonObj ) );

    return 0;
}

Int32 Device_sii9135DeInit (  )
{
    /*
     * Set to 0's for global object, descriptor memory
     */
    return 0;
}

