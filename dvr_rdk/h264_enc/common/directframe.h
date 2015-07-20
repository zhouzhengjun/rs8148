#ifndef RUISU_DIRECTFRAME_H_
#define RUISU_DIRECTFRAME_H_

/**
    Maximum Number of video frame buffers in VIDEO_FRAMEBUF_LIST_S
*/
#define     DF_FRAMEBUF_MAX             (16)

/**
    Maximum Number of video fields in VIDEO frame (TOP/BOTTOM)
*/
#define     DF_MAX_FIELDS               (2)

/**
    Maximum Number of video planes in video frame (Y/U/V)
*/
#define     DF_MAX_PLANES               (3)

typedef enum
{
    DF_STD_NTSC = 0u,
    /**< 720x480 30FPS interlaced NTSC standard. */
    DF_STD_PAL,
    /**< 720x576 30FPS interlaced PAL standard. */

    DF_STD_480I,
    /**< 720x480 30FPS interlaced SD standard. */
    DF_STD_576I,
    /**< 720x576 30FPS interlaced SD standard. */

    DF_STD_CIF,
    /**< Interlaced, 360x120 per field NTSC, 360x144 per field PAL. */
    DF_STD_HALF_D1,
    /**< Interlaced, 360x240 per field NTSC, 360x288 per field PAL. */
    DF_STD_D1,
    /**< Interlaced, 720x240 per field NTSC, 720x288 per field PAL. */

    DF_STD_480P,
    /**< 720x480 60FPS progressive ED standard. */
    DF_STD_576P,
    /**< 720x576 60FPS progressive ED standard. */

    DF_STD_720P_60,
    /**< 1280x720 60FPS progressive HD standard. */
    DF_STD_720P_50,
    /**< 1280x720 50FPS progressive HD standard. */

    DF_STD_1080I_60,
    /**< 1920x1080 30FPS interlaced HD standard. */
    DF_STD_1080I_50,
    /**< 1920x1080 50FPS interlaced HD standard. */

    DF_STD_1080P_60,
    /**< 1920x1080 60FPS progressive HD standard. */
    DF_STD_1080P_50,
    /**< 1920x1080 50FPS progressive HD standard. */

    DF_STD_1080P_24,
    /**< 1920x1080 24FPS progressive HD standard. */
    DF_STD_1080P_30,
    /**< 1920x1080 30FPS progressive HD standard. */

    DF_STD_VGA_60,
    /**< 640x480 60FPS VESA standard. */
    DF_STD_VGA_72,
    /**< 640x480 72FPS VESA standard. */
    DF_STD_VGA_75,
    /**< 640x480 75FPS VESA standard. */
    DF_STD_VGA_85,
    /**< 640x480 85FPS VESA standard. */

    DF_STD_SVGA_60,
    /**< 800x600 60FPS VESA standard. */
    DF_STD_SVGA_72,
    /**< 800x600 72FPS VESA standard. */
    DF_STD_SVGA_75,
    /**< 800x600 75FPS VESA standard. */
    DF_STD_SVGA_85,
    /**< 800x600 85FPS VESA standard. */

    DF_STD_XGA_60,
    /**< 1024x768 60FPS VESA standard. */
    DF_STD_XGA_70,
    /**< 1024x768 72FPS VESA standard. */
    DF_STD_XGA_75,
    /**< 1024x768 75FPS VESA standard. */
    DF_STD_XGA_85,
    /**< 1024x768 85FPS VESA standard. */

    DF_STD_WXGA_60,
    /**< 1280x768 60FPS VESA standard. */
    DF_STD_WXGA_75,
    /**< 1280x768 75FPS VESA standard. */
    DF_STD_WXGA_85,
    /**< 1280x768 85FPS VESA standard. */

    DF_STD_SXGA_60,
    /**< 1280x1024 60FPS VESA standard. */
    DF_STD_SXGA_75,
    /**< 1280x1024 75FPS VESA standard. */
    DF_STD_SXGA_85,
    /**< 1280x1024 85FPS VESA standard. */

    DF_STD_SXGAP_60,
    /**< 1400x1050 60FPS VESA standard. */
    DF_STD_SXGAP_75,
    /**< 1400x1050 75FPS VESA standard. */

    DF_STD_UXGA_60,
    /**< 1600x1200 60FPS VESA standard. */

    DF_STD_MUX_2CH_D1,
    /**< Interlaced, 2Ch D1, NTSC or PAL. */
    DF_STD_MUX_2CH_HALF_D1,
    /**< Interlaced, 2ch half D1, NTSC or PAL. */
    DF_STD_MUX_2CH_CIF,
    /**< Interlaced, 2ch CIF, NTSC or PAL. */
    DF_STD_MUX_4CH_D1,
    /**< Interlaced, 4Ch D1, NTSC or PAL. */
    DF_STD_MUX_4CH_CIF,
    /**< Interlaced, 4Ch CIF, NTSC or PAL. */
    DF_STD_MUX_4CH_HALF_D1,
    /**< Interlaced, 4Ch Half-D1, NTSC or PAL. */
    DF_STD_MUX_8CH_CIF,
    /**< Interlaced, 8Ch CIF, NTSC or PAL. */
    DF_STD_MUX_8CH_HALF_D1,
    /**< Interlaced, 8Ch Half-D1, NTSC or PAL. */

    DF_STD_AUTO_DETECT,
    /**< Auto-detect standard. Used in capture mode. */
    DF_STD_CUSTOM,
    /**< Custom standard used when connecting to external LCD etc...
         The video timing is provided by the application.
         Used in display mode. */

    DF_STD_MAX
    /**< Should be the last value of this enumeration.
         Will be used by driver for validating the input parameters. */
} DF_VIDEO_STANDARD_E;

