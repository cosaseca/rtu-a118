/*
 * leds.c
 *
 *  Created on: Oct 28, 2014
 *      Author: ygz
 *
 *      test io module
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/io.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <plat/mux.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#define GPIO_0_BASE         0x44E07000
#define GPIO_1_BASE         0x4804C000
#define GPIO_2_BASE         0x481AC000
#define GPIO_3_BASE         0x481AE000
#define GPIO_SYSCONFIG      0x10
#define GPIO_SETDATAOUT     0x194
#define GPIO_DATAIN         0x138
#define GPIO_DATAOUT        0x13C
#define GPIO_CLEARDATAOUT   0x190
#define GPIO_OE				0x134


#define CONTROL_MODULE      0x44E10000
#define CONFIG_GPMC_CLCK    0x88C       //gpio2_1
#define CONFIG_GPMC_AD12    0x830       //gpio1_12
#define CONFIG_GPMC_AD13    0x834       //gpio1_13
#define CONFIG_GPMC_AD14    0x838       //gpio1_14
#define CONFIG_GPMC_AD15    0x83C       //gpio1_15

#define CONFIG_SPI0_CS0     0x95C       //gpio0_5
#define CONFIG_MII1_RX_CLK  0x930       //gpio3_10
#define CONFIG_GPMC_AD8     0x820       //gpio0_22
#define CONFIG_GPMC_AD9     0x824       //gpio0_23
#define CONFIG_GPMC_AD10    0x828       //gpio0_26
#define CONFIG_GPMC_AD11    0x82C       //gpio0_27

/*******************************************/
#define NAME "leds"
#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio))

static int major = 0;//249; //定义主设备号, 0为自动分配
struct class *leds_class;
struct device *leds_dev_node;

#if 0
static void pinmod_clear(unsigned base, unsigned offset) {
	unsigned ret = __raw_readl(base + offset);
	ret |= 0x7;
	__raw_writel(ret, base + offset);
}

static void pinmod_init(void) {
	volatile unsigned base;
	base = (volatile unsigned)ioremap(CONTROL_MODULE, 0x1000);
	pinmod_clear(base, CONFIG_GPMC_CLCK);
//	pinmod_clear(base, CONFIG_GPMC_AD12);
//	pinmod_clear(base, CONFIG_GPMC_AD13);
//	pinmod_clear(base, CONFIG_GPMC_AD14);
//	pinmod_clear(base, CONFIG_GPMC_AD15);

//	pinmod_clear(base, CONFIG_SPI0_CS0);
//	pinmod_clear(base, CONFIG_MII1_RX_CLK);
//	pinmod_clear(base, CONFIG_GPMC_AD8);
//	pinmod_clear(base, CONFIG_GPMC_AD9);
//	pinmod_clear(base, CONFIG_GPMC_AD10);
//	pinmod_clear(base,  CONFIG_GPMC_AD11);
	iounmap((void *)base);
}
#endif

/*******************************************/
void led_on(void) {
	gpio_set_value(GPIO_TO_PIN(2, 1), 1);
	gpio_set_value(GPIO_TO_PIN(1, 30), 1);
	gpio_set_value(GPIO_TO_PIN(2, 0), 1);
	gpio_set_value(GPIO_TO_PIN(3, 2), 1);
	gpio_set_value(GPIO_TO_PIN(3, 1), 1);
	gpio_set_value(GPIO_TO_PIN(3, 0), 1);

//	gpio_set_value(GPIO_TO_PIN(0, 31), 1);
//	gpio_set_value(GPIO_TO_PIN(1, 12), 1);
//	gpio_set_value(GPIO_TO_PIN(1, 13), 1);
//	gpio_set_value(GPIO_TO_PIN(1, 14), 1);
//	gpio_set_value(GPIO_TO_PIN(1, 15), 1);

//	gpio_set_value(GPIO_TO_PIN(0, 5), 1);
//	gpio_set_value(GPIO_TO_PIN(3, 10), 1);
//	gpio_set_value(GPIO_TO_PIN(0, 22), 1);
//	gpio_set_value(GPIO_TO_PIN(0, 23), 1);
//	gpio_set_value(GPIO_TO_PIN(0, 26), 1);
//	gpio_set_value(GPIO_TO_PIN(0, 27), 1);
}

