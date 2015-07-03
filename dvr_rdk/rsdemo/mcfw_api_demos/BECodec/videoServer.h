#ifndef BELL_SERVER_H_
#define BELL_SERVER_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h> 
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "zmq/zmq.h"
#include <pthread.h>
#define OPEN_MODE 00777

#define VIDEO_SERVER_VERSION        0x0001000A
#define VIDEO_SERVER_MAJOR_VERSION  (((VIDEO_SERVER_VERSION)&0xFF000000)>>24)
#define VIDEO_SERVER_MINOR_VERSION  (((VIDEO_SERVER_VERSION)&0x00FF0000)>>16)
#define VIDEO_SERVER_REVISION       (((VIDEO_SERVER_VERSION)&0x0000FF00)>>8)
#define VIDEO_SERVER_STUFFIX        ((VIDEO_SERVER_VERSION)&0x000000FF)

static int video_server_version_str(char version[64])
{
    int i = 0;
    if(    VIDEO_SERVER_MAJOR_VERSION > 9
        || VIDEO_SERVER_MINOR_VERSION > 99
        || VIDEO_SERVER_REVISION > 99)
    {
        return -1;
    }

    memset(version, 0, 32);
    version[i++] = 'v';
    version[i++] = '0'+VIDEO_SERVER_MAJOR_VERSION;
    version[i++] = '.';
    version[i++] = '0'+(VIDEO_SERVER_MINOR_VERSION/10);
    version[i++] = '0'+(VIDEO_SERVER_MINOR_VERSION%10);
    version[i++] = '.';
    version[i++] = '0'+(VIDEO_SERVER_REVISION/10);
    version[i++] = '0'+(VIDEO_SERVER_REVISION%10);

    switch(VIDEO_SERVER_STUFFIX)
    {
    case 0x0A:
        strcat(version, "-Alpha");
        break;
    case 0x0B:
        strcat(version, "-Beta");
        break;
    case 0x0C:
        strcat(version, "-Certified");
        break;
    case 0x0D:
        strcat(version, "-Demo");
        break;
    case 0x0E:
        strcat(version, "-Evaluation");
        break;
    case 0x0F:
        strcat(version, "-Full");
        break;
    default:
        strcat(version, "-Build(");
        strcat(version, __DATE__);
        strcat(version, ")");
        break;
    }

    return strlen(version);
}

typedef struct bellloop_t
{
    int             shm_id;
    char const      *name;
    unsigned char   *mmap;
    unsigned char   *start_addr;
    int             size;
    int             *rd_offset;
    int             *wt_offset;
    int             *flag_loop;
	pthread_mutex_t *pmutex;
}bellloop_t;

typedef struct bell_zmq_repack_t
{
    void *ctx;
    void *socket;
}bell_zmq_repack_t;

#define IPC_BELL_VIDEOSERVER "ipc:///opt/bin/bell_videoserver"

static int disconnect_with_videoserver(bell_zmq_repack_t *client)
{
    if(client == NULL)
        return 0;

    if(client->socket)
    {
        zmq_disconnect(client->socket, IPC_BELL_VIDEOSERVER);
        zmq_close(client->socket);
    }

    if(client->ctx)
        zmq_ctx_destroy(client->ctx);

    free(client);

    return 0;
}

static bell_zmq_repack_t* connect_to_videoserver(const char *server_name)
{
    int rc = 0;
    bell_zmq_repack_t *client = (bell_zmq_repack_t*)malloc(sizeof(bell_zmq_repack_t));
    if(client == NULL)
        return NULL;
    
    memset(client, 0, sizeof(bell_zmq_repack_t));
    client->ctx = zmq_init(1);
    client->socket = zmq_socket(client->ctx,ZMQ_PUSH);
    if(server_name)
        rc = zmq_connect(client->socket, server_name);
    else
        rc = zmq_connect(client->socket, IPC_BELL_VIDEOSERVER);
    if(rc != 0 || client->socket == NULL || client->ctx == NULL)
    {
        printf("Connect to Video Server Failed!\n");
        disconnect_with_videoserver(client);
        client = NULL;
    }
    
    return client;
}

static int send_cmd_to_videoserver(bell_zmq_repack_t *client, char const *msg)
{
	int str_size = 0, ret = 0;
    zmq_msg_t reply;
    if(msg == NULL)
        return -1;

    str_size = strlen(msg)+1;
    zmq_msg_init_size(&reply, str_size);
    memcpy(zmq_msg_data(&reply), msg, str_size);
    ret = zmq_sendmsg(client->socket, &reply, ZMQ_DONTWAIT);
	printf("send: %s\n",msg);
    zmq_msg_close(&reply);
    return ret;
}

