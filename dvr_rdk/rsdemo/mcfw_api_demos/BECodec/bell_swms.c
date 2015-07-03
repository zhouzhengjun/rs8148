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

#include <bell_swms.h>
#include <mcfw/interfaces/common_def/ti_vsys_common_def.h>


// #define PAL_INPUT

Void Bell_swMsSetOutputFPS(VDIS_MOSAIC_S * vdMosaicParam, UInt32 outputFPS)
{
    vdMosaicParam->outputFPS = outputFPS;
}

Int32 Bell_swMsGetOutputFPS(VDIS_MOSAIC_S * vdMosaicParam)
{
    return vdMosaicParam->outputFPS;
}
Void Bell_swMsGenerateLayout_for_switch(VDIS_DEV devId, UInt32 startChId, UInt32 maxChns, UInt32 layoutId,
          VDIS_MOSAIC_S * vdMosaicParam, Bool forceLowCostScaling, UInt32 demoType, UInt32 resolution)
{
    UInt32 outWidth, outHeight, row, col, winId, widthAlign, heightAlign, i;
    UInt32 rowMax, colMax;

    Bell_GetSize(resolution, &outWidth, &outHeight);

    widthAlign = 8;
    heightAlign = 1;

    /* In the LAYOUT_MODE_4CH_4CH leave out portion of
     * screen blank by reducing the outWIdth and outHeight.
     * This is to test if the portion of video buffer
     * not containing active video is blanked or not
     */
    if (LAYOUT_MODE_4CH_4CH == layoutId)
    {
        outWidth  -= 80;
        outHeight -= 80;
    }
    /***END DEBUG***/
    if(devId>=VDIS_DEV_MAX)
        devId = VDIS_DEV_HDMI;


    vdMosaicParam->onlyCh2WinMapChanged = FALSE;

    vdMosaicParam->displayWindow.height     = outHeight;
    vdMosaicParam->displayWindow.width      = outWidth;
    vdMosaicParam->displayWindow.start_X    = 0;
    vdMosaicParam->displayWindow.start_Y    = 0;

    if(layoutId >= LAYOUT_MAX)
        layoutId = LAYOUT_MODE_4CH_4CH;

    for (i = 0; i < VDIS_MOSAIC_WIN_MAX; i++)
    {
        /*vdMosaicParam->useLowCostScaling[i] = 1;*/
        vdMosaicParam->chnMap[i] = VDIS_CHN_INVALID;
    }

    if(layoutId == LAYOUT_MODE_4CH_4CH)
    {
        vdMosaicParam->numberOfWindows = 8;

        for(row=0; row<2; row++)
        {
            for(col=0; col<2; col++)
            {
                winId = row*2+col;

                vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth*2)/5, widthAlign);
                vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/2, heightAlign);
                vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
                vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

                vdMosaicParam->chnMap[winId] = winId;
                if (forceLowCostScaling == TRUE)
                    vdMosaicParam->useLowCostScaling[winId] = 1;
                else
                    vdMosaicParam->useLowCostScaling[winId] = 0;
            }
        }

        for(row=0; row<4; row++)
        {
            winId = 4 + row;

            vdMosaicParam->winList[winId].width = vdMosaicParam->winList[0].width/2;
            vdMosaicParam->winList[winId].height = vdMosaicParam->winList[0].height/2;
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width*2;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
        }
    }

    if(layoutId == LAYOUT_MODE_1CH)
    {
        vdMosaicParam->numberOfWindows = 1;

        winId = 0;

        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;
        vdMosaicParam->winList[winId].width = outWidth;
        vdMosaicParam->winList[winId].height = outHeight;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;
    }

    if(layoutId == LAYOUT_MODE_6CH)
    {
        vdMosaicParam->numberOfWindows = 6;

        winId = 0;

        vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth*2)/3, widthAlign);
        vdMosaicParam->winList[winId].height = VsysUtils_floor((outHeight*2)/3, heightAlign);
        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;

        for(row=0; row<2; row++)
        {
            winId = 1 + row;

            vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].height = VsysUtils_floor(vdMosaicParam->winList[0].height/2, heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;

        }

        for(col=0; col<3; col++)
        {
            winId = 3 + col;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(vdMosaicParam->winList[0].width/2, widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(vdMosaicParam->winList[0].height/2, heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[0].height;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
            if (winId == 5) /* the last window */
            {
                vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[3].width * 2;
            }

        }

    }


    if(layoutId == LAYOUT_MODE_7CH_1CH)
    {
        vdMosaicParam->numberOfWindows = 8;

        winId = 0;

        vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth)/4, widthAlign)*3;
        vdMosaicParam->winList[winId].height = VsysUtils_floor((outHeight)/4, heightAlign)*3;
        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;

        for(row=0; row<3; row++)
        {
            winId = 1 + row;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(
                                outWidth - vdMosaicParam->winList[0].width,
                                widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(
                                vdMosaicParam->winList[0].height / 3,
                                heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;

        }

        for(col=0; col<4; col++)
        {
            winId = 4 + col;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth / 4, widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(
                                outHeight - vdMosaicParam->winList[0].height,
                                heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[0].height;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
        }
    }

	if((layoutId == LAYOUT_MODE_PIP) || (layoutId == LAYOUT_MODE_PIP2) || (layoutId == LAYOUT_MODE_PIP2_MAIN2))
	{
			winId = 0;
	
			vdMosaicParam->winList[winId].start_X = 0;
			vdMosaicParam->winList[winId].start_Y = 0;
			vdMosaicParam->winList[winId].width = outWidth;
			vdMosaicParam->winList[winId].height = outHeight;

			if (layoutId == LAYOUT_MODE_PIP2_MAIN2)
				vdMosaicParam->chnMap[winId] = 2;
			else
				vdMosaicParam->chnMap[winId] = winId;
	
			if (forceLowCostScaling == TRUE)
				vdMosaicParam->useLowCostScaling[winId] = 1;
			else
				vdMosaicParam->useLowCostScaling[winId] = 0;
	
#define BOTTOM_MIDDLE
#define BOTTOM_RIGHT
#ifdef BOTTOM_RIGHT
		winId++;
		vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/4, widthAlign);
		vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/4, heightAlign);
			//window 2, Bottom Right
		vdMosaicParam->winList[winId].start_X = VsysUtils_floor(outWidth * 7 /10, widthAlign);
	
		vdMosaicParam->winList[winId].start_Y = VsysUtils_floor(
							outHeight - vdMosaicParam->winList[winId].height - outHeight/20,
							heightAlign
							);
		vdMosaicParam->chnMap[winId] = winId;
		vdMosaicParam->useLowCostScaling[winId] = 1;
#endif
#ifdef BOTTOM_MIDDLE
		if ((layoutId == LAYOUT_MODE_PIP2) || (layoutId == LAYOUT_MODE_PIP2_MAIN2)) {
			winId++;
			vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/4, widthAlign);
			vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/4, heightAlign);
				//window 1, Bottom Middle
			vdMosaicParam->winList[winId].start_X = VsysUtils_floor(outWidth * 3 / 8, widthAlign);
			vdMosaicParam->winList[winId].start_Y = VsysUtils_floor(
							outHeight - vdMosaicParam->winList[winId].height - outHeight/20,
								heightAlign
								);
			if ((layoutId == LAYOUT_MODE_PIP2_MAIN2))
				vdMosaicParam->chnMap[winId] = 0;
			else
				vdMosaicParam->chnMap[winId] = winId;
			vdMosaicParam->useLowCostScaling[winId] = 1;
		}
