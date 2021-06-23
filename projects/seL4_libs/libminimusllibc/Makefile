CC = gcc
CCFLAG = -Wall -g -O0
CCFLAGS += ${NK_CFLAGS}

ifeq (${SOURCE_DIR},)
		SOURCE_DIR = $(shell pwd)
endif

ifeq (${BUILD_DIR},)
		BUILD_DIR = $(shell pwd)
endif

LIBSRCDIR = ${SOURCE_DIR}/src
LIBOBJDIR = ${BUILD_DIR}/build/src
ALLSRC = $(wildcard ${LIBSRCDIR}/*.c)
LIBSRC = $(addprefix ${BUILD_DIR}/build/src/, $(notdir $(ALLSRC)))
LIBOBJS = $(patsubst %.c, %.o, $(LIBSRC))

all : build_dir ${BUILD_DIR}/build/libminimusllibc.a

build_dir :
		mkdir -p $(BUILD_DIR)/build
		cp -r $(SOURCE_DIR)/include $(BUILD_DIR)/build/include
		cp -r $(SOURCE_DIR)/src $(BUILD_DIR)/build/src
		echo $(LIBOBJDIR)

$(LIBOBJDIR)/%.o : $(LIBSRCDIR)/%.c
		$(CC) $(CCFLAG) -c -o $@ $<

${BUILD_DIR}/build/libminimusllibc.a : $(LIBOBJS)
		rm -f $@
		ar rc $@ $^
		ranlib $@

clean:
		rm -rf ${BUILD_DIR}/build

.PHONY: all clean install