typedef struct {

    void *    addr[DF_MAX_FIELDS][DF_MAX_PLANES];
    /**< virtual address of vid frame buffer pointers */

    void *    phyAddr[DF_MAX_FIELDS][DF_MAX_PLANES];
    /**< virtual address of vid frame buffer pointers */

    unsigned int channelNum;
    /**< Coding type */

    unsigned int timeStamp;
    /**< Capture or Display time stamp */

    unsigned int fid;
    /**< Field indentifier (TOP/BOTTOM/FRAME) */

    unsigned int frameWidth;
    /**< Width of the frame */

    unsigned int frameHeight;
    /**< Height of the frame */
    
    unsigned int framePitch[DF_MAX_PLANES];
    /**< Pitch of the frame */

    void *    linkPrivate;
    /**< Link private info. Application should preserve this value and not overwrite it */

} df_fb;

/**
    \brief Video Bitstream Buffer List

    List of Video Bitstream Buffer's allows user to efficient exchange
    multiple frames with VENC, VDEC sub-system with less overhead
*/
typedef struct {

    unsigned int              numFrames;
    /**< Number of valid frame's in frames[]. MUST be <= VIDEO_FRAMEBUF_MAX */

    df_fb    frames[DF_FRAMEBUF_MAX];
    /**< Video frame information list */

} df_fblist;

typedef void (*df_framein_cb) (void *cbCtx);

void* dframe_create(int outwidth, int outheight, int videostd,int argc, char **argv);
void dframe_delete(void *ctx);
int dframe_start(void *ctx);
int dframe_stop(void *ctx);
//VIDFrame_Buf * dframe_cap_getVIDFrame(void *ctx);
//int dframe_cap_putVIDFrame(void *ctx,VIDFrame_Buf *frameBuf);
int dframe_cap_getframe(void *ctx,unsigned char *frame, int* size);
int dframe_cap_getframes(void *ctx,df_fblist *fblist);
void dframe_cap_setcb(void *ctx,df_framein_cb cb,void *cbctx);
int dframe_cap_putempty(void *ctx,df_fblist *fblist);
int dframe_dis_getempty(void *ctx,df_fblist *fblist);
int dframe_dis_putframes(void *ctx,df_fblist *fblist);

#endif