#endif
		vdMosaicParam->numberOfWindows = winId + 1;
	}
    if(layoutId == LAYOUT_MODE_PIP_FAKE)
    {
        vdMosaicParam->winList[0].start_X = 0;
        vdMosaicParam->winList[0].start_Y = 0;
        vdMosaicParam->winList[0].width = outWidth;
        vdMosaicParam->winList[0].height = outHeight;
        vdMosaicParam->chnMap[0] = 1;
        vdMosaicParam->useLowCostScaling[0] = 1;

        vdMosaicParam->winList[1].start_X = 0;
        vdMosaicParam->winList[1].start_Y = 0;
        vdMosaicParam->winList[1].width = outWidth;
        vdMosaicParam->winList[1].height = outHeight;
        vdMosaicParam->chnMap[1] = 0;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[1] = 1;
        else
            vdMosaicParam->useLowCostScaling[1] = 0;

        vdMosaicParam->numberOfWindows = 2;

    }

    switch (layoutId){
        case    LAYOUT_MODE_4CH:
            rowMax = 2;
            colMax = 2;
            break;
        case    LAYOUT_MODE_9CH:
            rowMax = 3;
            colMax = 3;
            break;
        case    LAYOUT_MODE_16CH:
            rowMax = 4;
            colMax = 4;
            break;
        case    LAYOUT_MODE_20CH_4X5:
            rowMax = 4;
            colMax = 5;
            break;
        case    LAYOUT_MODE_25CH_5X5:
            rowMax = 5;
            colMax = 5;
            break;
        case    LAYOUT_MODE_30CH_5X6:
            rowMax = 5;
            colMax = 6;
            break;
        case    LAYOUT_MODE_36CH_6X6:
            rowMax = 6;
            colMax = 6;
            break;
        default:
            rowMax = 0;
            colMax = 0;
    }

    if(rowMax != 0)
    {
        vdMosaicParam->numberOfWindows = rowMax * colMax;

        for(row=0; row<rowMax; row++)
        {
            for(col=0; col<colMax; col++)
            {
                winId = row*colMax+col;

                vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/colMax, widthAlign);
                vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/rowMax, heightAlign);
                vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
                vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;
                
                if (col == colMax - 1) /* the last col */
                {
                    vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[0].width * (colMax - 1);
                }

                if(winId < maxChns)
                    vdMosaicParam->chnMap[winId] = winId;
                else
                    vdMosaicParam->chnMap[winId] = SW_MS_INVALID_ID;


                if (layoutId != LAYOUT_MODE_4CH)
                {
                    vdMosaicParam->useLowCostScaling[winId] = 1;
                }
                else
                {
                    vdMosaicParam->useLowCostScaling[winId] = 0;
                }
                if (forceLowCostScaling == TRUE)
                    vdMosaicParam->useLowCostScaling[winId] = 1;
            }
       }

    }

    for (winId=0; winId<vdMosaicParam->numberOfWindows; winId++)
    {
     if (winId < maxChns)
        vdMosaicParam->chnMap[winId] = (vdMosaicParam->chnMap[winId] + startChId)%maxChns;
     else
        vdMosaicParam->chnMap[winId] = SW_MS_INVALID_ID;

    }

