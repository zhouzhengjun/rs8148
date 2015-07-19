# (c) Texas Instruments

include Rules.make

dvr_rdk: dvr_rdk_linux dvr_rdk_bios6 fsupdate

dvr_rdk_bios6_algs: 
	rm -rf build/lib/
	rm -rf build/obj/
	$(MAKE) -fMAKEFILE.MK -C $(dvr_rdk_PATH)/mcfw/src_bios6/alg/scd

clean: dvr_rdk_linux_clean dvr_rdk_bios6_clean

all: clean dvr_rdk

# demo application
dvr_rdk_linux:
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/mcfw/src_linux
#	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/rsdemo
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/avcap
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_enc
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_dec
dvr_rdk_linux_clean:
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/mcfw/src_linux clean
#	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/rsdemo clean
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/avcap clean
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_enc clean
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_dec clean
	
	
dvr_rdk_linux_all: dvr_rdk_linux_clean dvr_rdk_linux

dvr_rdk_bios6: 
	$(MAKE) -fMAKEFILE.MK -C $(dvr_rdk_PATH)/mcfw//src_bios6 $(TARGET)

dvr_rdk_bios6_clean:
	$(MAKE) -fMAKEFILE.MK -C $(dvr_rdk_PATH)/mcfw//src_bios6 clean

dvr_rdk_bios6_all: dvr_rdk_bios6_clean dvr_rdk_bios6
#---------------------------------------------------------------------
# dvr application with QT
dvrlib:
	$(MAKE) -fMAKEFILE.MK -C $(dvr_rdk_PATH)/mcfw/src_linux
	$(MAKE) -fMAKEFILE.MK -C $(dvrapp_PATH)/app

dvrlib_clean:
	$(MAKE) -fMAKEFILE.MK -C $(dvrapp_PATH)/app clean

dvrgui:
ifeq (,$(qt_PATH))
	@echo -e "\n [info] dvrgui need path(qt_PATH) for QT4.6.2 ---\n"
else
	cd $(dvrapp_PATH)/dvrgui; sh qmake.sh;
	$(MAKE) -fMAKEFILE.MK -C $(dvrapp_PATH)/dvrgui
endif

dvrgui_clean:
	$(MAKE) -fMAKEFILE.MK -C $(dvrapp_PATH)/dvrgui clean

dvrapp: dvrlib dvrgui fsupdate
	@echo -e "\n--- $(MAKE) dvrapp done! (`date +'%H:%M:%S'`)---\n"

dvrapp_clean: dvrlib_clean dvrgui_clean
dvrapp_all: dvrapp_clean dvrapp

#-----------------------------------------------------------------------------------------------------------------
# HDVENC_OUTPUT_SWAP variable has been added to hdvpss build rule for initial revisions of DM8107 DVR and alpha 
# versions of DM8107 EVM. This ensures right colors are seen on VGA output. The issue with color swapping should 
# be fixed in later revisions of DM8107 DVR and in beta versions of DM8107 EVM.
#
# If the DVR_RDK_BOARD_TYPE is DM8107_EVM and the evm is of alpha version, ensure HDVENC_OUTPUT_SWAP=1  
#-----------------------------------------------------------------------------------------------------------------

hdvpss:
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/vps $(TARGET) CORE=m3vpss HDVENC_OUTPUT_SWAP=1
else
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/vps $(TARGET) CORE=m3vpss HDVENC_OUTPUT_SWAP=0
endif
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/i2c $(TARGET) CORE=m3vpss
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/devices $(TARGET) CORE=m3vpss
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/platforms $(TARGET) CORE=m3vpss
	$(MAKE) -C $(hdvpss_PATH)/packages/ti/psp/proxyServer $(TARGET) CORE=m3vpss

hdvpss_clean:
	$(MAKE) hdvpss TARGET=clean

hdvpss_all: hdvpss_clean hdvpss

rpe:
	$(MAKE) -C $(rpe_PATH) $(RPE_BUILD_VARS) $(TARGET)

rpe_clean:
	$(MAKE) -C $(rpe_PATH) $(RPE_BUILD_VARS) clean

rpe_all: rpe_clean rpe

iss:
	$(MAKE) -C ./makerules -f build_iss.mk iss 
	$(MAKE) -C ./makerules -f build_iss.mk iss_install

iss_install:
	$(MAKE) -C ./makerules -f build_iss.mk iss_install 

iss_clean:
	$(MAKE) -C ./makerules -f build_iss.mk iss_clean 

iss_all: iss_clean iss

uboot_build:
	$(MAKE) -C$(UBOOTDIR) CROSS_COMPILE=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi- ARCH=arm $(TARGET)

uboot_build_cent:
	$(MAKE) -C$(UBOOTDIR) CROSS_COMPILE=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi- ARCH=arm

