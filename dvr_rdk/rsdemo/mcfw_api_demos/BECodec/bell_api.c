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

#include <bell_api.h>
#include "bell_header.h"
#include "bell_common.h"
#include "bell_bits_rdwr.h"

#include <bell_osd.h>
#include <bell_swms.h>
#include <bell_blindArea.h>
#include "rsdemo/graphic/graphic.h"
#include "rsdemo/display_process/display_process.h"

typedef struct {
    UInt32 maxVencChannels;
    UInt32 maxVcapChannels;
    UInt32 maxVdisChannels;
    UInt32 maxVdecChannels;
    UInt32 VsysNumChs;
    bellType    Type;
    Bool        osdEnable;

    void        *ipcHdl;
    
} Bell_Info;

int Bell_apiBitsGet(void *h, VCODEC_BITSBUF_LIST_S *buflist)
{
    Bell_Info *hdl = (Bell_Info*)h;
    if(hdl == NULL || hdl->ipcHdl == NULL)
        return -1;

    return Bell_ipcBitsGet(hdl->ipcHdl, buflist);
}

int Bell_apiBitsPut(VCODEC_BITSBUF_LIST_S *buflist)
{
    return Bell_ipcBitsPut(buflist);    
}

VCODEC_BITSBUF_S* Bell_apiGetEmptyBuf(void *h, int size)
{
    Bell_Info *hdl = (Bell_Info*)h;
	if(hdl == NULL || hdl->ipcHdl == NULL)
        return NULL;
    return Bell_ipcGetEmptyBuf(hdl->ipcHdl, size);
}

int Bell_apiBitsFeed(void *h, VCODEC_BITSBUF_S *pFullBufInfo)
{
    Bell_Info *hdl = (Bell_Info*)h;
    if(hdl == NULL || hdl->ipcHdl == NULL)
        return -1;
    return Bell_ipcBitsFeed(hdl->ipcHdl, pFullBufInfo);
}

Void Bell_api_param_init(CodecPrm *prm)
{
    prm->usecase = BELL_USECASE_MAX;

    Venc_params_init((VENC_PARAMS_S*)(&prm->vencParams));
    Vdec_params_init((VDEC_PARAMS_S*)(&prm->vdecParams));
}

const BELL_VENC_PARAMS_S const* Bell_api_get_enc_param(Int32 vencChnId)
{
    EncLink_GetDynParams params = { 0 };

    params.chId = vencChnId;
    System_linkControl(gVencModuleContext.encId, ENC_LINK_CMD_GET_CODEC_PARAMS,
                       &params, sizeof(params), TRUE);
    
    gVencModuleContext.vencConfig.encChannelParams[vencChnId].dynamicParam.frameRate
        = params.targetFps/VENC_FRAMERATE_LINK_MULTIPLICATION_FACTOR;
    gVencModuleContext.vencConfig.encChannelParams[vencChnId].dynamicParam.intraFrameInterval
        = params.intraFrameInterval;
    gVencModuleContext.vencConfig.encChannelParams[vencChnId].dynamicParam.targetBitRate
        = params.targetBitRate;
    gVencModuleContext.vencConfig.encChannelParams[vencChnId].videoWidth = params.inputWidth;
    gVencModuleContext.vencConfig.encChannelParams[vencChnId].videoHeight = params.inputHeight;
    
    return (const BELL_VENC_PARAMS_S const*)&gVencModuleContext.vencConfig;
}

Int32 Bell_api_set_enc_dynamic_param(Int32 ChId, BELL_VENC_CHN_DYNAMIC_PARAM_S *dyn_prm, BELL_VENC_PARAM_E paramId)
{    
    if(ChId >= VENC_CHN_MAX || paramId >= VENC_ALL)
    {
        return OSA_EFAIL;
    }
    if(paramId != BELL_VENC_IN_FRAMERATE)
        Venc_setDynamicParam(ChId, 0, (VENC_CHN_DYNAMIC_PARAM_S*)dyn_prm, (VENC_PARAM_E)paramId);
    else
        Venc_setInputFrameRate(ChId, dyn_prm->inputFrameRate);
    
    return OSA_SOK;
}

