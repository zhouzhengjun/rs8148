 /*******************************************************************************
 *                                                                             *
 * Copyright (c) 2009 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/
 
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/psp/vps/vps.h>
 
 /*
 * By default, when an exception occurs, an ExcContext structure is allocated on the ISR stack
 * and filled in within the exception handler. If excContextBuffer is initialized by the user,
 * the ExcContext structure will be placed at that address instead. The buffer must be large
 * enough to contain an ExcContext structure.
 *
 * By default, when an exception occurs, a pointer to the base address of the stack being used
 * by the thread causing the exception is used. If excStackBuffer is initialized by the user,
 * the stack contents of the thread causing the exception will be copied to that address instead.
 * The buffer must be large enough to contain the largest task stack or ISR stack defined in the application.
 *
 */
 
 Void Utils_exceptionHookFxn(Hwi_ExcContext *excCtx)
 {

	Vps_rprintf ("Unhandled Exception:");
	if (excCtx->threadType == BIOS_ThreadType_Hwi){
		Vps_rprintf ("Exception occurred in ThreadType_HWI");
	} else if (excCtx->threadType == BIOS_ThreadType_Swi){
		Vps_rprintf ("Exception occurred in ThreadType_SWI");
	} else if (excCtx->threadType == BIOS_ThreadType_Task){
		Vps_rprintf ("Exception occurred in ThreadType_Task");
	} else if (excCtx->threadType == BIOS_ThreadType_Main){
		Vps_rprintf ("Exception occurred in ThreadType_Main");
	}

	Vps_rprintf ("handle: 0x%x.\n", excCtx->threadHandle);
	Vps_rprintf ("stack base: 0x%x.\n", excCtx->threadStack);
	Vps_rprintf ("stack size: 0x%x.\n", excCtx->threadStackSize);

	Vps_rprintf ("R0 = 0x%08x  R8  = 0x%08x\n", excCtx->r0, excCtx->r8);
	Vps_rprintf ("R1 = 0x%08x  R9  = 0x%08x\n", excCtx->r1, excCtx->r9);
	Vps_rprintf ("R2 = 0x%08x  R10 = 0x%08x\n", excCtx->r2, excCtx->r10);
	Vps_rprintf ("R3 = 0x%08x  R11 = 0x%08x\n", excCtx->r3, excCtx->r11);
	Vps_rprintf ("R4 = 0x%08x  R12 = 0x%08x\n", excCtx->r4, excCtx->r12);
	Vps_rprintf ("R5 = 0x%08x  SP(R13) = 0x%08x\n", excCtx->r5, excCtx->sp);
	Vps_rprintf ("R6 = 0x%08x  LR(R14) = 0x%08x\n", excCtx->r6, excCtx->lr);
	Vps_rprintf ("R7 = 0x%08x  PC(R15) = 0x%08x\n", excCtx->r7, excCtx->pc);
	Vps_rprintf ("PSR = 0x%08x\n", excCtx->psr);
	Vps_rprintf ("ICSR = 0x%08x\n", Hwi_nvic.ICSR);
	Vps_rprintf ("MMFSR = 0x%02x\n", Hwi_nvic.MMFSR);
	Vps_rprintf ("BFSR = 0x%02x\n", Hwi_nvic.BFSR);
	Vps_rprintf ("UFSR = 0x%04x\n", Hwi_nvic.UFSR);
	Vps_rprintf ("HFSR = 0x%08x\n", Hwi_nvic.HFSR);
	Vps_rprintf ("DFSR = 0x%08x\n", Hwi_nvic.DFSR);
	Vps_rprintf ("MMAR = 0x%08x\n", Hwi_nvic.MMAR);
	Vps_rprintf ("BFAR = 0x%08x\n", Hwi_nvic.BFAR);
	Vps_rprintf ("AFSR = 0x%08x\n", Hwi_nvic.AFSR);
	Vps_rprintf ("Terminating Execution...");
}
