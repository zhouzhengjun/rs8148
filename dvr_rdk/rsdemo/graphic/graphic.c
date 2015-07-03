/*******************************************************************************
 *                                                                             *
 * Copyright (c) 2011 Texas Instruments Incorporated - http://www.ti.com/      *
 *                        ALL RIGHTS RESERVED                                  *
 *                                                                             *
 ******************************************************************************/

/*----------------------------------------------------------------------------
 Defines referenced header files
-----------------------------------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/ti81xxfb.h>

#define GRPX_SC_MARGIN_OFFSET   (3)

#include "ti_media_std.h"
#include "graphic.h"
#include "graphic_priv.h"

#include "rsdemo/mcfw_api_demos/BECodec/bell_swms.h"

#if defined(TI_816X_BUILD)
#include "graphic_single_buf_nontied.h"
#define GRPX_FB_SINGLE_BUFFER_NON_TIED_GRPX
#endif

#if defined(TI_814X_BUILD)
#include "graphic_single_buf_tied.h"
#define GRPX_FB_SINGLE_BUFFER_TIED_GRPX
#endif

#if defined(TI_8107_BUILD)
#include "graphic_separate_buf_nontied.h"
#define GRPX_FB_SEPARATE_BUFFER_NON_TIED_GRPX
// #define NUM_GRPX_DISPLAYS   2
#endif

/*----------------------------------------------------------------------------
 local function
-----------------------------------------------------------------------------*/

int disp_getregparams(int display_fd)
{
    int ret;

    struct ti81xxfb_region_params regp;

    memset(&regp, 0, sizeof(regp));

    ret = ioctl(display_fd, TIFB_GET_PARAMS, &regp);
    if (ret < 0) {
        eprintf("TIFB_GET_PARAMS\n");
        return ret;
    }

    dprintf("\n");
    dprintf("Reg Params Info\n");
    dprintf("---------------\n");
    dprintf("region %d, postion %d x %d, prioirty %d\n",
        regp.ridx,
        regp.pos_x,
        regp.pos_y,
        regp.priority);
    dprintf("first %d, last %d\n",
        regp.firstregion,
        regp.lastregion);
    dprintf("sc en %d, sten en %d\n",
        regp.scalaren,
        regp.stencilingen);
    dprintf("tran en %d, type %d, key %d\n",
        regp.transen,
        regp.transtype,
        regp.transcolor);
    dprintf("blend %d, alpha %d\n"
        ,regp.blendtype,
        regp.blendalpha);
    dprintf("bb en %d, alpha %d\n",
        regp.bben,
        regp.bbalpha);
    dprintf("\n");
    return 0;
}

int disp_fbinfo(int fd)
{
    struct fb_fix_screeninfo fixinfo;
    struct fb_var_screeninfo varinfo, org_varinfo;
    int size;
    int ret;

    /* Get fix screen information. Fix screen information gives
     * fix information like panning step for horizontal and vertical
     * direction, line length, memory mapped start address and length etc.
     */
    ret = ioctl(fd, FBIOGET_FSCREENINFO, &fixinfo);
    if (ret < 0) {
        eprintf("FBIOGET_FSCREENINFO !!!\n");
        return -1;
    }

    {
        dprintf("==========================================================\n");
        dprintf("Fix Screen Info\n");
        dprintf("---------------\n");
        dprintf("Line Length - %d\n", fixinfo.line_length);
        dprintf("Physical Address = %lx\n",fixinfo.smem_start);
        dprintf("Buffer Length = %d\n",fixinfo.smem_len);
        dprintf("\n");
    }

    /* Get variable screen information. Variable screen information
     * gives informtion like size of the image, bites per pixel,
     * virtual size of the image etc. */
    ret = ioctl(fd, FBIOGET_VSCREENINFO, &varinfo);
    if (ret < 0) {
        eprintf("FBIOGET_VSCREENINFO !!!\n");
        return -1;
    }

    {
        dprintf("\n");
        dprintf("Var Screen Info\n");
        dprintf("---------------\n");
        dprintf("Xres - %d\n", varinfo.xres);
        dprintf("Yres - %d\n", varinfo.yres);
        dprintf("Xres Virtual - %d\n", varinfo.xres_virtual);
        dprintf("Yres Virtual - %d\n", varinfo.yres_virtual);
        dprintf("Bits Per Pixel - %d\n", varinfo.bits_per_pixel);
        dprintf("Pixel Clk - %d\n", varinfo.pixclock);
        dprintf("Rotation - %d\n", varinfo.rotate);
        dprintf("\n");
    }

    disp_getregparams(fd);

    memcpy(&org_varinfo, &varinfo, sizeof(varinfo));

    /*
     * Set the resolution which read before again to prove the
     * FBIOPUT_VSCREENINFO ioctl.
     */

    ret = ioctl(fd, FBIOPUT_VSCREENINFO, &org_varinfo);
    if (ret < 0) {
        eprintf("FBIOPUT_VSCREENINFO !!!\n");
        return -1;
    }

    /* It is better to get fix screen information again. its because
     * changing variable screen info may also change fix screen info. */
    ret = ioctl(fd, FBIOGET_FSCREENINFO, &fixinfo);
    if (ret < 0) {
        eprintf("FBIOGET_FSCREENINFO !!!\n");
        return -1;
    }

    size = varinfo.xres*varinfo.yres*(varinfo.bits_per_pixel/8);
    dprintf("\n");
    dprintf("### BUF SIZE = %d Bytes !!! \n", size);
    dprintf("\n");

    return size;
}

