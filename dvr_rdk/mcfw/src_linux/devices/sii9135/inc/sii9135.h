/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef _DEVICEDRV_SII9135_H_
#define _DEVICEDRV_SII9135_H_
#if 0
Int32 Device_tvp5158Init (  );
Int32 Device_tvp5158DeInit (  );

Device_Tvp5158Handle Device_tvp5158Create ( UInt32 drvId,
                                UInt32 instanceId,
                                Ptr createArgs,
                                Ptr createStatusArgs);

Int32 Device_tvp5158Delete ( Device_Tvp5158Handle handle, Ptr deleteArgs );

Int32 Device_tvp5158Control ( Device_Tvp5158Handle handle,
                           UInt32 cmd, Ptr cmdArgs, Ptr cmdStatusArgs );
#endif
Int32 Device_sii9135Control ( Device_Sii9135Handle handle,
                           UInt32 cmd, Ptr cmdArgs, Ptr cmdStatusArgs );
Device_Sii9135Handle Device_sii9135Create ( UInt8 drvId,
                                            UInt8 instId,
                                            Ptr createArgs,
                                            Ptr createStatusArgs);
Int32 Device_sii9135Delete ( Device_Sii9135Handle handle, Ptr deleteArgs );

Int32 Device_sii9135Init (  );
Int32 Device_sii9135DeInit (  );

#endif
