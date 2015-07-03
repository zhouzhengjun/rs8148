
#include <dev_dma_priv.h>

int DMA_copyFill1D(  DMA_CopyFillObj *pCopyFillObj)
{
  int i;
  unsigned int aCnt, bCnt, cCnt;
  unsigned int remainSize, transferSize;
  unsigned int *pFill32;
  int status, k;

  for(i=0; i<pCopyFillObj->count; i++) {
  
    if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {
    
      pFill32 =  (unsigned int *)pCopyFillObj->transferPrm[i].dstVirtAddr;
      
      for(k=0; k<8; k++) {
        status = copy_to_user(pFill32, &pCopyFillObj->transferPrm[i].fillValue, sizeof(unsigned int));
        if(status<0) {
          FUNCERR("copy_to_user()\n");
          return -1;
        }
        pFill32++;
      }
        
      pCopyFillObj->transferPrm[i].srcPhysAddr = pCopyFillObj->transferPrm[i].dstPhysAddr;  
    }
          
    if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {
      aCnt = 32;
      bCnt = 512;
    } else {      
      aCnt = 32;
      bCnt = 512;          
    }

    transferSize = 0;
    remainSize = pCopyFillObj->transferPrm[i].size;
    
    if(remainSize > aCnt*bCnt )
    {
      cCnt = remainSize /(aCnt*bCnt);

      pCopyFillObj->dmaParam.src = (uint32_t) pCopyFillObj->transferPrm[i].srcPhysAddr;
      pCopyFillObj->dmaParam.dst = (uint32_t) pCopyFillObj->transferPrm[i].dstPhysAddr;
      pCopyFillObj->dmaParam.link_bcntrld = DMA_PARAM_NO_LINK | (bCnt <<16); 
      
      pCopyFillObj->dmaParam.a_b_cnt = (uint32_t)(bCnt<<16)|aCnt;
      
      pCopyFillObj->dmaParam.ccnt = cCnt;
      pCopyFillObj->dmaParam.opt = DMA_PARAM_OPT_TCINTEN | DMA_PARAM_OPT_SYNCDIM_AB |
                                        ((pCopyFillObj->chId&0x3f)<<12) 
                                        |DMA_PARAM_OPT_ITCCHEN
                                        ;

      if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {
        pCopyFillObj->dmaParam.src_dst_cidx = (0) | (aCnt*bCnt << 16);
        pCopyFillObj->dmaParam.src_dst_bidx = (0) | (aCnt<<16);               
      } else {             
        pCopyFillObj->dmaParam.src_dst_cidx = (aCnt*bCnt) | (aCnt*bCnt << 16);
        pCopyFillObj->dmaParam.src_dst_bidx = aCnt | (aCnt<<16);            
      }
      
      edma_write_slot( pCopyFillObj->chId, &pCopyFillObj->dmaParam );
      
      #ifdef DMA_DEBUG
      DMA_dumpPARAM("CopyFill1D", pCopyFillObj->chId);
      #endif
      #if 1
      edma_start(pCopyFillObj->chId);

      wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
      INIT_COMPLETION(pCopyFillObj->dma_complete);
      #endif
      
      transferSize = aCnt*bCnt*cCnt;
      remainSize = remainSize - transferSize;
    }
    
    if(remainSize>0) {
    
      if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {        
        aCnt = 4;
        bCnt = remainSize/aCnt;          
      } else {
        aCnt = remainSize;
        bCnt = 1;
      }
      cCnt = 1;

      if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {        
        pCopyFillObj->dmaParam.src = (uint32_t) pCopyFillObj->transferPrm[i].srcPhysAddr;          
      } else {
        pCopyFillObj->dmaParam.src = (uint32_t) pCopyFillObj->transferPrm[i].srcPhysAddr+transferSize;          
      }
      
      pCopyFillObj->dmaParam.dst = (uint32_t) pCopyFillObj->transferPrm[i].dstPhysAddr+transferSize;
      pCopyFillObj->dmaParam.link_bcntrld = DMA_PARAM_NO_LINK | (bCnt <<16); 
      
      pCopyFillObj->dmaParam.a_b_cnt = (uint32_t)(bCnt<<16)|aCnt;
      
      pCopyFillObj->dmaParam.ccnt = cCnt;
      pCopyFillObj->dmaParam.opt = DMA_PARAM_OPT_TCINTEN | DMA_PARAM_OPT_SYNCDIM_AB |
                                        ((pCopyFillObj->chId&0x3f)<<12) 
                                        ;

      if(pCopyFillObj->copyFillType==DMA_CMD_FILL_1D) {
        pCopyFillObj->dmaParam.src_dst_cidx = (0) | (aCnt*bCnt << 16);
        pCopyFillObj->dmaParam.src_dst_bidx = (0) | (aCnt<<16);               
      } else {             
        pCopyFillObj->dmaParam.src_dst_cidx = (aCnt*bCnt) | (aCnt*bCnt << 16);
        pCopyFillObj->dmaParam.src_dst_bidx = aCnt | (aCnt<<16);            
      }

      edma_write_slot( pCopyFillObj->chId, &pCopyFillObj->dmaParam );
 
      #ifdef DMA_DEBUG
      DMA_dumpPARAM("CopyFill1D", pCopyFillObj->chId);
      #endif
      
      #if 1                      
      edma_start(pCopyFillObj->chId);

      wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
      INIT_COMPLETION(pCopyFillObj->dma_complete);
      #endif
      
      transferSize = aCnt*bCnt*cCnt;
      remainSize = remainSize - transferSize;
    }
  }

  return 0;
}