//    Bell_swMs_PrintLayoutParams(vdMosaicParam);
}

Void Bell_swMsGenerateLayout(VDIS_DEV devId, UInt32 startChId, UInt32 maxChns, UInt32 layoutId,
          VDIS_MOSAIC_S * vdMosaicParam, Bool forceLowCostScaling, UInt32 demoType, UInt32 resolution)
{
    UInt32 outWidth, outHeight, row, col, winId, widthAlign, heightAlign, i;
    UInt32 rowMax, colMax;

    Bell_GetSize(resolution, &outWidth, &outHeight);

    widthAlign = 8;
    heightAlign = 1;

    /* In the LAYOUT_MODE_4CH_4CH leave out portion of
     * screen blank by reducing the outWIdth and outHeight.
     * This is to test if the portion of video buffer
     * not containing active video is blanked or not
     */
    if (LAYOUT_MODE_4CH_4CH == layoutId)
    {
        outWidth  -= 80;
        outHeight -= 80;
    }
    /***END DEBUG***/
    if(devId>=VDIS_DEV_MAX)
        devId = VDIS_DEV_HDMI;

    /* Set the swMs output fps custom value
       OR
       Modify with new value if required */
    if (demoType == BELL_TYPE_PROGRESSIVE)
    {
#ifdef PAL_INPUT
         vdMosaicParam->outputFPS = 25;
#else
#ifdef TI_816X_BUILD
         vdMosaicParam->outputFPS = 29;
#else
         vdMosaicParam->outputFPS = 30;
#endif
#endif
    }
    else
    {
        vdMosaicParam->outputFPS = 60;
        if (devId == VDIS_DEV_SD)
        {
            vdMosaicParam->outputFPS = 30;
        }
    }

    vdMosaicParam->onlyCh2WinMapChanged = FALSE;

    vdMosaicParam->displayWindow.height     = outHeight;
    vdMosaicParam->displayWindow.width      = outWidth;
    vdMosaicParam->displayWindow.start_X    = 0;
    vdMosaicParam->displayWindow.start_Y    = 0;

    if(layoutId >= LAYOUT_MAX)
        layoutId = LAYOUT_MODE_4CH_4CH;

    for (i = 0; i < VDIS_MOSAIC_WIN_MAX; i++)
    {
        /*vdMosaicParam->useLowCostScaling[i] = 1;*/
        vdMosaicParam->chnMap[i] = VDIS_CHN_INVALID;
    }

    if(layoutId == LAYOUT_MODE_4CH_4CH)
    {
        vdMosaicParam->numberOfWindows = 8;

        for(row=0; row<2; row++)
        {
            for(col=0; col<2; col++)
            {
                winId = row*2+col;

                vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth*2)/5, widthAlign);
                vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/2, heightAlign);
                vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
                vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

                vdMosaicParam->chnMap[winId] = winId;
                if (forceLowCostScaling == TRUE)
                    vdMosaicParam->useLowCostScaling[winId] = 1;
                else
                    vdMosaicParam->useLowCostScaling[winId] = 0;
            }
        }

        for(row=0; row<4; row++)
        {
            winId = 4 + row;

            vdMosaicParam->winList[winId].width = vdMosaicParam->winList[0].width/2;
            vdMosaicParam->winList[winId].height = vdMosaicParam->winList[0].height/2;
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width*2;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
        }
    }

    if(layoutId == LAYOUT_MODE_1CH)
    {
        vdMosaicParam->numberOfWindows = 1;

        winId = 0;

        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;
        vdMosaicParam->winList[winId].width = outWidth;
        vdMosaicParam->winList[winId].height = outHeight;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;
    }

    if(layoutId == LAYOUT_MODE_6CH)
    {
        vdMosaicParam->numberOfWindows = 6;

        winId = 0;

        vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth*2)/3, widthAlign);
        vdMosaicParam->winList[winId].height = VsysUtils_floor((outHeight*2)/3, heightAlign);
        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;

        for(row=0; row<2; row++)
        {
            winId = 1 + row;

            vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].height = VsysUtils_floor(vdMosaicParam->winList[0].height/2, heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;

        }

        for(col=0; col<3; col++)
        {
            winId = 3 + col;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(vdMosaicParam->winList[0].width/2, widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(vdMosaicParam->winList[0].height/2, heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[0].height;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
            if (winId == 5) /* the last window */
            {
                vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[3].width * 2;
            }

        }

    }


    if(layoutId == LAYOUT_MODE_7CH_1CH)
    {
        vdMosaicParam->numberOfWindows = 8;

        winId = 0;

        vdMosaicParam->winList[winId].width = VsysUtils_floor((outWidth)/4, widthAlign)*3;
        vdMosaicParam->winList[winId].height = VsysUtils_floor((outHeight)/4, heightAlign)*3;
        vdMosaicParam->winList[winId].start_X = 0;
        vdMosaicParam->winList[winId].start_Y = 0;

        vdMosaicParam->chnMap[winId] = winId;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[winId] = 1;
        else
            vdMosaicParam->useLowCostScaling[winId] = 0;

        for(row=0; row<3; row++)
        {
            winId = 1 + row;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(
                                outWidth - vdMosaicParam->winList[0].width,
                                widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(
                                vdMosaicParam->winList[0].height / 3,
                                heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[0].width;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;

        }

        for(col=0; col<4; col++)
        {
            winId = 4 + col;

            vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth / 4, widthAlign);
            vdMosaicParam->winList[winId].height = VsysUtils_floor(
                                outHeight - vdMosaicParam->winList[0].height,
                                heightAlign);
            vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
            vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[0].height;

            vdMosaicParam->chnMap[winId] = winId;
            vdMosaicParam->useLowCostScaling[winId] = 1;
        }
    }

	if((layoutId == LAYOUT_MODE_PIP) || (layoutId == LAYOUT_MODE_PIP2) || (layoutId == LAYOUT_MODE_PIP2_MAIN2))
	{
			winId = 0;
	
			vdMosaicParam->winList[winId].start_X = 0;
			vdMosaicParam->winList[winId].start_Y = 0;
			vdMosaicParam->winList[winId].width = outWidth;
			vdMosaicParam->winList[winId].height = outHeight;

			if (layoutId == LAYOUT_MODE_PIP2_MAIN2)
				vdMosaicParam->chnMap[winId] = 2;
			else
				vdMosaicParam->chnMap[winId] = winId;
	
			if (forceLowCostScaling == TRUE)
				vdMosaicParam->useLowCostScaling[winId] = 1;
			else
				vdMosaicParam->useLowCostScaling[winId] = 0;
	
#define BOTTOM_MIDDLE
#define BOTTOM_RIGHT
#ifdef BOTTOM_RIGHT
		winId++;
		vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/4, widthAlign);
		vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/4, heightAlign);
			//window 2, Bottom Right
		vdMosaicParam->winList[winId].start_X = VsysUtils_floor(outWidth * 7 /10, widthAlign);
	
		vdMosaicParam->winList[winId].start_Y = VsysUtils_floor(
							outHeight - vdMosaicParam->winList[winId].height - outHeight/20,
							heightAlign
							);
		vdMosaicParam->chnMap[winId] = winId;
		vdMosaicParam->useLowCostScaling[winId] = 1;
#endif
#ifdef BOTTOM_MIDDLE
		if ((layoutId == LAYOUT_MODE_PIP2) || (layoutId == LAYOUT_MODE_PIP2_MAIN2)) {
			winId++;
			vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/4, widthAlign);
			vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/4, heightAlign);
				//window 1, Bottom Middle
			vdMosaicParam->winList[winId].start_X = VsysUtils_floor(outWidth * 3 / 8, widthAlign);
			vdMosaicParam->winList[winId].start_Y = VsysUtils_floor(
							outHeight - vdMosaicParam->winList[winId].height - outHeight/20,
								heightAlign
								);
			if ((layoutId == LAYOUT_MODE_PIP2_MAIN2))
				vdMosaicParam->chnMap[winId] = 0;
			else
				vdMosaicParam->chnMap[winId] = winId;
			vdMosaicParam->useLowCostScaling[winId] = 1;
		}