void* Bell_api_create()
{
    Bell_Info *hdl;

    VSYS_PARAMS_S vsysParams;
    VCAP_PARAMS_S vcapParams;
    VDIS_PARAMS_S vdisParams;

    hdl = (Bell_Info*)malloc(sizeof(Bell_Info));
    if(hdl == NULL)
        return NULL;
    
    memset(hdl, 0, sizeof(Bell_Info));

    Vsys_params_init(&vsysParams);
    Vcap_params_init(&vcapParams);
    Vdis_params_init(&vdisParams);
    vsysParams.numChs  = 1;
    
    vsysParams.enableCapture = TRUE;
    vsysParams.enableEncode = TRUE;
    vsysParams.enableDecode = TRUE;
    vsysParams.enableOsd = FALSE;
    vsysParams.enableSecondaryOut   = FALSE;
    vsysParams.enableNsf            = TRUE;
    vsysParams.enableCapture        = TRUE;
    vsysParams.enableNullSrc        = FALSE;
    vsysParams.numDeis              = 0;
    vsysParams.numSwMs              = 0;
    vsysParams.numDisplays          = 0;

    /* Override the context here as needed */
    Vsys_init(&vsysParams);

    vcapParams.numChn = 1;

    /* Override the context here as needed */
    Vcap_init(&vcapParams);
    
    /* Override the context here as needed */
    
    vdisParams.numChannels = 1;
    vdisParams.enableLayoutGridDraw = FALSE;
    vdisParams.deviceParams[VDIS_DEV_HDMI].resolution   = VSYS_STD_1080P_60;
    Vdis_init(&vdisParams);

    /* Configure display in order to start grpx before video */
    Vsys_configureDisplay();

#if USE_FBDEV
    grpx_init(GRPX_FORMAT_RGB888);
#endif

    return hdl;
}

Int32 Bell_disp_layout(int winId0, int winId1, int numWin, VDIS_DEV dev)
{
    UInt32 winId, status;
    status = Vdis_getMosaicParams(dev, &gVdisModuleContext.vdisConfig.mosaicParams[dev]);

    if(status<0)
        return status;

    if(numWin == 1)
    {
        Bell_swMsGenerateLayout_for_switch(dev, 0, 4,
	                          LAYOUT_MODE_PIP_FAKE,
	                          &gVdisModuleContext.vdisConfig.mosaicParams[dev], FALSE,
	                          BELL_TYPE_PROGRESSIVE,
	                          gVdisModuleContext.vdisConfig.deviceParams[dev].resolution);
    }
    else
    {
        Bell_swMsGenerateLayout_for_switch(dev, 0, 4,
                          LAYOUT_MODE_PIP ,
                          &gVdisModuleContext.vdisConfig.mosaicParams[dev], FALSE,
                          BELL_TYPE_PROGRESSIVE,
                          gVdisModuleContext.vdisConfig.deviceParams[dev].resolution);
        gVdisModuleContext.vdisConfig.mosaicParams[dev].chnMap[0] = winId0;
        gVdisModuleContext.vdisConfig.mosaicParams[dev].chnMap[1] = winId1;
        for(winId=2;winId<VDIS_MOSAIC_WIN_MAX;winId++)
        {
            gVdisModuleContext.vdisConfig.mosaicParams[dev].chnMap[winId] = VDIS_CHN_INVALID;
        }
    }
    return 0;
}

Int32 Bell_api_disp_layout(int winId0, int winId1, int numWin)
{
    UInt32 status;

    status = Bell_disp_layout(winId0, winId1, numWin, VDIS_DEV_HDMI);
    if(status < 0)
        return status;
    
    status = Vdis_setMosaicParams(VDIS_DEV_HDMI, &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI]);
    return status;
}