int draw_fill_color(unsigned char *buf_addr, int curWidth, int curHeight)
{
    unsigned int i, j;
    unsigned char *p;
    app_grpx_t *grpx = &grpx_obj;

    if(buf_addr==NULL)
        return -1;

    p = (unsigned char *)buf_addr;
    for(i = 0; i < curHeight; i++) {
        for(j = 0; j < curWidth; j++) {
            switch(grpx->planeType)
            {
                case GRPX_FORMAT_RGB565:
                    *p++  = (RGB_KEY_16BIT_GRAY >> 0 ) & 0xFF;
                    *p++  = (RGB_KEY_16BIT_GRAY >> 8 ) & 0xFF;
                break;
                case GRPX_FORMAT_RGB888:
                    *p++  = (RGB_KEY_24BIT_GRAY >> 0  ) & 0xFF;
                    *p++  = (RGB_KEY_24BIT_GRAY >> 8  ) & 0xFF;
                    *p++  = (RGB_KEY_24BIT_GRAY >> 16 ) & 0xFF;
                    *p++  = 0xFF;
                break;
                case GRPX_FORMAT_MAX:
                default:
                break;
            }
        }
    }

    return 0;
}
/* function to draw box on grapics plane */
int draw_grid(unsigned char *buf_addr, int flag)
{
    unsigned int i, j, k;
    unsigned char *p;
    int            width;
    int            height;
    int            lineWidth;
    int            numHoriGridLines;
    int            numVerGridLines;
    int            horiGridSpace;
    int            verGridSpace;
    int            planeWidth;
    int            color;
    app_grpx_t *grpx = &grpx_obj;

    if(buf_addr==NULL)
        return -1;

    width       = GRPX_PLANE_GRID_WIDTH;
    height      = GRPX_PLANE_GRID_HEIGHT;
    planeWidth  = GRPX_PLANE_GRID_WIDTH;
    numHoriGridLines = 3; /* for 3x3 Grid, Need to make it configurable*/
    numVerGridLines  = 3; /* for 3x3 Grid, Need to make it configurable*/
    horiGridSpace    = width/(numVerGridLines- 1);
    verGridSpace     = height/(numHoriGridLines - 1);
    lineWidth        = 2;  /* In pixels */

    p = (unsigned char *)(buf_addr);

    if(flag == TRUE)
       color = GRPX_PLANE_GRID_COLOR;
    else
       color = GRPX_PLANE_GRID_COLOR_BLANK;
       
    for(k=0; k<numHoriGridLines; k++)
    {

         p = (unsigned char *)(buf_addr) + 
                    grpx->bytes_per_pixel * (planeWidth * (k* ((height/(numVerGridLines - 1) - lineWidth))));

        for(i=0; i<lineWidth; i++)
        {
            for(j=0; j<width; j++)
            {            
                if(grpx->planeType == GRPX_FORMAT_RGB565)
                {
                    {
                        *p = color;
                        *(p + 1) = color;
                    }
                }
                if(grpx->planeType == GRPX_FORMAT_RGB888)
                {
                    {

                        *p = color;
                        *(p + 1) = color;
                        *(p + 2) = color;
                        *(p + 3) = color;
                    }
                }
                p += grpx->bytes_per_pixel;
             }
         }
    }

    for(k=0; k<numVerGridLines; k++)
    {
        int pos = (grpx->bytes_per_pixel * (k * (width/(numVerGridLines - 1))));
        if(pos >= width)
            pos = width - 1;
        p = (unsigned char *)(buf_addr) + pos;

        for(i=0; i<height; i++)
        {
            for(j=0; j<lineWidth; j++)
            {            
                if(grpx->planeType == GRPX_FORMAT_RGB565)
                {
                    {
                        *p = color;
                        *(p + 1) = color;
                    }
                }
                if(grpx->planeType == GRPX_FORMAT_RGB888)
                {
                    {

                        *p = color;
                        *(p + 1) = color;
                        *(p + 2) = color;
                        *(p + 3) = color;
                    }
                }
                p += grpx->bytes_per_pixel;
             }
             p += (grpx->bytes_per_pixel * (planeWidth - lineWidth));
         }
    }
    return 0;

}