int DMA_copyFill2D(  DMA_CopyFillObj *pCopyFillObj)
{
  int i, status, k;
  unsigned int aCnt, bCnt;
  unsigned int *pFill32;

  for(i=0; i<pCopyFillObj->count; i++) {
  
    if(pCopyFillObj->copyFillType==DMA_CMD_FILL_2D) {
      pFill32 =  (unsigned int *)pCopyFillObj->transferPrm[i].dstVirtAddr;
      
      for(k=0; k<4; k++) {      
        status = copy_to_user(pFill32, &pCopyFillObj->transferPrm[i].fillValue, sizeof(unsigned int));
        if(status<0) {
          FUNCERR("copy_to_user()\n");
          return -1;
        }
        pFill32++;
      }
        
      pCopyFillObj->transferPrm[i].srcPhysAddr = pCopyFillObj->transferPrm[i].dstPhysAddr;  
    }

    pCopyFillObj->dmaParam.src = (uint32_t) pCopyFillObj->transferPrm[i].srcPhysAddr;
    pCopyFillObj->dmaParam.dst = (uint32_t) pCopyFillObj->transferPrm[i].dstPhysAddr;

    pCopyFillObj->dmaParam.opt = DMA_PARAM_OPT_TCINTEN | DMA_PARAM_OPT_SYNCDIM_AB |
                                ((pCopyFillObj->chId&0x3f)<<12);
  
    if(pCopyFillObj->copyFillType==DMA_CMD_FILL_2D) {  

      aCnt = 8;
      bCnt = (pCopyFillObj->transferPrm[i].copyWidth+aCnt-1)/aCnt;

      pCopyFillObj->dmaParam.link_bcntrld = DMA_PARAM_NO_LINK | (bCnt << 16);
      pCopyFillObj->dmaParam.src_dst_cidx = (pCopyFillObj->transferPrm[i].dstOffsetH<<16)|(0);
      pCopyFillObj->dmaParam.ccnt = pCopyFillObj->transferPrm[i].copyHeight;
      
      pCopyFillObj->dmaParam.opt |= DMA_PARAM_OPT_ITCCHEN;
  
      pCopyFillObj->dmaParam.src_dst_bidx = (aCnt<<16) | (0);
      
      pCopyFillObj->dmaParam.a_b_cnt = (uint32_t)(bCnt<<16)|aCnt;
      
    } else {
    
      if(pCopyFillObj->transferPrm[i].skipH == 0 ) {

        pCopyFillObj->dmaParam.link_bcntrld = DMA_PARAM_NO_LINK;
        pCopyFillObj->dmaParam.src_dst_cidx = 0;
        pCopyFillObj->dmaParam.ccnt = 1;
    
        pCopyFillObj->dmaParam.src_dst_bidx = (pCopyFillObj->transferPrm[i].dstOffsetH<<16)|pCopyFillObj->transferPrm[i].srcOffsetH;
        pCopyFillObj->dmaParam.a_b_cnt = (uint32_t)(pCopyFillObj->transferPrm[i].copyHeight<<16)|
                                          pCopyFillObj->transferPrm[i].copyWidth;
      } else {

        aCnt = pCopyFillObj->transferPrm[i].skipH;
        bCnt = (pCopyFillObj->transferPrm[i].copyWidth)/aCnt;

        pCopyFillObj->dmaParam.link_bcntrld = DMA_PARAM_NO_LINK | (bCnt << 16);
        pCopyFillObj->dmaParam.src_dst_cidx = (pCopyFillObj->transferPrm[i].dstOffsetH<<16)|pCopyFillObj->transferPrm[i].srcOffsetH;
        pCopyFillObj->dmaParam.ccnt = pCopyFillObj->transferPrm[i].copyHeight;
        
        pCopyFillObj->dmaParam.opt |= DMA_PARAM_OPT_ITCCHEN;
    
        pCopyFillObj->dmaParam.src_dst_bidx = ((aCnt*2) << 16) | (aCnt);
        
        pCopyFillObj->dmaParam.a_b_cnt = (uint32_t)(bCnt<<16)|aCnt;
      }
    }
  
    edma_write_slot( pCopyFillObj->chId, &pCopyFillObj->dmaParam );
    
    #ifdef DMA_DEBUG
    DMA_dumpPARAM("CopyFill2D", pCopyFillObj->chId);
    #endif
    
    #if 1
    edma_start(pCopyFillObj->chId);

    wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
    INIT_COMPLETION(pCopyFillObj->dma_complete);
    #endif
  }
  
  return 0;
}

	void yuyv_yuv420p(int w, int h, unsigned char *inbuf, unsigned char *outbuf)
	{
	int i,j;
#if 1
	for(j=0;j<h;j++){
		for(i=0;i<w;i++){
		//Y
		*(outbuf+j*w+i)=*(inbuf+j*2*w+i*2);
		}
	}
	
	
	//U
	for(j=0;j<h/2;j++){
	for(i=0;i<w/2;i++){
	*(outbuf+h*w+j*(w/2)+i)=*(inbuf+j*4*w+i*4+1);
	
	}
	}
	
	//V
	for(j=0;j<h/2;j++){
	for(i=0;i<w/2;i++){
	*(outbuf+h*w+h*w/4+j*(w/2)+i)=*(inbuf+j*4*w+i*4+3);
	}
	}
#endif
	}

