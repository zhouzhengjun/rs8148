#include "videoServer.h"
#include "jsonc/json.h"
#include "bell_api.h"
#include "ti_vsys.h"
#include <osa_thr.h>
#include <signal.h>
void *global_hdl;

typedef struct param_enc_t{
    int width;
    int height;
    int fps;
    int br;
    int pkt_mode;
    int profile;
    int level;
    int max_fps;
    int max_mbps;
    int max_smbps;
    int max_fs;
    int max_br;
}param_enc_t;

typedef struct vs_hdl_t
{
    void    *bellHdl;
    bellloop_t *get_loopbuf;
    bellloop_t *send_loopbuf;
    
    OSA_ThrHndl send_task_hdl;
    OSA_ThrHndl get_task_hdl;
    bell_zmq_repack_t *server;
    
    int stop_get_task;
    int stop_send_task;
    
    param_enc_t encp;
}vs_hdl_t;

int videoServer_printInfo()
{
    Vsys_printDetailedStatistics();
    Bell_printStatistics(TRUE, TRUE);
    return 0;
}

int level_preset[11][6] = 
{
    {11, 3000,      396,    377500, 352, 288},//level 1.1
    {12, 6000,      396,    891000, 352, 288},//level 1.2
    {13, 11880,     396,    891000, 352, 288},//level 1.3
    {20, 11880,     396,    891000, 352, 288},//level 2.0
    {21, 19800,     792,    1782000, 352, 288},//level 2.1
    {22, 20250,     1620,   3037500, 640, 480},//level 2.2
    {30, 40500,     1620,   3037500, 704, 576},//level 3.0
    {31, 108000,    5120,   6750000, 1280, 720},//level 3.1
    {32, 216000,    8192,   7680000, 1920, 1080},//level 3.2
    {40, 245760,    8192,   12288000, 1920, 1080},//level 4.0
    {41, 245760,    8192,   12288000, 1920, 1080},//level 4.1
};