#endif
		vdMosaicParam->numberOfWindows = winId + 1;
	}
    if(layoutId == LAYOUT_MODE_PIP_FAKE)
    {
        vdMosaicParam->winList[0].start_X = 0;
        vdMosaicParam->winList[0].start_Y = 0;
        vdMosaicParam->winList[0].width = outWidth;
        vdMosaicParam->winList[0].height = outHeight;
        vdMosaicParam->chnMap[0] = 1;
        vdMosaicParam->useLowCostScaling[0] = 1;

        vdMosaicParam->winList[1].start_X = 0;
        vdMosaicParam->winList[1].start_Y = 0;
        vdMosaicParam->winList[1].width = outWidth;
        vdMosaicParam->winList[1].height = outHeight;
        vdMosaicParam->chnMap[1] = 0;

        if (forceLowCostScaling == TRUE)
            vdMosaicParam->useLowCostScaling[1] = 1;
        else
            vdMosaicParam->useLowCostScaling[1] = 0;

        vdMosaicParam->numberOfWindows = 2;

    }

    switch (layoutId){
        case    LAYOUT_MODE_4CH:
            rowMax = 2;
            colMax = 2;
            break;
        case    LAYOUT_MODE_9CH:
            rowMax = 3;
            colMax = 3;
            break;
        case    LAYOUT_MODE_16CH:
            rowMax = 4;
            colMax = 4;
            break;
        case    LAYOUT_MODE_20CH_4X5:
            rowMax = 4;
            colMax = 5;
            break;
        case    LAYOUT_MODE_25CH_5X5:
            rowMax = 5;
            colMax = 5;
            break;
        case    LAYOUT_MODE_30CH_5X6:
            rowMax = 5;
            colMax = 6;
            break;
        case    LAYOUT_MODE_36CH_6X6:
            rowMax = 6;
            colMax = 6;
            break;
        default:
            rowMax = 0;
            colMax = 0;
    }

    if(rowMax != 0)
    {
        vdMosaicParam->numberOfWindows = rowMax * colMax;

        for(row=0; row<rowMax; row++)
        {
            for(col=0; col<colMax; col++)
            {
                winId = row*colMax+col;

                vdMosaicParam->winList[winId].width = VsysUtils_floor(outWidth/colMax, widthAlign);
                vdMosaicParam->winList[winId].height = VsysUtils_floor(outHeight/rowMax, heightAlign);
                vdMosaicParam->winList[winId].start_X = vdMosaicParam->winList[winId].width*col;
                vdMosaicParam->winList[winId].start_Y = vdMosaicParam->winList[winId].height*row;
                
                if (col == colMax - 1) /* the last col */
                {
                    vdMosaicParam->winList[winId].width = outWidth - vdMosaicParam->winList[0].width * (colMax - 1);
                }

                if(winId < maxChns)
                    vdMosaicParam->chnMap[winId] = winId;
                else
                    vdMosaicParam->chnMap[winId] = SW_MS_INVALID_ID;


                if (layoutId != LAYOUT_MODE_4CH)
                {
                    vdMosaicParam->useLowCostScaling[winId] = 1;
                }
                else
                {
                    vdMosaicParam->useLowCostScaling[winId] = 0;
                }
                if (forceLowCostScaling == TRUE)
                    vdMosaicParam->useLowCostScaling[winId] = 1;
            }
       }

    }

    for (winId=0; winId<vdMosaicParam->numberOfWindows; winId++)
    {
     if (winId < maxChns)
        vdMosaicParam->chnMap[winId] = (vdMosaicParam->chnMap[winId] + startChId)%maxChns;
     else
        vdMosaicParam->chnMap[winId] = SW_MS_INVALID_ID;

    }