uboot_clean:
	$(MAKE) -C$(UBOOTDIR) CROSS_COMPILE=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi- ARCH=arm distclean

uboot:
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
	$(MAKE) uboot_build TARGET=ti8168_dvr_config
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.noxip.bin $(TFTP_HOME)/uboot_NAND_$(DVR_RDK_BOARD_TYPE)
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_EVM)
	$(MAKE) uboot_build TARGET=ti8168_evm_config
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.noxip.bin $(TFTP_HOME)/uboot_NAND_$(DVR_RDK_BOARD_TYPE)
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_min_uart
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.uart $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.uart
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_min_nand
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.nand $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.nand
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_config_nand
	$(MAKE) uboot_build_cent
	cp $(UBOOTDIR)/u-boot.bin $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).bin
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_min_uart
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.uart $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.uart
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_min_nand
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.nand $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.nand
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=ti8148_evm_config_nand
	$(MAKE) uboot_build_cent
	cp $(UBOOTDIR)/u-boot.bin $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).bin
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_min_sd
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.sd $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.sd	 
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_min_uart
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.uart $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.uart
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_min_nand
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.nand $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.nand
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_config_nand
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.bin $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).bin
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_config_sd
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.bin $(TFTP_HOME)/u-boot_SD_$(DVR_RDK_BOARD_TYPE).bin
endif

ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR) 
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_min_uart_dvr
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.uart $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.uart
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_min_nand_dvr
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.min.nand $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).min.nand
	$(MAKE) uboot_clean
	$(MAKE) uboot_build TARGET=dm385_evm_config_nand_dvr
	$(MAKE) uboot_build TARGET=u-boot.ti
	cp $(UBOOTDIR)/u-boot.bin $(TFTP_HOME)/u-boot_$(DVR_RDK_BOARD_TYPE).bin
endif

uboot_all:
	$(MAKE) uboot_clean
	$(MAKE) uboot

lsp_build:
	$(MAKE) -C$(KERNELDIR) CROSS_COMPILE=$(CODEGEN_PATH_A8)/bin/arm-arago-linux-gnueabi- ARCH=arm $(TARGET)

lsp:
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
	$(MAKE) lsp_build TARGET=ti8168_dvr_defconfig
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_EVM)
	$(MAKE) lsp_build TARGET=ti8168_evm_defconfig
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
	#$(MAKE) lsp_build TARGET=ti8148_evm_defconfig
	$(MAKE) lsp_build TARGET=ti8148_rs8148_debug_defconfig	
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
	$(MAKE) lsp_build TARGET=ti8148_evm_defconfig
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
	$(MAKE) lsp_build TARGET=dm385_evm_defconfig
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
	$(MAKE) lsp_build TARGET=ti810X_dvr_defconfig
endif
	$(MAKE) lsp_build TARGET=uImage
	$(MAKE) lsp_build TARGET=modules
	$(MAKE) lsp_build TARGET=headers_install
	-mkdir -p $(TFTP_HOME)/
	cp $(KERNELDIR)/arch/arm/boot/uImage $(TFTP_HOME)/uImage_rs8148
	-mkdir -p $(TARGET_FS_DIR)/kermod
	cp $(KERNELDIR)/drivers/video/ti81xx/vpss/vpss.ko $(TARGET_FS_DIR)/kermod/.
	cp $(KERNELDIR)/drivers/video/ti81xx/ti81xxfb/ti81xxfb.ko $(TARGET_FS_DIR)/kermod/.
	cp $(KERNELDIR)/drivers/video/ti81xx/ti81xxhdmi/ti81xxhdmi.ko $(TARGET_FS_DIR)/kermod/.

lsp_all: lsp_clean lsp

lsp_clean:
	$(MAKE) lsp_build TARGET=distclean

syslink_build:
	cp ./makerules/syslink_products.mak $(syslink_PATH)/products.mak
	$(MAKE) -C$(syslink_PATH) $(TARGET)

syslink:
ifeq ($(PLATFORM),ti816x-evm)
	$(MAKE) syslink_build DEVICE=TI816X TARGET=syslink	
	-mkdir -p $(TARGET_FS_DIR)/kermod
	cp $(syslink_PATH)/packages/ti/syslink/bin/TI816X/syslink.ko $(TARGET_FS_DIR)/kermod/.
endif    
ifeq ($(PLATFORM),ti814x-evm)	
	$(MAKE) syslink_build DEVICE=TI814X TARGET=syslink
	-mkdir -p $(TARGET_FS_DIR)/kermod
	cp $(syslink_PATH)/packages/ti/syslink/bin/TI814X/syslink.ko $(TARGET_FS_DIR)/kermod/.