void led_off(void) {
	gpio_set_value(GPIO_TO_PIN(2, 1), 0);
	gpio_set_value(GPIO_TO_PIN(1, 30), 0);
	gpio_set_value(GPIO_TO_PIN(2, 0), 0);
	gpio_set_value(GPIO_TO_PIN(3, 2), 0);
	gpio_set_value(GPIO_TO_PIN(3, 1), 0);
	gpio_set_value(GPIO_TO_PIN(3, 0), 0);

//	gpio_set_value(GPIO_TO_PIN(0, 31), 0);
//	gpio_set_value(GPIO_TO_PIN(1, 12), 0);
//	gpio_set_value(GPIO_TO_PIN(1, 13), 0);
//	gpio_set_value(GPIO_TO_PIN(1, 14), 0);
//	gpio_set_value(GPIO_TO_PIN(1, 15), 0);

//	gpio_set_value(GPIO_TO_PIN(0, 5), 0);
//	gpio_set_value(GPIO_TO_PIN(3, 10), 0);
//	gpio_set_value(GPIO_TO_PIN(0, 22), 0);
//	gpio_set_value(GPIO_TO_PIN(0, 23), 0);
//	gpio_set_value(GPIO_TO_PIN(0, 26), 0);
//	gpio_set_value(GPIO_TO_PIN(0, 27), 0);
}

void leds_io_init(void) {
	int result;
//	pinmod_init();
	/* Allocating GPIOs and setting direction */
	result = gpio_request(GPIO_TO_PIN(2, 1), "Leds"); //usr1
	result = gpio_request(GPIO_TO_PIN(1, 30), "Leds"); //usr1
	result = gpio_request(GPIO_TO_PIN(2, 0), "Leds"); //usr1
	result = gpio_request(GPIO_TO_PIN(3, 2), "Leds"); //usr1
	result = gpio_request(GPIO_TO_PIN(3, 1), "Leds"); //usr1
	result = gpio_request(GPIO_TO_PIN(3, 0), "Leds"); //usr1
	gpio_export(GPIO_TO_PIN(2, 1), 1);
	gpio_export(GPIO_TO_PIN(1, 30), 1);
	gpio_export(GPIO_TO_PIN(2, 0), 1);
	gpio_export(GPIO_TO_PIN(3, 2), 1);
	gpio_export(GPIO_TO_PIN(3, 1), 1);
	gpio_export(GPIO_TO_PIN(3, 0), 1);
//	result = gpio_request(GPIO_TO_PIN(0, 31), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(1, 12), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(1, 13), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(1, 14), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(1, 15), "Leds"); //usr1

//	result = gpio_request(GPIO_TO_PIN(0, 5), "Leds"); //usr1  485 s
//	result = gpio_request(GPIO_TO_PIN(3, 10), "Leds"); //usr1 485 m
//	result = gpio_request(GPIO_TO_PIN(0, 22), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(0, 23), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(0, 26), "Leds"); //usr1
//	result = gpio_request(GPIO_TO_PIN(0, 27), "Leds"); //usr1

//	result = gpio_direction_output(GPIO_TO_PIN(2, 1), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(1, 30), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(2, 0), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(3, 2), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(3, 1), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(3, 0), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(0, 31), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(1, 12), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(1, 13), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(1, 14), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(1, 15), 1);

//	result = gpio_direction_output(GPIO_TO_PIN(0, 5), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(3, 10), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(0, 22), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(0, 23), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(0, 26), 1);
//	result = gpio_direction_output(GPIO_TO_PIN(0, 27), 1);
}