Int32 DecErrHandler(UInt32 eventId, Ptr pPrm, Ptr appData)
{
    if(eventId==VSYS_EVENT_VIDEO_DETECT)
    {
        printf(" \n");
        printf(" DEMO: Received event VSYS_EVENT_VIDEO_DETECT [0x%04x]\n", eventId);
    }

    if(eventId==VSYS_EVENT_TAMPER_DETECT)
    {
    }

    if(eventId==VSYS_EVENT_MOTION_DETECT)
    {
    }

    if(eventId== VSYS_EVENT_DECODER_ERROR)
    {
        VDEC_CH_ERROR_MSG *msg = (VDEC_CH_ERROR_MSG*)pPrm;
        printf(" \n");
        printf(" DEMO: Received event VSYS_EVENT_DECODER_ERROR [0x%x]\n", msg->errorMsg);
    }

    return 0;
}

int Bell_api_start(void *h, CodecPrm *prm)
{
    Bell_Info *hdl = (Bell_Info*)h;

    VSYS_PARAMS_S vsysParams;
    VCAP_PARAMS_S vcapParams;
    VENC_PARAMS_S vencParams;
    VDEC_PARAMS_S vdecParams;

	Bell_res ipcbits_resArray[IPCBITS_RESOLUTION_TYPES];
	
    UInt8 osdFormat[ALG_LINK_OSD_MAX_CH];

	printf("%s: gpio20 down\n",__func__);
	system("echo 20 > /sys/class/gpio/export");
	system("echo out > /sys/class/gpio/gpio20/direction");
	system("echo 1 > /sys/class/gpio/gpio20/value");

	printf("%s: gpio25 down\n",__func__);
	system("echo 25 > /sys/class/gpio/export");
	system("echo out > /sys/class/gpio/gpio25/direction");
	system("echo 1 > /sys/class/gpio/gpio25/value");

    if(prm->usecase >= BELL_USECASE_MAX)
    {
        printf("Usecase indicator is not recognized!\n");
        return -1;
    }

    memset(osdFormat, SYSTEM_DF_YUV420SP_UV, ALG_LINK_OSD_MAX_CH);
    
    hdl->osdEnable = FALSE;
    
    hdl->Type = BELL_TYPE_PROGRESSIVE;

    hdl->maxVcapChannels = 1;
    hdl->maxVdisChannels = 4;
    hdl->maxVencChannels = 4;
    hdl->maxVdecChannels = 4;
	hdl->VsysNumChs  = 4;
	
	memcpy(&vencParams, &prm->vencParams, sizeof(VENC_PARAMS_S));

    ipcbits_resArray[0].width = vencParams.encChannelParams[0].videoWidth;
	ipcbits_resArray[0].height = vencParams.encChannelParams[0].videoHeight;
    ipcbits_resArray[1].width = 0;
	ipcbits_resArray[1].height = 0;

	Venc_init(&vencParams);

    memcpy(&vdecParams, &prm->vdecParams, sizeof(VDEC_PARAMS_S));
    
	Vdec_init(&vdecParams);

	switch (prm->usecase) {
	case BELL_USECASE_IPNCDEMO:
		 Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
		                          LAYOUT_MODE_4CH,
		                          &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
		                          hdl->Type,
		                          gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
	case BELL_USECASE_BELL_BASIC:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
                          LAYOUT_MODE_PIP,
                          &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
                          hdl->Type,
                          gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
    case BELL_USECASE_MODE5_0:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
		                          LAYOUT_MODE_PIP,
		                          &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
		                          hdl->Type,
		                          gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
        Bell_disp_layout(0, 1, 2, VDIS_DEV_HDMI);
		break;
	case BELL_USECASE_MODE5_1:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
		                          LAYOUT_MODE_PIP,
		                          &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
		                          hdl->Type,
		                          gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
        Bell_disp_layout(1, 0, 2, VDIS_DEV_HDMI);
		break;
	case BELL_USECASE_BELL_BASIC_HDMI_IN:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
		                          LAYOUT_MODE_PIP2,
		                          &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
		                          hdl->Type,
		                          gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
	case BELL_USECASE_PREVIEW:
	case BELL_USECASE_MODE0:
	case BELL_USECASE_MODE6:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
								  LAYOUT_MODE_1CH,
								  &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
								  hdl->Type,
								  gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
	case BELL_USECASE_MODE1:
	case BELL_USECASE_MODE2:
	case BELL_USECASE_MODE7:
	case BELL_USECASE_MODE8:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
								  LAYOUT_MODE_PIP,
								  &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
								  hdl->Type,
								  gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
	case BELL_USECASE_MODE3:
	case BELL_USECASE_MODE4:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
								  LAYOUT_MODE_PIP2,
								  &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
								  hdl->Type,
								  gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
/*	case BELL_USECASE_MODE5:
		Bell_swMsGenerateLayout(VDIS_DEV_HDMI, 0, hdl->maxVdisChannels,
								  LAYOUT_MODE_PIP2_MAIN2,
								  &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], FALSE,
								  hdl->Type,
								  gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution);
		break;
*/
	}
	
    gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI].userSetDefaultSWMLayout = TRUE;
    Bell_swMsSetOutputFPS(&gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI], Bell_swMsGetOutputFPS(&gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI])*2);
    Vdis_setMosaicParams(VDIS_DEV_HDMI, &gVdisModuleContext.vdisConfig.mosaicParams[VDIS_DEV_HDMI]);
    
    hdl->ipcHdl = Bell_ipcBitsInit(ipcbits_resArray);

    /* Create Link instances and connects compoent blocks */
    bell_create_path(prm);

    if(vsysParams.enableOsd)
    {
	    Int32 chId = 0;
        hdl->osdEnable = TRUE;

        /* Create and initialize OSD window buffers */
        Bell_osdInit(hdl->maxVencChannels, osdFormat);

        for(chId = 0; chId < hdl->maxVencChannels; chId++)
        {
            vcapParams.channelParams[chId].dynamicParams.osdChWinPrm = &g_osdChParam[chId];
            /* Initailize osdLink with created and set win params */
            Vcap_setDynamicParamChn(chId, &vcapParams.channelParams[chId].dynamicParams, VCAP_OSDWINPRM);

            vcapParams.channelParams[chId].dynamicParams.osdChBlindWinPrm = &g_osdChBlindParam[chId];
            /* Initailize osdLink with created and set win params */
            Vcap_setDynamicParamChn(chId, &vcapParams.channelParams[chId].dynamicParams, VCAP_OSDBLINDWINPRM);

        }
    }
	Bell_blindAreaInit(hdl->maxVcapChannels);

    Vsys_registerEventHandler(DecErrHandler, NULL);

    /* Start components in reverse order */
    Vdis_start();
	Venc_start();
	Vdec_start();
    Vcap_start();
	System_linkStart(SYSTEM_LINK_ID_CAMERA);

    return 0;
}

int Bell_api_force_idr()
{
    return Venc_forceIDR(0, 0);
}

int Bell_api_stop(void *h)
{    
    Bell_Info *hdl = (Bell_Info*)h;
    if(hdl == NULL || hdl->ipcHdl == NULL)
        return -1;
    
	printf("%s(): gpio20 down\n",__FUNCTION__);
	system("echo out > /sys/class/gpio/gpio20/direction");
 	system("echo 1 > /sys/class/gpio/gpio20/value");
	sleep(1);
    /* Stop components */
	System_linkStop(SYSTEM_LINK_ID_CAMERA);
    Vcap_stop();
	Vdec_stop();
	Venc_stop();
    Vdis_stop();

    
    if(hdl->osdEnable == TRUE)
        Bell_osdDeinit();

    bell_delete_path();

    Bell_ipcBitsExit();
    hdl->ipcHdl = NULL;
    
    return 0;
}

Void Bell_api_delete(void *h)
{
    
#if USE_FBDEV
    grpx_exit();
#endif
	Bell_api_stop(h);
	Vsys_deConfigureDisplay();

    /* De-initialize components */
    Vcap_exit();
    Vdis_exit();    
	Venc_exit();
	Vdec_exit();
    Vsys_exit();

    free(h);
}