endif
ifeq ($(PLATFORM),ti810x-evm)	
	$(MAKE) syslink_build DEVICE=TI814X TARGET=syslink
	-mkdir -p $(TARGET_FS_DIR)/kermod
	cp $(syslink_PATH)/packages/ti/syslink/bin/TI814X/syslink.ko $(TARGET_FS_DIR)/kermod/.
endif

syslink_clean:
ifeq ($(PLATFORM),ti816x-evm)
	$(MAKE) syslink_build DEVICE=TI816X TARGET=clean
endif    
ifeq ($(PLATFORM),ti814x-evm)	
	$(MAKE) syslink_build DEVICE=TI814X TARGET=clean
endif
ifeq ($(PLATFORM),ti810x-evm)	
	$(MAKE) syslink_build DEVICE=TI814X TARGET=clean
endif

syslink_all: syslink_clean syslink

jffs2_128:
	mkfs.jffs2 -lqn -e 128 -r $(TARGET_FS) -o  $(TFTP_HOME)/rfs_128_$(DVR_RDK_BOARD_TYPE).jffs2
jffs2_256:
	mkfs.jffs2 -lqn -e 256 -r $(TARGET_FS) -o  $(TFTP_HOME)/rfs_256_$(DVR_RDK_BOARD_TYPE).jffs2

fsupdate:
	@echo "-------- Copying binaries to [ $(TARGET_FS) ] --------"
	cp $(KERNELDIR)/drivers/video/ti81xx/vpss/vpss.ko $(TARGET_FS_DIR)/kermod/.
	cp $(KERNELDIR)/drivers/video/ti81xx/ti81xxfb/ti81xxfb.ko $(TARGET_FS_DIR)/kermod/.
	cp $(KERNELDIR)/drivers/video/ti81xx/ti81xxhdmi/ti81xxhdmi.ko $(TARGET_FS_DIR)/kermod/.
