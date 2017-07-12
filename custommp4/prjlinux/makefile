CCILINUX_eti_CC    := arm_v5t_le-gcc
CCILINUX_eti_CPP   := arm_v5t_le-g++
CCILINUX_eti_LD    := arm_v5t_le-ld
CCILINUX_eti_AR    := arm_v5t_le-ar
CCILINUX_eti_STRIP := arm_v5t_le-strip

INC_DIR := ../include
SRC_DIR := ../source

LIB_TARGET := libcustommp4.a
INSTALL_LIB_PATH := ../../common/lib/debug

CFLAGS += -Wall -fno-strict-aliasing -march=armv5t

#CFLAGS += -Wall -fno-strict-aliasing -march=armv5t -mabi=apcs-gnu

#CFLAGS += -D__DEBUG

LINUXKERNEL_INSTALL_DIR=/home/user/workdir/lsp/ti-davinci		
INC_PATH += $(INC_DIR) ../../common/include $(LINUXKERNEL_INSTALL_DIR)/include

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=$(SRC_DIR)/debug/custommp4					\
				$(SRC_DIR)/debug/util							\
				$(SRC_DIR)/debug/partitionindex		\
				$(SRC_DIR)/debug/diskmanage				\

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)
$(LIB_TARGET) : $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
	install -D -m 644 $(LIB_TARGET) $(INSTALL_LIB_PATH)/$(LIB_TARGET)
	cp $(LIB_TARGET) /home/user/dvevm_1_00_00_32/codec_engine_1_00/examples/apps/mpeg4/dualcpu/lib/
	
clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)