Void setEncParams(BELL_VENC_PARAMS_S *pVencParams, param_enc_t *encp)
{
    Int32 i;	
    int max_mbps = 0;
    int max_smbps = 0;
    int max_fs = 0;
    int max_br = 2*1024*1024;
    Venc_params_init(pVencParams);
	
	for (i=0; i < BELL_VENC_PRIMARY_CHANNELS; i++)
	{
		pVencParams->h264Profile[i] = BELL_VENC_CHN_BASELINE_PROFILE;
	}
	pVencParams->encChannelParams[0].enableAnalyticinfo = 0;	
	pVencParams->encChannelParams[0].numTemporalLayer = BELL_VENC_TEMPORAL_LAYERS_1;
	pVencParams->encChannelParams[0].enableSVCExtensionFlag = BELL_VENC_IH264_SVC_EXTENSION_FLAG_DISABLE;
	pVencParams->encChannelParams[0].enableAnalyticinfo = 0;
    pVencParams->encChannelParams[0].enableWaterMarking = 0;
    pVencParams->encChannelParams[0].rcType = BELL_VENC_RATE_CTRL_CBR;
    pVencParams->encChannelParams[0].dynamicParam.rcAlg = BELL_VENC_RATE_CTRL_CBR;
	pVencParams->encChannelParams[0].dynamicParam.targetBitRate = 2 * 1000 * 1000;
    pVencParams->numPrimaryChn = 1;

	i = 0;
    if(encp->profile != 0)
    {
        pVencParams->h264Profile[0] = encp->profile;
    }
    if(encp->level != 0)
    {
        for(i = 0; i < 11; i++)
        {
            if(encp->level == level_preset[i][0])
            {
                max_mbps = level_preset[i][1];
                max_smbps = level_preset[i][1];
                max_fs   = level_preset[i][2];
                max_br   = level_preset[i][3];
                break;
            }
        }
    }

    if(i == 0 || i == 11)
    {//default use level 3.1
        i = 8;
        max_mbps = level_preset[i][1];
        max_smbps = level_preset[i][1];
        max_fs   = level_preset[i][2];
        max_br   = level_preset[i][3];
    }

    max_fs = (encp->max_fs != 0 ? encp->max_fs : max_fs);
    max_mbps = (encp->max_mbps != 0 ? encp->max_mbps : max_mbps);
    max_smbps = (encp->max_smbps != 0 ? encp->max_smbps : max_smbps);

    while(i < 10 && max_mbps > level_preset[i][1])
    {
        i++;
    }
    while(i < 10 && max_smbps > level_preset[i][1])
    {
        i++;
    }
    while(i < 10 && max_fs > level_preset[i][2])
    {
        i++;
    }
	pVencParams->encChannelParams[0].videoWidth = (encp->width == 0 ? 1280 : encp->width);
	pVencParams->encChannelParams[0].videoHeight= (encp->height == 0 ? 720 : encp->height);

	if(	(pVencParams->encChannelParams[0].videoWidth > level_preset[i][4]) 
		|| (pVencParams->encChannelParams[0].videoHeight > level_preset[i][5]))
	{
		pVencParams->encChannelParams[0].videoWidth = level_preset[i][4];
        pVencParams->encChannelParams[0].videoHeight = level_preset[i][5];
	}
	encp->width = pVencParams->encChannelParams[0].videoWidth;
	encp->height = pVencParams->encChannelParams[0].videoHeight;

    if(encp->fps != 0)
    {
        if(encp->max_fps > 0 && encp->max_fps < encp->fps)
            encp->fps = encp->max_fps;
        pVencParams->encChannelParams[0].dynamicParam.frameRate = encp->fps;
    }

    max_br = (encp->max_br != 0 ? encp->max_br : max_br);
    encp->max_br = max_br;
    encp->max_fs = max_fs;
    encp->max_mbps = max_mbps;
    encp->max_smbps = max_smbps;

    printf("Change resolution to %dx%d \n", encp->width, encp->height);

    if(encp->max_br != 0)
        pVencParams->encChannelParams[0].maxBitRate= encp->max_br;

    if(pVencParams->encChannelParams[0].maxBitRate < 0)
        pVencParams->encChannelParams[0].maxBitRate = encp->br + (encp->br/5);

    if(encp->br != 0)
    {
        pVencParams->encChannelParams[0].dynamicParam.targetBitRate = encp->br;
        if(pVencParams->encChannelParams[0].maxBitRate < encp->br)
            pVencParams->encChannelParams[0].dynamicParam.targetBitRate = pVencParams->encChannelParams[0].maxBitRate;
    }
    
    pVencParams->encChannelParams[0].dynamicParam.intraFrameInterval = 200;
}

Void setDecParams(BELL_VDEC_PARAMS_S *pVdecParams)
{
    Vdec_params_init(pVdecParams);

    pVdecParams->decChannelParams[0].maxVideoWidth = 1920;
    pVdecParams->decChannelParams[0].maxVideoHeight = 1080;
    pVdecParams->decChannelParams[0].isCodec = BELL_VDEC_CHN_H264;
    //pVdecParams->decChannelParams[0].numBufPerCh = 5;
    //pVdecParams->decChannelParams[0].displayDelay = 5;
    //pVdecParams->decChannelParams[0].dynamicParam.frameRate = 30;
    //pVdecParams->decChannelParams[0].dynamicParam.targetBitRate = 2 * 1000 * 1000;
    //pVdecParams->forceUseDecChannelParams = TRUE;
    pVdecParams->numChn = 1;
}