ifeq ($(PLATFORM),ti816x-evm)
	cp $(syslink_PATH)/packages/ti/syslink/bin/TI816X/syslink.ko $(TARGET_FS_DIR)/kermod/.
	-mkdir -p $(TARGET_FS)/opt/dvr_rdk/ti816x
	cp -R $(TARGET_FS_DIR)/* $(TARGET_FS)/opt/dvr_rdk/ti816x/.
	chmod 755 $(TARGET_FS)/opt/dvr_rdk/ti816x/*.sh
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti816x/firmware/dvr_rdk_fw_m3video.xem3
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti816x/firmware/dvr_rdk_fw_m3vpss.xem3
	$(STRIP6x)  $(TARGET_FS)/opt/dvr_rdk/ti816x/firmware/dvr_rdk_fw_c6xdsp.xe674
	chmod 755   $(TARGET_FS)/opt/dvr_rdk/ti816x/firmware/*.*
endif
ifeq ($(PLATFORM),ti814x-evm)
	cp $(syslink_PATH)/packages/ti/syslink/bin/TI814X/syslink.ko $(TARGET_FS_DIR)/kermod/.
	-mkdir -p $(TARGET_FS)/opt/dvr_rdk/ti814x
	cp -R $(TARGET_FS_DIR)/* $(TARGET_FS)/opt/dvr_rdk/ti814x/.
	chmod 755 $(TARGET_FS)/opt/dvr_rdk/ti814x/*.sh
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti814x/firmware/dvr_rdk_fw_m3video.xem3
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti814x/firmware/dvr_rdk_fw_m3vpss.xem3
	$(STRIP6x)  $(TARGET_FS)/opt/dvr_rdk/ti814x/firmware/dvr_rdk_fw_c6xdsp.xe674
	chmod 755   $(TARGET_FS)/opt/dvr_rdk/ti814x/firmware/*.*
endif
ifeq ($(PLATFORM),ti810x-evm)
	cp  $(syslink_PATH)/packages/ti/syslink/bin/TI814X/syslink.ko $(TARGET_FS_DIR)/kermod/.
	-mkdir -p $(TARGET_FS)/opt/dvr_rdk/ti810x
	cp -R $(TARGET_FS_DIR)/* $(TARGET_FS)/opt/dvr_rdk/ti810x/.
	chmod 755 $(TARGET_FS)/opt/dvr_rdk/ti810x/*.sh
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti810x/firmware/dvr_rdk_fw_m3video.xem3
	$(STRIP470) $(TARGET_FS)/opt/dvr_rdk/ti810x/firmware/dvr_rdk_fw_m3vpss.xem3
	chmod 755   $(TARGET_FS)/opt/dvr_rdk/ti810x/firmware/*.*
endif
	@echo "-------- Build Completed for $(DVR_RDK_BOARD_TYPE) --------"

nfsreset:
	/usr/sbin/exportfs -av
	/etc/init.d/nfs-kernel-server restart

sys: uboot lsp syslink hdvpss dvr_rdk

sys_clean: clean hdvpss_clean syslink_clean lsp_clean uboot_clean

sys_all: sys_clean sys


##### Code Checkers #####
DIR=mcfw

sc_indent: 
	$(SC_SCRIPTS_BASE_DIR)/SCIndent_RDK.pl --dir $(DIR)        

sc_check:
	$(SC_SCRIPTS_BASE_DIR)/SCCheckers_RDK.pl --dir $(DIR)

sc_insert:
	$(SC_SCRIPTS_BASE_DIR)/SCInsert_RDK.pl --dir $(DIR)


copy_ti_tools_common:
	-mkdir -p $(dvr_rdk_BASE)/target
	-mkdir -p $(dvr_rdk_BASE)/tftphome
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti816x_evm
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti816x_dvr
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti814x_evm
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti814x_dvr
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti810x_evm
	-mkdir -p $(dvr_rdk_BASE)/pre_built_binary/ti810x_dvr
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/hdvpss
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/bios
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/syslink
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/xdc
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/ipc
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/framework_components
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/xdais
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/codecs
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/ivahd_hdvicp
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_devkit
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/cgt_m3
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/cgt_dsp
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/cgt_a8
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/edma3lld
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/rpe
	cp -R $(bios_PATH) $(dvr_rdk_BASE)/ti_tools/bios/
	cp -R $(xdc_PATH) $(dvr_rdk_BASE)/ti_tools/xdc/
	cp -R $(ipc_PATH) $(dvr_rdk_BASE)/ti_tools/ipc/
	cp -R $(fc_PATH) $(dvr_rdk_BASE)/ti_tools/framework_components/
	cp -R $(xdais_PATH) $(dvr_rdk_BASE)/ti_tools/xdais/
	cp -R $(h264dec_DIR) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(mpeg4dec_DIR) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(jpegdec_DIR) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(h264enc_DIR) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(jpegenc_DIR) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(hdvicplib_PATH) $(dvr_rdk_BASE)/ti_tools/ivahd_hdvicp/
	cp -R $(CODEGEN_PATH_M3) $(dvr_rdk_BASE)/ti_tools/cgt_m3/
	cp -R $(CODEGEN_PATH_DSP) $(dvr_rdk_BASE)/ti_tools/cgt_dsp/
	cp -R $(linuxdevkit_PATH) $(dvr_rdk_BASE)/ti_tools/linux_devkit/
	cp -R $(edma3lld_PATH) $(dvr_rdk_BASE)/ti_tools/edma3lld/
	cp -R $(rpe_PATH) $(dvr_rdk_BASE)/ti_tools/rpe
	cp -R $(aaclcdec_PATH) $(dvr_rdk_BASE)/ti_tools/codecs/
	cp -R $(aaclcenc_PATH) $(dvr_rdk_BASE)/ti_tools/codecs/
	rm -rf $(dvr_rdk_BASE)/dvr_rdk/internal_docs
	rm -rf $(dvr_rdk_BASE)/dvr_rdk/.??*
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM8168_DVR_RDK_Install_Guide.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM8168_DVR_RDK_Quick_Start_Guide.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM8168_DVR_RDK_ReleaseNotes.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM814x_DVR_RDK_Install_Guide.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM810x_DVR_RDK_Install_Guide.pdf $(dvr_rdk_BASE)	
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM814x_DVR_RDK_Quick_Start_Guide.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM814x_DVR_RDK_ReleaseNotes.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/docs/DM810x_DVR_RDK_ReleaseNotes.pdf $(dvr_rdk_BASE)
	mv $(dvr_rdk_BASE)/dvr_rdk/bin/samplelogo_480p.bmp $(dvr_rdk_BASE)/tftphome
	mv $(dvr_rdk_BASE)/dvr_rdk/bin/samplelogo_720p.bmp $(dvr_rdk_BASE)/tftphome

copy_ti_tools_specific:
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_DVR)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/TI814X_DVR
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/TI814X_DVR
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_DVR)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM814X_EVM)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM810X_EVM)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_EVM)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/
endif
ifeq ($(DVR_RDK_BOARD_TYPE),DM816X_DVR)
	-mkdir -p $(dvr_rdk_BASE)/ti_tools/linux_lsp/
	cp -R $(LSPDIR) $(dvr_rdk_BASE)/ti_tools/linux_lsp/
endif
	cp -R $(hdvpss_PATH) $(dvr_rdk_BASE)/ti_tools/hdvpss/
	cp -R $(syslink_PATH) $(dvr_rdk_BASE)/ti_tools/syslink/
	mv $(dvr_rdk_BASE)/dvr_rdk/Rules.make.release $(dvr_rdk_BASE)/dvr_rdk/Rules.make
	