static void mmap_close(bellloop_t *bellloop)
{
    int ret = -1;

    if(bellloop == NULL)
        return;

    if(bellloop->mmap)
    {
        ret = shmdt(bellloop->mmap);
        if(-1 == ret)
        {
            printf("munmap add_r failure\n");
        }
    }

    free(bellloop);
}

static void mmap_delete(bellloop_t *bellloop)
{
    int ret = -1;

    if(bellloop == NULL)
        return;

    if(bellloop->mmap)
    {
        ret = shmdt(bellloop->mmap);
        if(-1 == ret)
        {
            printf("munmap add_r failure\n");
        }
    }
    shmctl(bellloop->shm_id, IPC_RMID, NULL);
    free(bellloop);
}

#include <errno.h>
static bellloop_t* mmap_create(char const *name, int size, int create, int id)
{
    int ret = -1;
    bellloop_t *bellloop = NULL;
    key_t key;
    int shm_id;
    key = ftok("/", id);
    if(key == -1)
    {
        printf("ftok error");
        return NULL;
    }
    printf("key=%d\n",key) ;
    if(create)
        shm_id = shmget(key,size + 3*sizeof(int)+sizeof(pthread_mutex_t),IPC_CREAT|IPC_EXCL|0600);
    else
        shm_id = shmget(key,size + 3*sizeof(int)+sizeof(pthread_mutex_t),0|0600);
    
    if(-1 == (ret = shm_id))
    {
        return NULL;
    }
    printf("shm_id=%d\n", shm_id) ;

    bellloop = (bellloop_t*)malloc(sizeof(bellloop_t));
    if(bellloop == NULL)
    {
        return NULL;
    }
    
    memset(bellloop, 0, sizeof(bellloop_t));
    bellloop->size = size;
    bellloop->name = name;
    bellloop->shm_id = shm_id;
    
    bellloop->mmap = shmat(shm_id,NULL,0);
    if(NULL == bellloop->mmap)
    {
        mmap_delete(bellloop);
        return NULL;
    }
    bellloop->start_addr = bellloop->mmap + 3 * sizeof(int)+sizeof(pthread_mutex_t);
    bellloop->wt_offset = (int*)(bellloop->mmap);
    bellloop->rd_offset = (int*)(bellloop->mmap + sizeof(int));
    bellloop->flag_loop=   (int*)(bellloop->mmap + 2 * sizeof(int));
	bellloop->pmutex = (pthread_mutex_t*)(bellloop->mmap + 3 * sizeof(int));

	pthread_mutexattr_t attr_perf;
	pthread_mutexattr_init(&attr_perf);  
    pthread_mutexattr_setpshared(&attr_perf, PTHREAD_PROCESS_SHARED);  
    pthread_mutex_init(bellloop->pmutex, &attr_perf);  
    return bellloop;
}

#define BUF_START_CODE  (0x89ABCDEF)
#define BUF_LOOP_CODE  (0xFEDCBA98)

#define GET_BUF_SIZE    (1024*1024*4)
#define SEND_BUF_SIZE   (1024*1024*4)

#define OPEN_FEED_SHM(shm) \
do{ \
    shm = mmap_create("bell_stream_input", GET_BUF_SIZE, 0, 3);\
    printf("Creating mmap bell_stream_input \n");\
}while(0)

#define OPEN_ENC_SHM(shm) \
do{ \
    shm = mmap_create("bell_stream_output", SEND_BUF_SIZE, 0, 9);\
    printf("Creating mmap bell_stream_output \n");\
}while(0)

#define CLOSE_SHM(shm) mmap_close(shm)

/**
    \brief Codec Identifier for Decoder
*/
typedef enum
{
    BELL_VDEC_CHN_H264 = 66,                     
    /**< BaseLine Profile   */

    BELL_VDEC_CHN_MJPEG = 77,                     
    /**< Main Profile       */

    BELL_VDEC_CHN_MPEG4 = 88                     
    /**< Advanced Simple Profile */

} BELL_VDEC_CHN_CODEC_E;

typedef struct bellbuf_t
{
    unsigned int    starcode;
    unsigned int    buf_size;
    int             chn_id;
    BELL_VDEC_CHN_CODEC_E codec_type;
    void            *buf;
}bellbuf_t;