/* function to draw box on grapics plane */
int draw_box(unsigned char *buf_addr, int flag,
             int            startX,
             int            startY,
             int            width,
             int            height)
//             int            planeWidth)
{
    unsigned int i, j, k;
    unsigned char *p;
    int            lineWidth;
    int            numHoriGridLines;
    int            numVerGridLines;
    int            planeWidth;
    int            color;
    app_grpx_t *grpx = &grpx_obj;

    if(buf_addr==NULL)
        return -1;

    planeWidth  = GRPX_PLANE_GRID_WIDTH;

    numHoriGridLines = 2;
    numVerGridLines  = 2;
    lineWidth        = 2;  /* In pixels */

    p = (unsigned char *)(buf_addr) + ((startX * grpx->bytes_per_pixel)+( startY * grpx->bytes_per_pixel * planeWidth));

    if(flag == TRUE)
       color = GRPX_PLANE_GRID_COLOR;
    else
       color = GRPX_PLANE_GRID_COLOR_BLANK;
       
    for(k=0; k<numHoriGridLines; k++)
    {
        for(i=0; i<lineWidth; i++)
        {
            for(j=0; j<width; j++)
            {            
                if(grpx->planeType == GRPX_FORMAT_RGB565)
                {
                    {
                        *p = color;
                        *(p + 1) = color;
                    }
                }
                if(grpx->planeType == GRPX_FORMAT_RGB888)
                {
                    {

                        *p = color;
                        *(p + 1) = color;
                        *(p + 2) = color;
                        *(p + 3) = color;
                    }
                }
                p += grpx->bytes_per_pixel;
             }
             p += (grpx->bytes_per_pixel * (planeWidth - width));
         }
         p += grpx->bytes_per_pixel * (planeWidth * (height - lineWidth));
    }

    for(k=0; k<numVerGridLines; k++)
    {
        p = (unsigned char *)(buf_addr) + \
                  ((((k * width)  + startX) * grpx->bytes_per_pixel) +  \
                       ( startY * grpx->bytes_per_pixel * planeWidth));
        for(i=0; i<height; i++)
        {
            for(j=0; j<lineWidth; j++)
            {            
                if(grpx->planeType == GRPX_FORMAT_RGB565)
                {
                    {
                        *p = color;
                        *(p + 1) = color;
                    }
                }
                if(grpx->planeType == GRPX_FORMAT_RGB888)
                {
                    {

                        *p = color;
                        *(p + 1) = color;
                        *(p + 2) = color;
                        *(p + 3) = color;
                    }
                }
                p += grpx->bytes_per_pixel;
             }
             p += (grpx->bytes_per_pixel * (planeWidth - lineWidth));
         }
    }
    return 0;
}

int draw_img(unsigned char *buf_addr,
             unsigned char *img_addr,
             int            sx,
             int            sy,
             int            wi,
             int            ht,
			 int            planeWidth)
{
    unsigned int i, j;
    unsigned char *p;
    app_grpx_t *grpx = &grpx_obj;

    if(buf_addr==NULL || img_addr==NULL)
        return -1;

    p = (unsigned char *)(buf_addr + ((sx * grpx->bytes_per_pixel)+( sy * grpx->bytes_per_pixel * planeWidth)));

    for(j=0; j<ht; j++)
    {
        for(i=0; i<wi; i++)
        {
            if(grpx->planeType == GRPX_FORMAT_RGB565)
            {
                {
                    *p = *img_addr;
                    *(p + 1) = *(img_addr + 1);
                }
            }
            if(grpx->planeType == GRPX_FORMAT_RGB888)
            {
                {

                    *p = *img_addr;
                    *(p + 1) = *(img_addr + 1);
                    *(p + 2) = *(img_addr + 2);
                    *(p + 3) = *(img_addr + 3);
                }
            }
            p        += grpx->bytes_per_pixel;
            img_addr += grpx->bytes_per_pixel;
        }
        p += ((planeWidth-wi) * grpx->bytes_per_pixel);
    }

    return 0;
}

