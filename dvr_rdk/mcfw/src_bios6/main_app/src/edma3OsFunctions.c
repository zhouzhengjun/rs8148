#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sdo/edma3/drv/edma3_drv.h>

 /**
  * \brief   EDMA3 OS Semaphore Create
  *
  *      This function creates a counting semaphore with specified
  *      attributes and initial value. It should be used to create a semaphore
  *      with initial value as '1'. The semaphore is then passed by the user
  *      to the EDMA3 driver/RM for proper sharing of resources.
  * \param   initVal [IN] is initial value for semaphore
  * \param   semParams [IN] is the semaphore attributes.
  * \param   hSem [OUT] is location to receive the handle to just created
  *      semaphore.
  * \return  EDMA3_DRV_SOK if successful, else a suitable error code.
  */
 EDMA3_DRV_Result Utils_edma3OsSemCreate(int initVal,
                             const Semaphore_Params *semParams,
                             EDMA3_OS_Sem_Handle *hSem);



 /**
  * \brief   EDMA3 OS Semaphore Delete
  *
  *      This function deletes or removes the specified semaphore
  *      from the system. Associated dynamically allocated memory
  *      if any is also freed up.
  * \param   hSem [IN] handle to the semaphore to be deleted
  * \return  EDMA3_DRV_SOK if successful else a suitable error code
  */
 EDMA3_DRV_Result Utils_edma3OsSemDelete(EDMA3_OS_Sem_Handle hSem);
/*
 * EDMA3 critical section functions
 * Sample functions showing the implementation of critical section entry/exit
 * routines and various semaphore related routines (all OS depenedent). These
 * implementations MUST be provided by the user / application, using the EDMA3
 * Resource Manager, for its correct functioning.
 * */

//static unsigned int region_id;

void edma3OsSetRegionId(int id) {
//    region_id= id;
    return;
}


#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Task.h>

extern unsigned int VICP_EDMA3_ccXferCompInt[][EDMA3_MAX_REGIONS];
extern unsigned int VICP_EDMA3_ccErrorInt[];
extern unsigned int VICP_EDMA3_tcErrorInt[][EDMA3_MAX_TC];

/**
 * Shadow Region on which the executable is running. Its value is
 * populated with the DSP Instance Number here in this case.
 */

void edma3OsSemaphore_Params_init(Semaphore_Params *semParams){
    Semaphore_Params_init(semParams);
}

/**
 * \brief   EDMA3 OS Semaphore Create
 *
 *      This function creates a counting semaphore with specified
 *      attributes and initial value. It should be used to create a semaphore
 *      with initial value as '1'. The semaphore is then passed by the user
 *      to the EDMA3 driver/RM for proper sharing of resources.
 * \param   initVal [IN] is initial value for semaphore
 * \param   semParams [IN] is the semaphore attributes.
 * \param   hSem [OUT] is location to recieve the handle to just created
 *      semaphore
 * \return  EDMA3_DRV_SOK if succesful, else a suitable error code.
 */
EDMA3_DRV_Result edma3OsSemCreate(int initVal,
							const Semaphore_Params *semParams,
                           	EDMA3_OS_Sem_Handle *hSem)
    {
    

    return Utils_edma3OsSemCreate(initVal, semParams, hSem);
    }


/**
 * \brief   EDMA3 OS Semaphore Delete
 *
 *      This function deletes or removes the specified semaphore
 *      from the system. Associated dynamically allocated memory
 *      if any is also freed up.
 * \param   hSem [IN] handle to the semaphore to be deleted
 * \return  EDMA3_DRV_SOK if succesful else a suitable error code
 */
EDMA3_DRV_Result edma3OsSemDelete(EDMA3_OS_Sem_Handle hSem)
    {
    return Utils_edma3OsSemDelete(hSem);
    }






