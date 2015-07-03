/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

#include "system_priv_m3vpss.h"

#include <mcfw/src_bios6/alg/simcop/inc/cpisCore.h>
#include <mcfw/src_bios6/alg/simcop/inc/iss_init.h>


static CPIS_Init gSystem_cpisInitPrm;


static System_simcopCacheWbInv(void *addr, Uint32 size, Bool wait)
{
    Cache_wbInv(addr, size, TRUE, NULL);
}

Int32 System_initSimcop()
{
    Int32 status;

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : SIMCOP Init in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

    memset(&gSystem_cpisInitPrm, 0, sizeof(gSystem_cpisInitPrm));

    gSystem_cpisInitPrm.cacheWbInv          = System_simcopCacheWbInv;
    gSystem_cpisInitPrm.staticDmaAlloc      = 1;
    gSystem_cpisInitPrm.maxNumDma           = 1;
    gSystem_cpisInitPrm.maxNumProcFunc      = 1;
    gSystem_cpisInitPrm.lock                = NULL;
    gSystem_cpisInitPrm.unlock              = NULL;
    gSystem_cpisInitPrm.initFC              = CPIS_INIT_FC_ALL;
    gSystem_cpisInitPrm.engineName          = "alg_server";
    gSystem_cpisInitPrm.codecEngineHandle   = NULL;

    gSystem_cpisInitPrm.memSize = CPIS_getMemSize(gSystem_cpisInitPrm.maxNumProcFunc);

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : SIMCOP needs %d B of memory !!!\n",
               Utils_getCurTimeInMsec(),
               gSystem_cpisInitPrm.memSize
                );
#endif

    gSystem_cpisInitPrm.mem = (void *)malloc(gSystem_cpisInitPrm.memSize);
    if(gSystem_cpisInitPrm.mem == NULL)
       return FVID2_EFAIL;

    /* Initialize CPIS */
    status = CPIS_init(&gSystem_cpisInitPrm);

    UTILS_assert(status==FVID2_SOK);

    {
        extern Task_Handle VICP_IP_RUN_hTsk;

        UTILS_assert(VICP_IP_RUN_hTsk!=NULL);

        status = Utils_prfLoadRegister( VICP_IP_RUN_hTsk, "SIMCOP");
        UTILS_assert(status==FVID2_SOK);
    }

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : SIMCOP Init in progress DONE !!!\n",
               Utils_getCurTimeInMsec());
#endif

    return status;
}

Int32 System_deInitSimcop()
{
#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : SIMCOP De-Init in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

	/* CPIS De Init */
	CPIS_deInit();

    if(gSystem_cpisInitPrm.mem!=NULL)
        free(gSystem_cpisInitPrm.mem);

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : SIMCOP De-Init in progress DONE !!!\n",
               Utils_getCurTimeInMsec());
#endif

    return FVID2_SOK;
}

Int32 System_initIss()
{
   Int32 status = FVID2_SOK;

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS Init in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS Power-ON in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif

    /* Power ON Iss */
    *(volatile UInt32*)0x48180D00 = 0x2; /* PM_ISP_PWRSTCTRL     */
    *(volatile UInt32*)0x48180D10 = 0x3; /* RM_ISP_RSTCTRL       */
    *(volatile UInt32*)0x48180700 = 0x2; /* CM_ISP_CLKSTCTRL     */
    *(volatile UInt32*)0x48180720 = 0x2; /* CM_ISP_ISP_CLKCTRL   */
    *(volatile UInt32*)0x48180724 = 0x2; /* CM_ISP_FDIF_CLKCTRL  */
    Task_sleep(10);

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS Power-ON in progress DONE !!!\n",
               Utils_getCurTimeInMsec());
#endif

    status = Iss_init(NULL);
    UTILS_assert(status == 0);

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS Init in progress DONE !!!\n",
               Utils_getCurTimeInMsec());
#endif

    System_initSimcop();

   return status;
}


Int32 System_deInitIss()
{
   Int32 status = FVID2_SOK;

    System_deInitSimcop();

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS De-Init in progress !!!\n",
               Utils_getCurTimeInMsec());
#endif



    status = Iss_deInit(NULL);
    UTILS_assert(status == 0);

#ifdef SYSTEM_DEBUG
    Vps_printf(" %d: SYSTEM  : ISS De-Init in progress DONE !!!\n",
               Utils_getCurTimeInMsec());
#endif


   return status;
}