Int32 grpx_fb_scale(VDIS_DEV devId,
					UInt32   startX,
                    UInt32   startY,
                    UInt32   outWidth,
                    UInt32   outHeight)
{

    struct ti81xxfb_scparams scparams;
    Int32                    fd = 0, status = 0;
    app_grpx_t *grpx = &grpx_obj;
    int dummy;
    struct ti81xxfb_region_params  regp;
    char filename[100], buffer[10];
    int r = -1;

    if (devId == VDIS_DEV_HDMI){
        fd = grpx->fd;
        VDIS_CMD_IS_GRPX_ON(filename, buffer, VDIS_GET_GRPX, 0, 1, r);
    }
    if (devId == VDIS_DEV_SD){
        fd = grpx->fd2;
        VDIS_CMD_IS_GRPX_ON(filename, buffer, VDIS_GET_GRPX, 2, 1, r);
    }

    /* Set Scalar Params for resolution conversion 
     * inHeight and inWidth should remain same based on grpx buffer type 
	 */

#if defined(GRPX_FB_SEPARATE_BUFFER_NON_TIED_GRPX)
	if (devId == VDIS_DEV_HDMI){
		scparams.inwidth  = GRPX_PLANE_HD_WIDTH;
		scparams.inheight = GRPX_PLANE_HD_HEIGHT;
	}
    if (devId == VDIS_DEV_SD){
		scparams.inwidth  = GRPX_PLANE_SD_WIDTH;
		scparams.inheight = GRPX_PLANE_SD_HEIGHT;
	}
#endif 

#if defined(GRPX_FB_SINGLE_BUFFER_TIED_GRPX) || defined (GRPX_FB_SINGLE_BUFFER_NON_TIED_GRPX)
     scparams.inwidth  = GRPX_PLANE_WIDTH;
     scparams.inheight = GRPX_PLANE_HEIGHT;
#endif 



    // this "-GRPX_SC_MARGIN_OFFSET" is needed since scaling can result in +2 extra pixels, so we compensate by doing -2 here
    scparams.outwidth = outWidth - GRPX_SC_MARGIN_OFFSET;
    scparams.outheight = outHeight - GRPX_SC_MARGIN_OFFSET;
    scparams.coeff = NULL;

    if (ioctl(fd, TIFB_GET_PARAMS, &regp) < 0) {
        eprintf("TIFB_GET_PARAMS !!!\n");
    }

    regp.pos_x = startX;
    regp.pos_y = startY;
    regp.transen = TI81XXFB_FEATURE_DISABLE;
    regp.transcolor = 0x00FF00FF;
    regp.transtype = TI81XXFB_TRANSP_LSBMASK_NO;
    regp.scalaren = TI81XXFB_FEATURE_ENABLE;
    regp.blendtype = TI81XXFB_BLENDING_PIXEL;

    /*not call the IOCTL, ONLY if 100% sure that GRPX is off*/
    if (!((r == 0) && (atoi(buffer) == 0))) {
        if (ioctl(fd, FBIO_WAITFORVSYNC, &dummy)) {
            eprintf("FBIO_WAITFORVSYNC !!!\n");
            return -1;
        }
    }
    if ((status = ioctl(fd, TIFB_SET_SCINFO, &scparams)) < 0) {
        eprintf("TIFB_SET_SCINFO !!!\n");
    }


    if (ioctl(fd, TIFB_SET_PARAMS, &regp) < 0) {
        eprintf("TIFB_SET_PARAMS !!!\n");
    }

    return (status);

}

int grpx_fb_init(grpx_plane_type planeType)
{
    app_grpx_t *grpx = &grpx_obj;

    memset(grpx, 0, sizeof(app_grpx_t));

    // need to start and stop FBDev once for the RGB565 and SC to take effect

    if(planeType >= GRPX_FORMAT_MAX)
    {
        return -1;
    }
    else
    {
        grpx->planeType = planeType;
    }

    /* For TI816x */
#if defined(GRPX_FB_SINGLE_BUFFER_NON_TIED_GRPX)
	grpx_fb_start_singleBufNonTied();
#endif

    /* For TI814x */
#if defined(GRPX_FB_SINGLE_BUFFER_TIED_GRPX)
	grpx_fb_start_singleBufTied();
#endif

    /* For TI8107 */
#if defined (GRPX_FB_SEPARATE_BUFFER_NON_TIED_GRPX)
	grpx_fb_start_separateBufNonTied();
#endif


    return 0;
}

