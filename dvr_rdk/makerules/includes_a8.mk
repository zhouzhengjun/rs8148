# (c) Texas Instruments

ifndef $(INCLUDES_MK)
INCLUDES_MK = 1

OSA_INC=-I$(dvr_rdk_PATH)/mcfw/src_linux/osa/inc

DEVICES_INC=-I$(dvr_rdk_PATH)/mcfw/src_linux/devices/inc -I$(dvr_rdk_PATH)/mcfw/src_linux/devices/tvp5158/inc -I$(dvr_rdk_PATH)/mcfw/src_linux/devices/sii9135/inc -I$(dvr_rdk_PATH)/mcfw/src_linux/devices/sii9022a/inc  -I$(dvr_rdk_PATH)/mcfw/src_linux/devices/thsfilters/inc

OSA_KERMOD_INC=-I$(dvr_rdk_PATH)/mcfw/src_linux/osa_kermod/inc

SYSLINK_INC=-I$(syslink_PATH)/packages -I$(ipc_PATH)/packages

CODEC_INC=-I$(xdais_PATH)/packages -I$(h264dec_PATH)/Inc -I$(mpeg4dec_PATH)/Inc -I$(h264enc_PATH)/Inc -I$(jpegdec_PATH)/Inc -I$(jpegenc_PATH)/Inc

LINUXDEVKIT_INC=-I$(linuxdevkit_PATH)/usr/include 

RPE_INC=-I$(rpe_PATH)/include -I$(rpe_PATH)/src/include -I$(aaclcdec_PATH)/packages -I$(aaclcenc_PATH)/packages -I$(xdais_PATH)/packages

MCFW_INC=-I$(dvr_rdk_PATH)/mcfw/interfaces -I$(dvr_rdk_PATH)/mcfw/interfaces/common_def 

COMMON_INC=-I../inc -I../priv -I. -I$(dvr_rdk_PATH) 

SYSLINK_LIBS=$(syslink_PATH)/packages/ti/syslink/lib/syslink.a_release

AUDIO_LIBS=$(linuxdevkit_PATH)/usr/lib/libasound.so.2

RPE_LIBS=$(rpe_PATH)/lib/lib/a8/debug/rpe.av5T

MCFW_LIBS=$(LIB_DIR)/dvr_rdk_mcfw_api.a $(LIB_DIR)/dvr_rdk_link_api.a $(LIB_DIR)/dvr_rdk_osa.a $(SYSLINK_LIBS)

endif # ifndef $(INCLUDES_MK)