//    Bell_swMs_PrintLayoutParams(vdMosaicParam);
}

Void Bell_swMs_PrintLayoutParams(VDIS_MOSAIC_S * vdMosaicParam)
{
    UInt32 winId, chNum;

    printf( " \n"
            " ***  Mosaic Parameters *** \n"
            " \n"
            " Output     |  Output         | Display \n"
            " Start X, Y |  Width x Height | Windows \n"
            " -------------------------------------- \n"
            " %4d, %4d | %5d x %6d | %1d\n",
            vdMosaicParam->displayWindow.start_X,
            vdMosaicParam->displayWindow.start_Y,
            vdMosaicParam->displayWindow.width,
            vdMosaicParam->displayWindow.height,
            vdMosaicParam->numberOfWindows);

    printf( " \n"
            " ***  Mosaic Parameters *** \n"
            " \n"
            " Win | Ch  | Output     |  Output         | Low Cost \n"
            " Num | Num | Start X, Y |  Width x Height | ON / OFF \n"
            " --------------------------------------------------- \n"
            );


    for (winId = 0; winId < vdMosaicParam->numberOfWindows; winId++)
    {
        chNum = vdMosaicParam->chnMap[winId];

        printf
            (" %3d | %3d | %4d, %4d | %5d x %6d | %1d\n",
                winId,
                chNum,
                vdMosaicParam->winList[winId].start_X,
                vdMosaicParam->winList[winId].start_Y,
                vdMosaicParam->winList[winId].width,
                vdMosaicParam->winList[winId].height,
                vdMosaicParam->useLowCostScaling[winId]
            );
    }

    printf( " \n");

}




