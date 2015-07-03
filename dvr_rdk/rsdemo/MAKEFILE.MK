
demo:
	$(MAKE) -fMAKEFILE.MK libs

libs:
ifeq ($(PLATFORM),ti816x-evm)
	$(MAKE) -fMAKEFILE.MK -Clink_api_demos $(TARGET)
	$(MAKE) -fMAKEFILE.MK -Cmcfw_api_demos $(TARGET)
ifeq ($(DEMOTYPE),link_api_sd_demo)
	$(MAKE) -fMAKEFILE.MK -Clink_api_sd_demo $(TARGET)
endif
else
	$(MAKE) -fMAKEFILE.MK -Cmcfw_api_demos $(TARGET)
#	$(MAKE) -fMAKEFILE.MK -Ceyeballlite $(TARGET)
endif

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

all: clean demo