void leds_io_free(void) {
	gpio_unexport(GPIO_TO_PIN(2, 1));
	gpio_unexport(GPIO_TO_PIN(1, 30));
	gpio_unexport(GPIO_TO_PIN(2, 0));
	gpio_unexport(GPIO_TO_PIN(3, 2));
	gpio_unexport(GPIO_TO_PIN(3, 1));
	gpio_unexport(GPIO_TO_PIN(3, 0));
	gpio_free(GPIO_TO_PIN(2, 1));
	gpio_free(GPIO_TO_PIN(1, 30));
	gpio_free(GPIO_TO_PIN(2, 0));
	gpio_free(GPIO_TO_PIN(3, 2));
	gpio_free(GPIO_TO_PIN(3, 1));
	gpio_free(GPIO_TO_PIN(3, 0));
//	gpio_free(GPIO_TO_PIN(0, 31));
//	gpio_free(GPIO_TO_PIN(1, 12));
//	gpio_free(GPIO_TO_PIN(1, 13));
//	gpio_free(GPIO_TO_PIN(1, 14));
//	gpio_free(GPIO_TO_PIN(1, 15));

//	gpio_free(GPIO_TO_PIN(0, 5));
//	gpio_free(GPIO_TO_PIN(3, 10));
//	gpio_free(GPIO_TO_PIN(0, 22));
//	gpio_free(GPIO_TO_PIN(0, 23));
//	gpio_free(GPIO_TO_PIN(0, 26));
//	gpio_free(GPIO_TO_PIN(0, 27));
}

struct leds_dev {
	struct cdev cdev;
	unsigned char value;
};

struct leds_dev *leds_devp;

// 打开和关闭函数
int leds_open(struct inode *inode, struct file *filp) {
	struct leds_dev *dev;

// 获得设备结构体指针
	dev = container_of(inode->i_cdev,struct leds_dev,cdev);
// 让设备结构体作为设备的私有信息
	filp->private_data = dev;
	printk("leds_open \n");

	return 0;
}

int leds_release(struct inode *inode, struct file *filp) {
	printk("leds_release \n");
	return 0;
}

typedef struct Leds_io_t {
	int num;
	char status;
}Leds_io_t;

//灯控制接口
long leds_ioctl(struct file *filp, unsigned cmd, unsigned long arg) {
	Leds_io_t leds_io_t;
//	int aaa = 0;
	switch (cmd) {
	case 1:
		if (copy_from_user(&leds_io_t, (Leds_io_t *) arg,
				sizeof(Leds_io_t)))
			return -EFAULT;
		gpio_set_value(leds_io_t.num, leds_io_t.status);
//		copy_from_user(&aaa, rtu_a118_map_addr,
//					sizeof(int));
		break;

	case 0:
		if (copy_from_user(&leds_io_t, (Leds_io_t *) arg,
				sizeof(Leds_io_t)))
			return -EFAULT;
		leds_io_t.status = gpio_get_value(leds_io_t.num);
		if (copy_to_user((Leds_io_t *) arg,
				&leds_io_t, sizeof(Leds_io_t)))
			return -EFAULT;
		break;
	default:

		return -ENOTTY;
	}

	return 0;
}

struct file_operations leds_fops = {
		.owner = THIS_MODULE,
		.unlocked_ioctl = leds_ioctl,
		.open = leds_open,
		.release = leds_release
};

// 模块加载函数
static int leds_init(void) {
	int ret;
//	leds_io_init();
	ret = register_chrdev(major, NAME, &leds_fops);
	if (ret < 0) {
		printk("unable to register leds driver!\n");
		return ret;
	}
	major = (major == 0?ret:major);
	printk("light_init major %d\n", major);

	leds_class = class_create(THIS_MODULE, "leds_class"); //注册一个类，使mdev可以在"/dev/"目录下 面建立设备节点
	if(IS_ERR(leds_class)){
		printk("create class error\n");
	} else {
		//创建一个设备节点，节点名为DEVICE_NAME
		leds_dev_node = device_create(leds_class, NULL, MKDEV(major, 0), NULL, "leds");
	}
	return 0;
}

// 模块卸载函数
static void leds_exit(void) {
//	leds_io_free();
	device_destroy(leds_class, major);
	class_destroy(leds_class);
	unregister_chrdev(major, NAME);
	printk("leds_exit major %d\n", major);
}

module_init(leds_init);
module_exit(leds_exit);

MODULE_DESCRIPTION("leds driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wzkj");
