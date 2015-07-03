

iss:
	cp Rules_iss.make $(iss_PATH)/
	make -C $(iss_PATH) iss
#	make -C $(iss_PATH) timmosal
#	make -C $(iss_PATH) imx
#	make -C $(iss_PATH) swosd
#	make -C $(iss_PATH) ip_run2
#	make -C $(iss_PATH) ip_run2_sched
#	make -C $(iss_PATH) imx_core
#	make -C $(iss_PATH) scd
	make -f build_iss.mk iss_install 
	
iss_install:
	-mkdir -p $(DVR_RDK_ISS_INC_PATH)
	-mkdir -p $(DVR_RDK_ISS_LIB_PATH)
	-mkdir -p $(DVR_RDK_ISS_INC_PATH)/ti/psp/iss/alg/evf/inc
	cp $(iss_PATH)/packages/ti/psp/iss/alg/evf/inc/cpisCore.h $(DVR_RDK_ISS_INC_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/swosd/inc/sw_osd.h $(DVR_RDK_ISS_INC_PATH)/
#	cp $(iss_PATH)/packages/ti/psp/iss/alg/scd/inc/simcop_scd.h $(DVR_RDK_ISS_INC_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/drivers/iss_init.h $(DVR_RDK_ISS_INC_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/evf/inc/tistdtypes.h $(DVR_RDK_ISS_INC_PATH)/ti/psp/iss/alg/evf/inc
	cp $(iss_PATH)/build/ti/psp/iss/lib/ti814x-evm/m3/release/iss_drivers.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/build/ti/psp/platforms/lib/ti814x-evm/m3/release/iss_platform.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/build/ti/psp/devices/lib/ti814x-evm/m3/release/iss_devices.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	
	cp $(iss_PATH)/packages/ti/psp/iss/alg/evf/lib/release/*.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/imx/lib/release/iss_imx.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/ip_run2/lib/release/iss_ip_run2_bios.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/ip_run2_sched/lib/release/iss_ip_run2_sched.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/timmosal/lib/release/iss_timmosal.aem3 $(DVR_RDK_ISS_LIB_PATH)/
	cp $(iss_PATH)/packages/ti/psp/iss/alg/swosd/lib/release/iss_swosd.aem3 $(DVR_RDK_ISS_LIB_PATH)/swosd.aem3
#	cp $(iss_PATH)/packages/ti/psp/iss/alg/scd/lib/release/scd.aem3 $(DVR_RDK_ISS_LIB_PATH)/

iss_clean:
	rm -rf $(iss_PATH)/build