
include $(dvr_rdk_PATH)/makerules/common_header_a8.mk

INCLUDE+= $(OSA_INC) $(SYSLINK_INC) $(MCFW_INC) $(CODEC_INC) $(DEVICES_INC)

LIBS=$(LIB_DIR)/h264_dec.a $(MCFW_LIBS) $(AUDIO_LIBS)

include $(dvr_rdk_PATH)/makerules/common_footer_a8.mk