void* get_stream(void *hdl)
{
    VCODEC_BITSBUF_S *loop_buf;
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    bellloop_t *get_loopbuf;
    void *bell_hdl;
    if(h == NULL)
        return NULL;
    
    get_loopbuf = h->get_loopbuf;
    bell_hdl = h->bellHdl;
    
    while(!h->stop_get_task)
    {
        bellbuf_t *buf = get_buf(get_loopbuf, "DEC");
        if(buf == NULL)
        {
            OSA_waitMsecs(5);
            continue;
        }
        loop_buf = Bell_apiGetEmptyBuf(bell_hdl, buf->buf_size);
        if(loop_buf)
        {
            loop_buf->filledBufSize = buf->buf_size;
            loop_buf->chnId = buf->chn_id;
            loop_buf->codecType = buf->codec_type;
            memcpy(loop_buf->bufVirtAddr, buf->buf, buf->buf_size);
            Bell_apiBitsFeed(bell_hdl, loop_buf);
        }
        clean_buf(get_loopbuf,buf);
    }
    return NULL;
}

void* send_stream(void *hdl)
{
    VCODEC_BITSBUF_LIST_S buflist;    
    int i = 0;    
    bellloop_t *send_loopbuf;
    void *bell_hdl;
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    if(h == NULL)
        return NULL;
    
    send_loopbuf = h->send_loopbuf;
    bell_hdl = h->bellHdl;

    while(!h->stop_send_task)
    {
        int ret = 0;
        if(Bell_apiBitsGet(bell_hdl, &buflist) < 0)
        {
            OSA_waitMsecs(5);
            continue;
        }
        if(buflist.numBufs <= VCODEC_BITSBUF_MAX)
        {
            for(i = 0; i < buflist.numBufs; i++)
            {
                VCODEC_BITSBUF_S *recv_buf = &buflist.bitsBuf[i];
                if(recv_buf != NULL)
                {
                    bellbuf_t *buf = request_buf(send_loopbuf, recv_buf->filledBufSize, "ENC");
                    while(buf == NULL && !h->stop_send_task)
                    {
                        OSA_waitMsecs(5);
                        buf = request_buf(send_loopbuf, recv_buf->filledBufSize, "ENC");
                    }
                    if(buf)
                    {
                        memset(buf->buf, 0, recv_buf->filledBufSize);
                        memcpy(buf->buf, recv_buf->bufVirtAddr, recv_buf->filledBufSize);
                        buf->chn_id = recv_buf->chnId;
                        buf->codec_type = recv_buf->codecType;
                        ret = set_buf(send_loopbuf, buf);
                        if(ret < 0)
                        {
                            printf("write loop buf error!\n");
                        }
                    }
                }
            }
        }
        Bell_apiBitsPut(&buflist);
    }
    return NULL;
}

void videoServer_delete(void *hdl)
{   
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    if(h == NULL)
        return;
    if(h->server != NULL)
    {
        if(h->server->socket)
        {
            zmq_unbind(h->server->socket, IPC_BELL_VIDEOSERVER);
            zmq_close(h->server->socket);
        }

        if(h->server->ctx)
            zmq_term(h->server->ctx);

        free(h->server);
        h->server = NULL;
    }

    mmap_delete(h->get_loopbuf);
    mmap_delete(h->send_loopbuf);

    if(h->bellHdl)
        Bell_api_delete(h->bellHdl);

    free(h);
}

void* videoServer_create()
{
    vs_hdl_t *h = (vs_hdl_t*)malloc(sizeof(vs_hdl_t));
    if(h == NULL)
        return NULL;
    else
    {
        memset(h, 0, sizeof(vs_hdl_t));
        h->get_loopbuf = mmap_create("bell_stream_input", GET_BUF_SIZE, 1, 3);
        if(h->get_loopbuf == NULL)
        {
            videoServer_delete(h);
            return NULL;
        }
        
        *h->get_loopbuf->rd_offset = 0;
        *h->get_loopbuf->wt_offset = 0;
        memset(h->get_loopbuf->start_addr, 0, h->get_loopbuf->size);

        h->send_loopbuf = mmap_create("bell_stream_output", SEND_BUF_SIZE, 1, 9);
        if(h->send_loopbuf == NULL)
        {
            videoServer_delete(h);
            return NULL;
        }
        h->bellHdl = Bell_api_create();
        printf("Creating .... \n");
        if(h->bellHdl == NULL)
        {
            videoServer_delete(h);
            return NULL;
        }

        h->server = (bell_zmq_repack_t*)malloc(sizeof(bell_zmq_repack_t));
        if(h->server == NULL)
        {
            videoServer_delete(h);
            return NULL;
        }
        
        h->server->ctx = zmq_ctx_new();
        h->server->socket = zmq_socket(h->server->ctx,ZMQ_PULL);
        zmq_bind(h->server->socket, IPC_BELL_VIDEOSERVER);
        h->stop_send_task = h->stop_get_task = 1;

        printf("Creat Done \n");
    }
    return h;
}