#define STATIC_SHIFT                3
#define TCINTEN_SHIFT               20
#define ITCINTEN_SHIFT              21
#define TCCHEN_SHIFT                22
#define ITCCHEN_SHIFT               23

int DMA_copyYUYVtoYUV420p(  DMA_CopyFillObj *pCopyFillObj)
{
  int i,j, status, k;
  unsigned int aCnt, bCnt;
  int w,h;
  uint32_t src,dst;
  uint32_t srcindex, dstindex;
  struct edmacc_param param_set;  
  for(k=0; k<pCopyFillObj->count; k++) {
    w=pCopyFillObj->transferPrm[i].copyWidth;
	h=pCopyFillObj->transferPrm[i].copyHeight;
	src=(uint32_t) pCopyFillObj->transferPrm[i].srcPhysAddr;
	dst=(uint32_t) pCopyFillObj->transferPrm[i].dstPhysAddr;
	srcindex = (uint32_t) pCopyFillObj->transferPrm[i].srcOffsetH;
	dstindex = (uint32_t) pCopyFillObj->transferPrm[i].dstOffsetH;
	
	pCopyFillObj->tranCount = 0;
	pCopyFillObj->numTrans = h;
	edma_set_src (pCopyFillObj->chId, (unsigned long)(src), INCR, W8BIT);	
	edma_set_dest (pCopyFillObj->chId, (unsigned long)(dst), INCR, W8BIT);	
	edma_set_src_index (pCopyFillObj->chId, 2, srcindex);	
	edma_set_dest_index (pCopyFillObj->chId, 1, dstindex);	
	edma_set_transfer_params (pCopyFillObj->chId, 1, w, h, w, ABSYNC);	/* Enable the Interrupts on Channel 1 */	
	edma_read_slot (pCopyFillObj->chId, &param_set);	
	
	param_set.opt |= (1 << TCINTEN_SHIFT);	
//	param_set.opt |= (1 << ITCINTEN_SHIFT);	
//	param_set.opt |= (1 << TCCHEN_SHIFT);	
	param_set.opt |= (1 << ITCCHEN_SHIFT);
	
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(pCopyFillObj->chId));	
	edma_write_slot(pCopyFillObj->chId, &param_set);

    #ifdef DMA_DEBUG
    DMA_dumpPARAM("CopyYUYVtoYUV420p, Y", pCopyFillObj->chId);
    #endif
    
    #if 1
    edma_start(pCopyFillObj->chId);
    wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
    INIT_COMPLETION(pCopyFillObj->dma_complete);
    #endif

#if 0	
//copy U----------------------
	pCopyFillObj->tranCount = 0;
	pCopyFillObj->numTrans = h/2;

	edma_set_src (pCopyFillObj->chId, (unsigned long)(src+1), INCR, W8BIT);	
	edma_set_dest (pCopyFillObj->chId, (unsigned long)(dst+w*h), INCR, W8BIT);	
	edma_set_src_index (pCopyFillObj->chId, 4, 4*w);	
	edma_set_dest_index (pCopyFillObj->chId, 1, w/2);	
	edma_set_transfer_params (pCopyFillObj->chId, 1, w/2, h/2, w/2, ABSYNC);	/* Enable the Interrupts on Channel 1 */	
	edma_read_slot (pCopyFillObj->chId, &param_set);	
//	param_set.opt |= (1 << ITCCHEN_SHIFT);
	param_set.opt |= (1 << ITCINTEN_SHIFT);	
	param_set.opt |= (1 << TCINTEN_SHIFT);	
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(pCopyFillObj->chId));	
	edma_write_slot(pCopyFillObj->chId, &param_set);

