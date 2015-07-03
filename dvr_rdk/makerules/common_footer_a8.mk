# (c) Texas Instruments 

ifndef $(COMMON_FOOTER_MK)
COMMON_FOOTER_MK = 1

OBJS=$(subst .c,.o, $(FILES) )

OBJ_DIR=$(OBJ_BASE_DIR)/$(MODULE)/$(CONFIG)

vpath %.o $(OBJ_DIR) 

LIB = $(MODULE).a

EXE = $(EXE_BASE_DIR)/$(CONFIG)/$(MODULE).out

SO_LIB= $(EXE_BASE_DIR)/../lib/lib$(MODULE).so

.c.o:
	@echo \# $(MODULE): $(PLATFORM): Compiling $< 
	$(CC) $(CC_OPTS) $(OPTI_OPTS) $(DEFINE) $(INCLUDE) -o$(OBJ_DIR)/$@ $<


lib : $(LIB_DIR)/$(LIB)

$(LIB_DIR)/$(LIB) : $(OBJS)
	@echo \# $(MODULE): $(PLATFORM): Creating archive $(LIB)
	$(AR)	$(AR_OPTS) $(LIB_DIR)/$(LIB) $(OBJ_DIR)/*.o

obj: $(OBJS)
		
clean:
	@echo \# $(MODULE): $(PLATFORM): Deleting temporary files   
	-rm -f $(LIB_DIR)/$(LIB) 
	-rm -f $(OBJ_DIR)/*.*
	
depend:
#	@echo \# $(MODULE): $(PLATFORM): Making Directories, if not already created
	-mkdir -p $(LIB_DIR)	
	-mkdir -p $(OBJ_DIR)
	-mkdir -p $(EXE_BASE_DIR)	
	@echo \# $(MODULE): $(PLATFORM): Building dependancies
	$(CC)	$(DEFINE) $(INCLUDE) $(FILES) -M > MAKEFILE.DEPEND

so:
	@echo \# $(MODULE): $(PLATFORM): Linking to .so
	$(LD) $(LD_OPTS) -shared -o $(SO_LIB) $(OBJ_DIR)/*.o $(LIBS)
	@echo \# Final shared library $(SO_LIB) !!!	
	@echo \# 	
exe:
	@echo \# $(MODULE): $(PLATFORM): Linking
	$(LD) $(LD_OPTS) -o$(EXE) $(LIBS)
	@echo \# Final executable $(EXE) !!!	
	@echo \# 
	
install:
	
			 
-include MAKEFILE.DEPEND  

endif # ifndef $(COMMON_FOOTER_MK)