int videoServer_start(void *hdl, BELL_USECASES_E usecase)
{
    CodecPrm prm;
    void *bellHdl;
    bellloop_t *send_loopbuf;
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    if(h == NULL)
        return -1;
    
    send_loopbuf = h->send_loopbuf;
    bellHdl = h->bellHdl;

    *send_loopbuf->rd_offset = 0;
    *send_loopbuf->wt_offset = 0;
    memset(send_loopbuf->start_addr, 0, send_loopbuf->size);

    Bell_api_param_init(&prm);
    
    prm.usecase = usecase;
    setEncParams(&prm.vencParams, &h->encp);
    setDecParams(&prm.vdecParams);

    h->stop_send_task = h->stop_get_task = 1;

    if(usecase != BELL_USECASE_PREVIEW && usecase != BELL_USECASE_MODE6)
    {
        h->stop_send_task = 0;
        OSA_thrCreate(&h->send_task_hdl,
                      send_stream,
                      2,
                      0,
                      h);

        Bell_api_start(bellHdl, &prm);

        if(usecase != BELL_USECASE_BELLLITE)
        {
            h->stop_get_task = 0;
            OSA_thrCreate(&h->get_task_hdl,
                          get_stream,
                          2,
                          0,
                          h);
        }
    }
    else
    {
        Bell_api_start(bellHdl, &prm);
    }

    printf("Start Done \n");
    
    return 0;
}

int videoServer_stop(void *hdl)
{
    void *bellHdl;
    bellloop_t *get_loopbuf;    
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    if(h == NULL)
        return -1;
    
    get_loopbuf = h->get_loopbuf;
    bellHdl = h->bellHdl;


	 if(h->stop_send_task == 0)
    {
        h->stop_send_task = 1;
        OSA_thrJoin(&h->send_task_hdl);
    }
    if(h->stop_get_task == 0)
    {
        h->stop_get_task = 1;
        OSA_thrJoin(&h->get_task_hdl);
    }

    *get_loopbuf->rd_offset = 0;
    *get_loopbuf->wt_offset = 0;
    memset(get_loopbuf->start_addr, 0, get_loopbuf->size);

    bellHdl = h->bellHdl;

    Bell_api_stop(bellHdl);
    
   

    memset(&h->encp, 0, sizeof(param_enc_t));
    return 0;
}

#define DEMO_CORE_STATUS_CHECK_TIMEOUT 1000

typedef enum 
{
    CMD_NONE,
    CMD_INFO,
    CMD_STATUS,
    CMD_SWITCH_LAYOUT,
    CMD_SET_PREVIEW,
    CMD_SET_IPNC,
    CMD_SET_BELLLITE,
    CMD_SET_BELL_BASIC,
    CMD_SET_BELL_MODE0,
    CMD_SET_BELL_MODE1,
    CMD_SET_BELL_MODE2,
    CMD_SET_BELL_MODE3,
    CMD_SET_BELL_MODE4,
    CMD_SET_BELL_MODE5_0,
    CMD_SET_BELL_MODE5_1,
    CMD_SET_BELL_MODE6,
    CMD_SET_BELL_MODE7,
    CMD_SET_BELL_MODE8,
    CMD_SET_BASIC_HDMI_IN,
    CMD_STOP,
    CMD_START,
    CMD_EXIT,
}cmd_e;