#ifdef DMA_DEBUG
    DMA_dumpPARAM("CopyYUYVtoYUV420p, U", pCopyFillObj->chId);
#endif

    edma_start(pCopyFillObj->chId);
    wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
    INIT_COMPLETION(pCopyFillObj->dma_complete);
	
//copy V---------------------
	pCopyFillObj->tranCount = 0;
	pCopyFillObj->numTrans = h/2;
	edma_set_src (pCopyFillObj->chId, (unsigned long)(src+3), INCR, W8BIT); 
	edma_set_dest (pCopyFillObj->chId, (unsigned long)(dst+w*h+(w*h/4)), INCR, W8BIT);	
	edma_set_src_index (pCopyFillObj->chId, 4, 4*w);	
	edma_set_dest_index (pCopyFillObj->chId, 1, w/2);	
	edma_set_transfer_params (pCopyFillObj->chId, 1, w/2, h/2, w/2, ABSYNC);	/* Enable the Interrupts on Channel 1 */	
	edma_read_slot (pCopyFillObj->chId, &param_set);	
	//param_set.opt |= (1 << ITCCHEN_SHIFT);
	param_set.opt |= (1 << ITCINTEN_SHIFT); 
	param_set.opt |= (1 << TCINTEN_SHIFT);	
	param_set.opt |= EDMA_TCC(EDMA_CHAN_SLOT(pCopyFillObj->chId));	
	edma_write_slot(pCopyFillObj->chId, &param_set);

#ifdef DMA_DEBUG
	DMA_dumpPARAM("CopyYUYVtoYUV420p, V", pCopyFillObj->chId);
#endif

	edma_start(pCopyFillObj->chId);
	wait_for_completion_interruptible(&pCopyFillObj->dma_complete);
	INIT_COMPLETION(pCopyFillObj->dma_complete);