static bellbuf_t* get_buf(bellloop_t *loopbuf, char *cFlag)
{
    bellbuf_t *addr = NULL;
    int aligned_size = 0;
    if(loopbuf == NULL)
        return NULL;
     
    addr = (bellbuf_t*)(loopbuf->start_addr + (*loopbuf->rd_offset));
	
	
	if(addr->starcode != BUF_START_CODE && addr->starcode != BUF_LOOP_CODE)
	{
     	return NULL;
	}
	//printf("%s: get_buf flag is %d, flagaddr:%d rd:%d, startcode:%#x\n",cFlag,*loopbuf->flag_loop,loopbuf->flag_loop,(*loopbuf->rd_offset),addr->starcode);
	
    if(addr->starcode == BUF_LOOP_CODE)
    {		
		pthread_mutex_lock(loopbuf->pmutex);
        memset(addr, 0, GET_BUF_SIZE - (*loopbuf->rd_offset));		
		*loopbuf->rd_offset = 0;
		*loopbuf->flag_loop = 0;
		pthread_mutex_unlock(loopbuf->pmutex);
		printf("%s:get_buf-->> BUF_LOOP_CODE\n",cFlag);
		return NULL;
    }
    addr->buf = loopbuf->start_addr + (*loopbuf->rd_offset) + sizeof(bellbuf_t);
    aligned_size = (((addr->buf_size + sizeof(bellbuf_t) + 128) >> 4) << 4);
    *loopbuf->rd_offset = (*loopbuf->rd_offset) + aligned_size;
    return addr;
}

static int clean_buf(bellloop_t *loopbuf,bellbuf_t* buf)
{
    int aligned_size = 0;
    if(buf == NULL)
        return -1;

    aligned_size = (((buf->buf_size + sizeof(bellbuf_t) + 128) >> 4) << 4);
	pthread_mutex_lock(loopbuf->pmutex);
    memset(buf, 0, aligned_size);
	pthread_mutex_unlock(loopbuf->pmutex);
    return 0;
}

static bellbuf_t* request_buf(bellloop_t *loopbuf, int size, char *cFlag)
{
    bellbuf_t *addr = NULL;
    int aligned_size = 0;
    if(loopbuf == NULL)
        return NULL;

    //printf("%s: request_buf flag is %d, wd:%d\n",cFlag, *loopbuf->flag_loop, (*loopbuf->wt_offset));
    aligned_size = (((size + sizeof(bellbuf_t) + 128) >> 4) << 4);
    if(aligned_size > (SEND_BUF_SIZE - (*loopbuf->wt_offset) - sizeof(bellbuf_t)))
    {
        addr = (bellbuf_t*)(loopbuf->start_addr + (*loopbuf->wt_offset));
		pthread_mutex_lock(loopbuf->pmutex);
		*loopbuf->wt_offset = 0;	
		*loopbuf->flag_loop  =  1 ;
		
	    addr->starcode = BUF_LOOP_CODE;
		pthread_mutex_unlock(loopbuf->pmutex);
		printf("%s:request_buf-->> BUF_LOOP_CODE\n ",cFlag);
		//return NULL;
    }

    addr = (bellbuf_t*)(loopbuf->start_addr + (*loopbuf->wt_offset)); 
	
    if(addr->starcode != 0 || ((*loopbuf->flag_loop  != 0) && (*loopbuf->wt_offset >= *loopbuf->rd_offset)))
    //if(addr->starcode != 0 || *loopbuf->flag_loop != 0)
	{    	   
	  	printf("%s:request_buf-->return NULL!!! with starcode:%#x,flag_loop:%d,rd:%d,wt:%d\n",
			cFlag,addr->starcode,*loopbuf->flag_loop,*loopbuf->rd_offset,*loopbuf->wt_offset);
    	return NULL;
	}

    addr->buf = loopbuf->start_addr + (*loopbuf->wt_offset) + sizeof(bellbuf_t);
    addr->buf_size = size;
	
    return addr;
}

static int set_buf(bellloop_t *loopbuf, bellbuf_t *buf)
{
    int aligned_size = 0;
    if(buf == NULL)
        return -1;

    aligned_size = (((buf->buf_size + sizeof(bellbuf_t) + 128) >> 4) << 4);
    buf->buf = NULL;
	pthread_mutex_lock(loopbuf->pmutex);
    buf->starcode = BUF_START_CODE;
	pthread_mutex_unlock(loopbuf->pmutex);
    *loopbuf->wt_offset = (*loopbuf->wt_offset) + aligned_size;
    return 0;
}
#endif