cmd_e get_msg(void *hdl)
{
    void *msg_data = NULL;
    zmq_msg_t msg;
    cmd_e ret = CMD_NONE;
    json_object *json = NULL;
    vs_hdl_t *h = (vs_hdl_t*)hdl;
    int rc = 0;
    param_enc_t *encp;
    if(h == NULL)
        return CMD_NONE;

    encp = &h->encp;
    rc = zmq_msg_init(&msg);
    OSA_assert(rc == 0);

    rc = zmq_recvmsg(h->server->socket, &msg, 0);
    if(rc == -1)
    {
        zmq_msg_close (&msg);
        return ret;
    }

    msg_data = zmq_msg_data(&msg);
    printf(">>>>>>>>>>>> Get message : %s \n", (char*)msg_data);
    json = json_tokener_parse(msg_data);
    if(json && !is_error(json))
    {
        json_object *cmd = json_object_object_get(json, "bellserver_cmd");
        if(cmd && !is_error(cmd))
        {
            if(json_object_is_type(cmd, json_type_string))
            {
                const char *data = json_object_get_string(cmd);
                
                if(strcmp(data, "preview") == 0)
                    ret = CMD_SET_PREVIEW;
                else if(strcmp(data, "bell_basic") == 0)
                    ret = CMD_SET_BELL_BASIC;
                else if(strcmp(data, "belllite") == 0)
                    ret = CMD_SET_BELLLITE;
                else if(strcmp(data, "bell_mode0") == 0)
                    ret = CMD_SET_BELL_MODE0;
                else if(strcmp(data, "bell_mode1") == 0)
                    ret = CMD_SET_BELL_MODE1;
                else if(strcmp(data, "bell_mode2") == 0)
                    ret = CMD_SET_BELL_MODE2;
                else if(strcmp(data, "bell_mode3") == 0)
                    ret = CMD_SET_BELL_MODE3;
                else if(strcmp(data, "bell_mode4") == 0)
                    ret = CMD_SET_BELL_MODE4;
                else if(strcmp(data, "bell_mode5-0") == 0)
                    ret = CMD_SET_BELL_MODE5_0;
				else if(strcmp(data, "bell_mode5-1") == 0)
                    ret = CMD_SET_BELL_MODE5_1;
                else if(strcmp(data, "bell_mode6") == 0)
                    ret = CMD_SET_BELL_MODE6;
                else if(strcmp(data, "bell_mode7") == 0)
                    ret = CMD_SET_BELL_MODE7;
                else if(strcmp(data, "bell_mode8") == 0)
                    ret = CMD_SET_BELL_MODE8;
                else if(strcmp(data, "switch_layout") == 0)
                    ret = CMD_SWITCH_LAYOUT;
                else if(strstr(data, "setRes") != NULL)
                {
                    sscanf(strstr(data, "setRes"), "setRes(%d*%d@%d)", &encp->width, &encp->height, &encp->fps);
                }
                else if(strstr(data, "siz=") != NULL)
                {
                    sscanf(strstr(data, "siz="), "siz=%dx%d", &encp->width, &encp->height);
                }
                else if(strstr(data, "br=") != NULL)
                {
                    sscanf(strstr(data, "br="), "br=%d", &encp->br);
                }
                else if(strstr(data, "fps=") != NULL)
                {
                    sscanf(strstr(data, "fps="), "fps=%d", &encp->fps);
                }
                else if(strstr(data, "pkt=") != NULL)
                {
                    sscanf(strstr(data, "pkt="), "pkt=%d", &encp->pkt_mode);
                }
                else if(strstr(data, "profile=") != NULL)
                {
                    sscanf(strstr(data, "profile="), "profile=%d", &encp->profile);
                }
                else if(strstr(data, "level=") != NULL)
                {
                    sscanf(strstr(data, "level="), "level=%d", &encp->level);
                }
                else if(strstr(data, "max-br=") != NULL)
                {
                    sscanf(strstr(data, "max-br="), "max-br=%d", &encp->max_br);
                }
                else if(strstr(data, "max-fps=") != NULL)
                {
                    sscanf(strstr(data, "max-fps="), "max-fps=%d", &encp->max_fps);
                }
                else if(strstr(data, "max-mbps=") != NULL)
                {
                    sscanf(strstr(data, "max-mbps="), "max-mbps=%d", &encp->max_mbps);
                }
                else if(strstr(data, "max-smbps=") != NULL)
                {
                    sscanf(strstr(data, "max-smbps="), "max-smbps=%d", &encp->max_smbps);
                }
                else if(strstr(data, "max-fs=") != NULL)
                {
                    sscanf(strstr(data, "max-fs="), "max-fs=%d", &encp->max_fs);
                }
                else if(strstr(data, "force-IDR") != NULL)
                {
                    Bell_api_force_idr();
                }
                else if(strcmp(data, "stop") == 0)
                    ret = CMD_STOP;
                else if(strcmp(data, "exit") == 0)
                    ret = CMD_EXIT;

            }
            else if(json_object_is_type(cmd, json_type_object))
            {
            }
        }
        json_object_put(json);
    }

    printf(">>>>>>>>>>>> Change usecase to %d \n", ret);
    zmq_msg_close (&msg);
    return ret;
}

