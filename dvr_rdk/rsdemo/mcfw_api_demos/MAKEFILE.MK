
demo:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK apps TARGET=exe

mcfw_demo:
ifeq ($(DISABLE_AUDIO),no)
	$(MAKE) -fMAKEFILE.MK -C../audio_sample MODULE=videoServer $(TARGET)
endif
#ifeq ($(PLATFORM),ti816x-evm)
	$(MAKE) -fMAKEFILE.MK -C../graphic MODULE=videoServer $(TARGET)
#endif	
	$(MAKE) -fMAKEFILE.MK -C../display_process MODULE=videoServer $(TARGET)
	#$(MAKE) -fMAKEFILE.MK lib -CBECodec MODULE=videoServer $(TARGET)

libs: mcfw_demo 

apps:
#	$(MAKE) -fMAKEFILE.MK -CBECodec MODULE=videoServer $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

all: clean demo 

.PHONY: libs apps clean demo all mcfw_demo 