void grpx_fb_exit(void)
{
    dprintf("\n");
    dprintf("grpx_fb_exit ... \n");

#if defined(GRPX_FB_SINGLE_BUFFER_NON_TIED_GRPX)
	grpx_fb_stop_singleBufNonTied();
#endif

#if defined(GRPX_FB_SINGLE_BUFFER_TIED_GRPX)
	grpx_fb_stop_singleBufTied();
#endif

#if defined (GRPX_FB_SEPARATE_BUFFER_NON_TIED_GRPX)
	grpx_fb_stop_separateBufNonTied();
#endif

    return;
}

int grpx_fb_draw_grid()
{
    VDIS_DEV devId;
    app_grpx_t *grpx = &grpx_obj;

    dprintf("grpx_fb_draw_grid ... \n");
    
    devId = VDIS_DEV_HDMI;

    if(grpx->planeType >= GRPX_FORMAT_MAX)
    {
        return -1;
    }

    if (devId == VDIS_DEV_HDMI) 
    {
        draw_grid(grpx->buf[0], TRUE);

//       grpx_grid_fb_start_singleBufNonTied();
    }
    dprintf("grpx_fb_draw_grid ... Done !!! \n");

    return 0;
}


int grpx_fb_draw_grid_exit()
{
    dprintf("\n");
    dprintf("grpx_fb_grid_exit ... \n");
    {
        app_grpx_t *grpx = &grpx_obj;

        draw_grid(grpx->buf[0], FALSE);

        dprintf("grpx_fb_grid_exit ... Done!!!\n");
        dprintf("\n");

     }
//    grpx_grid_fb_stop_singleBufNonTied();

    return 0;
}

//int grpx_fb_draw_box()
int grpx_fb_draw_box(int width,
                     int height,
                     int startX,
                     int startY)

{
    VDIS_DEV devId;
    app_grpx_t *grpx = &grpx_obj;

//    dprintf("grpx_fb_draw_box ... \n");
    
    devId = VDIS_DEV_HDMI;

    if(grpx->planeType >= GRPX_FORMAT_MAX)
    {
        return -1;
    }

    if (devId == VDIS_DEV_HDMI) 
    {
#if 0
        width       = 100;
        height      = 50;
        startX      = 520;
        startY      = 300;

       printf(" Width %d height %d startX %d startY %d \n",
                       width, height, startX, startY );
#endif       
       draw_box(grpx->buf[0], TRUE,
                 startX,
                 startY,
                 width,
                 height
                 );

//       grpx_grid_fb_start_singleBufNonTied();
    }
//    dprintf("grpx_fb_draw_box ... Done !!! \n");

    return 0;
}

int grpx_fb_draw_box_exit(int width,
                          int height,
                          int startX,
                          int startY)
{
//    dprintf("grpx_fb_box_exit ... \n");
    {
        app_grpx_t *grpx = &grpx_obj;
#if 0
        int            width;
        int            height;
        int            startX;
        int            startY;
        width       = 100;
        height      = 50;
        startX      = 520;
        startY      = 300;
#endif
        draw_box(grpx->buf[0], FALSE,
                 startX,
                 startY,
                 width,
                 height
                 );
//        dprintf("grpx_fb_box_exit ... Done!!!\n");

     }
//    grpx_grid_fb_stop_singleBufNonTied();

    return 0;
}


#if 0
Int32 grpx_fb_demo()
{
    UInt32 devId;
    UInt32 outWidth, outHeight;
    UInt32 startX, startY;
    UInt32 offsetX, offsetY;
    UInt32 loopCount, i;
    UInt32 runCount;

    devId = VDIS_DEV_SD;

    runCount = 10000;

    loopCount = 100;
    offsetX = offsetY = 1;

    /* putting in a loop for test */
    while(runCount--)
    {
        /* putting in another loop to change size and position every few msecs */
        for(i=1; i<=loopCount; i++)
        {
            Bell_GetSize(Vdis_getResolution(devId), &outWidth, &outHeight);

            startX = offsetX*i;
            startY = offsetY*i;

            outWidth  -= startX*2;
            outHeight -= startY*2;

            grpx_fb_scale(devId, startX, startY, outWidth, outHeight);
        }
        for(i=loopCount; i>=1; i--)
        {
            Bell_GetSize(Vdis_getResolution(devId), &outWidth, &outHeight);

            startX = offsetX*i;
            startY = offsetY*i;

            outWidth  -= startX*2;
            outHeight -= startY*2;

            grpx_fb_scale(devId, startX, startY, outWidth, outHeight);
        }

        /* restore to original */
        Bell_GetSize(Vdis_getResolution(devId), &outWidth, &outHeight);

        dprintf("[reset] %d x %d\n", outWidth, outHeight);
        grpx_fb_scale(devId, 0, 0, outWidth, outHeight);
    }

    return 0;
}
#endif