void closeall(int fd)
{
    int fdlimit = sysconf(_SC_OPEN_MAX);

    while(fd < fdlimit)
    {
        close(fd++);
    }
}

int daemon_init(int close_tty)
{
    switch(fork())
    {
    case 0:
        break;
    case -1:
        return -1;
    default:
        _exit(0);
    }

    printf("fork 1\n");

    if(setsid() < 0)
    {
        printf("setsid 1\n");
        return -1;
    }

    switch(fork())
    {
    case 0:
        break;
    case -1:
        return -1;
    default:
        _exit(0);
    }

    printf("fork 2\n");
    if(close_tty)
    {
        closeall(0);
        dup(0);
        dup(0);
    }

    return 0;
}

void* thr_test(void* h)
{    
     bell_zmq_repack_t *b = connect_to_videoserver(NULL);
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"preview\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_basic\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"belllite\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode0\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode1\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode2\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode3\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode4\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode5-0\"}");
	 sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode5-1\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode6\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode7\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"bell_mode8\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"stop\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"preview\"}");
     sleep(5);
     send_cmd_to_videoserver(b, "{\"bellserver_cmd\":\"exit\"}");
     disconnect_with_videoserver(b);
     sleep(5);

     return NULL;
}

void test()
{
    OSA_ThrHndl hndl;
    OSA_thrCreate(&hndl, thr_test, 2, 0, NULL);
}

void cleanTask(int sig) {
    void           *array[32];    /* Array to store backtrace symbols */
    size_t          size;     /* To store the exact no of values stored */
    char          **strings;    /* To store functions from the backtrace list in ARRAY */
    size_t          nCnt;

    size = backtrace(array, 32);
    strings = backtrace_symbols(array, size);

    /* prints each string of function names of trace*/
    for (nCnt = 0; nCnt < size; nCnt++)
        fprintf(stderr, "%s\n", strings[nCnt]);
  
    videoServer_stop(global_hdl);
    videoServer_delete(global_hdl);
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d @@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", sig);
    exit(0);
}

