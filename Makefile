MAKE_FILE_VERSION=1.1
#export CC=gcc
BSP_DIR=~/AZM335X_LINUX_BSP_201303
ROOT_OUTPUT=$(BSP_DIR)/rootfs/buildroot/output
export CC=$(ROOT_OUTPUT)/host/usr/bin/arm-linux-gnueabihf-gcc
MAKE=make -w 
CP=cp -rf

ifdef ver
else
ver=1
endif

DFLAGS=
#DFLAGS += -D_MSG_QUEUE_VERSION=2
DFLAGS += -DHAVE_DECL_TIOCSRS485=1
DFLAGS += -DRTU_VERSION=$(ver)

export CFLAGS=-O2 -Wall
CFLAGS += -D_DEBUG_LEVER=0
CFLAGS += $(DFLAGS)

apppath = $(shell pwd)

export INC=-I$(apppath)
INC += -I$(apppath)/libmodbus-3.0.6/src
INC += -I$(ROOT_OUTPUT)/build/zlib-1.2.7
#INC += -I$(ROOT_OUTPUT)/build/sqlite-3071300
#INC += -I$(ROOT_OUTPUT)/build/cjson-undefined/cJSON

export LIBS=-lpthread
LIBS += -L$(ROOT_OUTPUT)/target/usr/lib
LIBS += -L$(ROOT_OUTPUT)/target/lib
LIBS += -lsqlite3
LIBS += -lcJSON
LIBS += -lz
LIBS += -lm
LIBS += -lrt
LIBS += -ldl
#LIBS += -Wl,-rpath=./lib

common=$(apppath)/common
newwork=$(apppath)/network
zigbee=$(apppath)/zigbee
modbus_485=$(apppath)/modbus_485
modbus_232=$(apppath)/modbus_232
libmodbus=$(apppath)/libmodbus-3.0.6/src
libmodbus_tests=$(apppath)/libmodbus-3.0.6/tests
data=$(apppath)/data
meter=$(apppath)/meter
gpio=$(apppath)/gpio
driver=$(apppath)/driver

OBJS=rtu_a118.o
TARGET=rtu_a118
SUBOBJS=${common}/*.o
SUBOBJS += ${newwork}/*.o
SUBOBJS += ${zigbee}/*.o
SUBOBJS += ${modbus_485}/*.o
SUBOBJS += ${modbus_232}/*.o
SUBOBJS += ${libmodbus}/*.o
SUBOBJS += ${data}/*.o
SUBOBJS += ${meter}/*.o
SUBOBJS += ${gpio}/*.o

all:#$(OBJS)
	$(CC) $(CFLAGS) $(INC) -c *.c
	$(MAKE) -f Makefile.rtu_a118 -C ${libmodbus}
	$(MAKE) -C ${common}
	$(MAKE) -C ${newwork}
	$(MAKE) -C ${zigbee}
	$(MAKE) -C ${modbus_485}
	$(MAKE) -C ${modbus_232}
	$(MAKE) -C ${data}
	$(MAKE) -C ${meter}
	$(MAKE) -C ${gpio}
	$(CC) ${CFLAGS} -o ${TARGET} *.o ${SUBOBJS} ${LIBS}
	cp -rf ${TARGET} image

%.o:%.c
	$(CC) -c ${CFLAGS} ${INC} -o $@ $^
	
drivers:
	$(MAKE) -C $(driver)

clean:
	rm -f ${TARGET} *.o
	$(MAKE) -f Makefile.rtu_a118 -C ${libmodbus} clean
	$(MAKE) -C ${common} clean
	$(MAKE) -C ${newwork} clean
	$(MAKE) -C ${zigbee} clean
	$(MAKE) -C ${modbus_485} clean
	$(MAKE) -C ${modbus_232} clean
	$(MAKE) -C ${data} clean
	$(MAKE) -C ${meter} clean
	$(MAKE) -C ${gpio} clean
