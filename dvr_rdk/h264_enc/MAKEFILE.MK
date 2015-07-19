
demo:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK apps TARGET=so
	$(MAKE) -fMAKEFILE.MK apps TARGET=exe	

libs:
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_enc/common MODULE=h264_enc $(TARGET)
apps:
	$(MAKE) -fMAKEFILE.MK -C$(dvr_rdk_PATH)/h264_enc/common MODULE=h264_enc $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

all: clean demo