int main(int argc, char *argv[])
{
    int done = FALSE;
    int usercase = -1;
    int prev_case = usercase;
    int layout_status = 0;
    char ver[64];
    int ver_len = 0;
    if(daemon_init(0) < 0)
    {
        exit(2);
    }
    signal(SIGINT, cleanTask);
    signal(SIGTERM, cleanTask);
    signal(SIGABRT, cleanTask);
    signal(SIGSEGV, cleanTask);
    
    ver_len = video_server_version_str(ver);
    if(ver_len > 0)
    {
        int i = 0;
        printf("================================================================\n");
        printf("=                                                              =\n");
        printf("=");
        for(i = 0; i < (62 - ver_len)/2; i++)
            printf(" ");
        printf(ver);
        i += ver_len;
        for(; i < 62; i++)
            printf(" ");
        printf("=\n");
        printf("=                                                              =\n");
        printf("================================================================\n");
    }

    global_hdl = videoServer_create();
    if(global_hdl == NULL)
    {
        printf("ERROR: Demo start failed!!!\n");
        return -1;
    }
    //test();
    while(!done)
    {
        switch(get_msg(global_hdl))
        {
            case CMD_INFO:
                videoServer_printInfo();
                break;
            case CMD_STATUS:
                Vsys_getCoreStatus(DEMO_CORE_STATUS_CHECK_TIMEOUT);
                break;
            case CMD_SWITCH_LAYOUT:
                if(layout_status == 0)
                {
                    Bell_api_disp_layout(0, 1, 1);
                    layout_status = 1;
                }
                else if(layout_status == 1)
                {
                    Bell_api_disp_layout(1, 0, 2);
                    layout_status = 2;
                }
                else
                {
                    Bell_api_disp_layout(0,1, 2);
                    layout_status = 0;
                }
                break;
            case CMD_EXIT:
                done = TRUE;
                break;
            case CMD_STOP:
                videoServer_stop(global_hdl);
                layout_status = 0;
                usercase = -1;
                break;
            case CMD_SET_PREVIEW:
                usercase = BELL_USECASE_PREVIEW;
                break;
            case CMD_SET_IPNC:
                usercase = BELL_USECASE_IPNCDEMO;
                break;
            case CMD_SET_BELLLITE:
                usercase = BELL_USECASE_BELLLITE;
                break;
            case CMD_SET_BELL_BASIC:
                usercase = BELL_USECASE_BELL_BASIC;
                break;
            case CMD_SET_BELL_MODE0:
                usercase =  BELL_USECASE_MODE0;
                break;
            case CMD_SET_BELL_MODE1:
                usercase =  BELL_USECASE_MODE1;
                break;
            case CMD_SET_BELL_MODE2:
                usercase =  BELL_USECASE_MODE2;
                break;
            case CMD_SET_BELL_MODE3:
                usercase =  BELL_USECASE_MODE3;
                break;
            case CMD_SET_BELL_MODE4:
                usercase =  BELL_USECASE_MODE4;
                break;
            case CMD_SET_BELL_MODE5_0:
                usercase =  BELL_USECASE_MODE5_0;
                layout_status = 0;
                break;
			case CMD_SET_BELL_MODE5_1:
                usercase =  BELL_USECASE_MODE5_1;
                layout_status = 2;
                break;
            case CMD_SET_BELL_MODE6:
                usercase =  BELL_USECASE_MODE6;
                break;
            case CMD_SET_BELL_MODE7:
                usercase =  BELL_USECASE_MODE7;
                break;
            case CMD_SET_BELL_MODE8:
                usercase =  BELL_USECASE_MODE8;
                break;                
            case CMD_SET_BASIC_HDMI_IN:
                usercase = BELL_USECASE_BELL_BASIC_HDMI_IN;
                break;
            default:
                //sleep(2);
                break;
        }
        
        if(prev_case == -1)
        {
            if(usercase >= 0)
            {
                printf("starting usecase %d \n", usercase);
                videoServer_start(global_hdl, usercase);
                sleep(5);
            }
        }
        prev_case = usercase;
        OSA_waitMsecs(5);
    }

    videoServer_stop(global_hdl);    
    videoServer_delete(global_hdl);
    
    return 0;
}