#endif	
  }
  
  return 0;
}
int DMA_copyFillRun( DMA_CopyFillPrm *pUserPrm )
{
  DMA_CopyFillPrm kernelPrm;
  DMA_Obj *pObj;
  DMA_CopyFillObj *pCopyFillObj;
  OSA_DmaCopy1D copy1D;
  OSA_DmaFill1D fill1D;
  OSA_DmaCopy2D copy2D;
  OSA_DmaFill2D fill2D;    
  OSA_DmaYUYVtoYUV420p copyYUV;
   
  int status, i;
    
  status = copy_from_user(&kernelPrm, pUserPrm, sizeof(kernelPrm));
  if(status < 0) {
    FUNCERR( "copy_from_user()\n" );          
    return status;
  }
  
  if(kernelPrm.chId >= DMA_DEV_MAX_CH) {
    FUNCERR( "Illegal parameter (chId = %d)\n", kernelPrm.chId );        
    return -1;
  }
      
  pObj = gDMA_dev.pObj[kernelPrm.chId];
  if(pObj==NULL) {
    FUNCERR( "Illegal parameter (chId = %d)\n", kernelPrm.chId );          
    return -1;
  }
  
  pCopyFillObj = pObj->copyFillObj;
  if(pCopyFillObj==NULL) {
    FUNCERR( "Illegal parameter (chId = %d)\n", kernelPrm.chId );          
    return -1;
  }
    
  if(kernelPrm.count>pCopyFillObj->maxCount) {
    FUNCERR( "Illegal parameter (chId = %d)\n", kernelPrm.chId );          
    return -1;
  }
    
  pCopyFillObj->count = kernelPrm.count;
  pCopyFillObj->curCount = 0;
  
  pCopyFillObj->copyFillType = kernelPrm.copyFillType;
  for(i=0; i<pCopyFillObj->count; i++)
  {
    switch(pCopyFillObj->copyFillType)
    {
      case DMA_CMD_COPY_1D:
        status = copy_from_user(&copy1D, kernelPrm.prm, sizeof(copy1D));
        if(status < 0) {
          FUNCERR( "copy_from_user() (chId=%d)\n", kernelPrm.chId );                  
          return status;
        }
          
        kernelPrm.prm += sizeof(copy1D);
        
        pCopyFillObj->transferPrm[i].srcPhysAddr = copy1D.srcPhysAddr;
        pCopyFillObj->transferPrm[i].dstPhysAddr = copy1D.dstPhysAddr;  
        pCopyFillObj->transferPrm[i].copyWidth = 0;
        pCopyFillObj->transferPrm[i].copyHeight = 0;  
        pCopyFillObj->transferPrm[i].srcOffsetH = 0;
        pCopyFillObj->transferPrm[i].dstOffsetH = 0;  
        pCopyFillObj->transferPrm[i].skipH = 0;                    
        pCopyFillObj->transferPrm[i].dstVirtAddr = 0;                    
        pCopyFillObj->transferPrm[i].fillValue = 0;                      
        pCopyFillObj->transferPrm[i].size = copy1D.size;
        break;
    case DMA_CMD_COPY_YUYV_YUV420P:
        status = copy_from_user(&copyYUV, kernelPrm.prm, sizeof(copyYUV));
        if(status < 0) {
          FUNCERR( "copy_from_user() (chId=%d)\n", kernelPrm.chId );                  
          return status;
        }
          
        kernelPrm.prm += sizeof(copyYUV);
        
        pCopyFillObj->transferPrm[i].srcPhysAddr = copyYUV.srcPhysAddr;
        pCopyFillObj->transferPrm[i].dstPhysAddr = copyYUV.dstPhysAddr;  
        pCopyFillObj->transferPrm[i].copyWidth = copyYUV.imageWidth;
        pCopyFillObj->transferPrm[i].copyHeight = copyYUV.imageHeight;  
        pCopyFillObj->transferPrm[i].srcOffsetH = copyYUV.srcOffsetH;
        pCopyFillObj->transferPrm[i].dstOffsetH = copyYUV.dstOffsetH;  
        pCopyFillObj->transferPrm[i].skipH = 0;                    
        pCopyFillObj->transferPrm[i].dstVirtAddr = 0;                    
        pCopyFillObj->transferPrm[i].fillValue = 0;                      
        pCopyFillObj->transferPrm[i].size = 0;
        break;
            
      case DMA_CMD_COPY_2D:
        status = copy_from_user(&copy2D, kernelPrm.prm, sizeof(copy2D));
        if(status < 0) {
          FUNCERR( "copy_from_user() (chId=%d)\n", kernelPrm.chId );                          
          return status;
        }
          
        kernelPrm.prm += sizeof(copy2D);
        
        pCopyFillObj->transferPrm[i].srcPhysAddr = copy2D.srcPhysAddr;
        pCopyFillObj->transferPrm[i].dstPhysAddr = copy2D.dstPhysAddr;  
        pCopyFillObj->transferPrm[i].copyWidth = copy2D.copyWidth;
        pCopyFillObj->transferPrm[i].copyHeight = copy2D.copyHeight;  
        pCopyFillObj->transferPrm[i].srcOffsetH = copy2D.srcOffsetH;
        pCopyFillObj->transferPrm[i].dstOffsetH = copy2D.dstOffsetH;  
        pCopyFillObj->transferPrm[i].skipH = copy2D.skipH;                    
        pCopyFillObj->transferPrm[i].dstVirtAddr = 0;                    
        pCopyFillObj->transferPrm[i].fillValue = 0;                      
        pCopyFillObj->transferPrm[i].size = 0;
        break;

      case DMA_CMD_FILL_1D:
        status = copy_from_user(&fill1D, kernelPrm.prm, sizeof(fill1D));
        if(status < 0) {
          FUNCERR( "copy_from_user() (chId=%d)\n", kernelPrm.chId );                          
          return status;
        }
          
        kernelPrm.prm += sizeof(fill1D);
        
        pCopyFillObj->transferPrm[i].srcPhysAddr = 0;
        pCopyFillObj->transferPrm[i].dstPhysAddr = fill1D.dstPhysAddr;  
        pCopyFillObj->transferPrm[i].copyWidth = 0;
        pCopyFillObj->transferPrm[i].copyHeight = 0;  
        pCopyFillObj->transferPrm[i].srcOffsetH = 0;
        pCopyFillObj->transferPrm[i].dstOffsetH = 0;  
        pCopyFillObj->transferPrm[i].skipH = 0;                    
        pCopyFillObj->transferPrm[i].dstVirtAddr = fill1D.dstVirtAddr;                    
        pCopyFillObj->transferPrm[i].fillValue = fill1D.fillValue;                      
        pCopyFillObj->transferPrm[i].size = fill1D.size;
        break;
    
      case DMA_CMD_FILL_2D:
        status = copy_from_user(&fill2D, kernelPrm.prm, sizeof(fill2D));
        if(status < 0) {
          FUNCERR( "copy_from_user() (chId=%d)\n", kernelPrm.chId );                          
          return status;
        }
          
        kernelPrm.prm += sizeof(fill2D);
        
        pCopyFillObj->transferPrm[i].srcPhysAddr = 0;
        pCopyFillObj->transferPrm[i].dstPhysAddr = fill2D.dstPhysAddr;  
        pCopyFillObj->transferPrm[i].copyWidth = fill2D.copyWidth;
        pCopyFillObj->transferPrm[i].copyHeight = fill2D.copyHeight;  
        pCopyFillObj->transferPrm[i].srcOffsetH = 0;
        pCopyFillObj->transferPrm[i].dstOffsetH = fill2D.dstOffsetH;  
        pCopyFillObj->transferPrm[i].skipH = 0;                    
        pCopyFillObj->transferPrm[i].dstVirtAddr = fill2D.dstVirtAddr;                    
        pCopyFillObj->transferPrm[i].fillValue = fill2D.fillValue;                      
        pCopyFillObj->transferPrm[i].size = 0;
        break;
      default:
        return -1;        
    }
  }    


  switch(pCopyFillObj->copyFillType) {
    case DMA_CMD_COPY_2D:  
    case DMA_CMD_FILL_2D:      
      status = DMA_copyFill2D(pCopyFillObj);
      break;
    case DMA_CMD_COPY_1D:        
    case DMA_CMD_FILL_1D:    
      status = DMA_copyFill1D(pCopyFillObj);
      break;
    case DMA_CMD_COPY_YUYV_YUV420P:      
      status = DMA_copyYUYVtoYUV420p(pCopyFillObj);
      break;               
    default:
      return -1;
  }
  
  return status ;
}

