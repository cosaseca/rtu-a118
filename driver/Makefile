ARCH=arm
CROSS_COMPILE=~/AZM335X_LINUX_BSP_201303/rootfs/buildroot/output/host/usr/bin/arm-linux-gnueabihf-
KDIR := ~/AZM335X_LINUX_BSP_201303/kernel
INC = -I$(KDIR)/drivers/staging/iio/adc
obj-m := gpio_irq.o leds.o wzkj_adc.o ti_adc.o
PWD := $(shell pwd)
all:
	cp -rf $(KDIR)/drivers/gpio/leds.c .
	cp -rf $(KDIR)/drivers/gpio/gpio_irq.c .
#	cp -rf $(KDIR)/drivers/staging/iio/adc/ti_adc.c .
	cp -rf $(KDIR)/drivers/staging/iio/adc/wzkj_adc.c .
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules $(INC)
	
install:
	cp -rf leds.c $(KDIR)/drivers/gpio/leds.c
	cp -rf gpio_irq.c $(KDIR)/drivers/gpio/gpio_irq.c
	cp -rf gpio_Kconfig $(KDIR)/drivers/gpio/Kconfig
	cp -rf gpio_Makefile $(KDIR)/drivers/gpio/Makefile
	cp -rf wzkj_kernel_config $(KDIR)/.config
	cp -rf omap-serial.h $(KDIR)/arch/arm/plat-omap/include/plat/omap-serial.h
	cp -rf omap-serial.c $(KDIR)/drivers/tty/serial/omap-serial.c
	cp -rf serial_reg.h $(KDIR)/include/linux/serial_reg.h
	cp -rf board-am335xevm.c $(KDIR)/arch/arm/mach-omap2/board-am335xevm.c
	cp -rf mux33xx.c $(KDIR)/arch/arm/mach-omap2/mux33xx.c
	
app: leds_test.c
	$(CROSS_COMPILE)gcc -o leds_test leds_test.c	
	$(CROSS_COMPILE)gcc -o netlink_test netlink_test.c
	$(CROSS_COMPILE)gcc -o knevn knevn.c -lpthread
	$(CROSS_COMPILE)gcc -o gpio_irq_test gpio_irq_test.c
	
clean:
	rm -rf *.o *.ko *.mod.c modules.order Module.symvers