
SRCS_COMMON += utils_que.c utils_buf.c utils_mbx.c utils_tsk.c \
		utils_prf.c utils_mem.c utils.c utils_bit_buf.c utils_buf_ext.c \
        
	
		 
SRCS_m3video += remote_debug_server_m3video.c utils_execp_trace.c

SRCS_m3vpss += utils_tiler.c utils_tiler_allocator.c utils_dma.c utils_dma_bios6.c utils_dma_m3vpss_cfg.c 
SRCS_m3vpss += utils_dmtimer.c utils_execp_trace.c FNPROF_HOOK.c

ifeq ($(SYSTEM_SIMCOP_ENABLE),yes)
SRCS_m3vpss += utils_dma_m3vpss_simcop_cfg.c
endif

SRCS_c6xdsp += remote_debug_server_c6xdsp.c utils_dma_c6xdsp_cfg.c utils_tiler.c