static void DMA_copyFillCallback(unsigned channel, u16 ch_status, void *data)
{
  DMA_CopyFillObj *pCopyFillObj=(DMA_CopyFillObj *)data;
  
 // if(pCopyFillObj->copyFillType == DMA_CMD_COPY_YUYV_YUV420P)
 // 	{
 // 	    pCopyFillObj->tranCount++;
//		if ( pCopyFillObj->tranCount < pCopyFillObj->numTrans) {
//	        edma_start(pCopyFillObj->chId);	  
//		} else {
//			edma_stop(pCopyFillObj->chId);
//			complete(&pCopyFillObj->dma_complete);
//		}
 // 	}
 // else{
  
  edma_stop(pCopyFillObj->chId);
  complete(&pCopyFillObj->dma_complete);
  //	}
}

int DMA_copyFillCreate(DMA_OpenClosePrm *prm)
{
  DMA_Obj *pObj;

#ifdef DMA_DEBUG
  printk(KERN_INFO "DMA: DMA_copyFillCreate() ... \n");
#endif

 
  pObj = kmalloc(sizeof(DMA_Obj), GFP_KERNEL);
  if(pObj==NULL) {
    FUNCERR( "Unable to alloc memory\n" );    
    return -1;
  }
  
  pObj->dmaMode = prm->mode;
  
  pObj->copyFillObj = kmalloc(sizeof(DMA_CopyFillObj), GFP_KERNEL);
  if(pObj->copyFillObj==NULL)
  {
    FUNCERR( "Unable to alloc memory\n" );    
    kfree(pObj);
    return -1;
  }
  
  memset(pObj->copyFillObj, 0, sizeof(DMA_CopyFillObj));
  
  pObj->copyFillObj->transferPrm = kmalloc(sizeof(DRV_CopyFillPrm)*prm->maxTransfers, GFP_KERNEL);
  if(pObj->copyFillObj->transferPrm==NULL)
  {
    FUNCERR( "Unable to alloc memory\n" );  
    kfree(pObj->copyFillObj);
    kfree(pObj);
    return -1;
  }
  
  pObj->copyFillObj->maxCount = prm->maxTransfers;  
  
  memset(pObj->copyFillObj->transferPrm, 0, sizeof(DRV_CopyFillPrm)*prm->maxTransfers);
  
  init_completion(&pObj->copyFillObj->dma_complete);  
  
  pObj->copyFillObj->chId = edma_alloc_channel(EDMA_CHANNEL_ANY, DMA_copyFillCallback, 
                          pObj->copyFillObj, DMA_NORMAL_EVENT_QUEUE)  ;
                          
  if (pObj->copyFillObj->chId<0 || pObj->copyFillObj->chId >= DMA_DEV_MAX_CH)
  {
    FUNCERR( "Unable to request DMA (channel=%d).\n", pObj->copyFillObj->chId );
    kfree(pObj->copyFillObj->transferPrm);
    kfree(pObj->copyFillObj);
    kfree(pObj);    
    return -EFAULT;
  }
  
  if(pObj->copyFillObj->chId >= DMA_DEV_MAX_CH) {
  
    FUNCERR( "Invalid channel ID (channel=%d).\n", pObj->copyFillObj->chId );
    edma_free_channel( pObj->copyFillObj->chId );    
    kfree(pObj->copyFillObj->transferPrm);
    kfree(pObj->copyFillObj);
    kfree(pObj);    
    
    return -EFAULT;
  }

  gDMA_dev.pObj[pObj->copyFillObj->chId] = pObj;
  
  prm->chId = pObj->copyFillObj->chId;
  
#ifdef DMA_DEBUG
  printk(KERN_INFO "DMA: DMA_copyFillCreate() success ... chId is %d\n", prm->chId);
#endif
  
  return 0;
}

int DMA_copyFillDelete(DMA_OpenClosePrm *prm)
{
  DMA_Obj *pObj;
  
  if(prm->chId >= DMA_DEV_MAX_CH)
    return -1;
    
  pObj = gDMA_dev.pObj[prm->chId];
  if(pObj==NULL) {
    FUNCERR( "Illegal parameter (chId = %d)\n", prm->chId );      
    return -1;
  }
    
  if(pObj->copyFillObj==NULL)  {
    FUNCERR( "Illegal parameter (chId = %d)\n", prm->chId );      
    return -1;
  }
    
  edma_stop( pObj->copyFillObj->chId );
  edma_free_channel( pObj->copyFillObj->chId );

  if(pObj->copyFillObj->transferPrm!=NULL)
    kfree(pObj->copyFillObj->transferPrm);
  kfree(pObj->copyFillObj);
  kfree(pObj);  
  
  gDMA_dev.pObj[prm->chId] = NULL;  

  return 0;
}


