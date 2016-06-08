/*
 * gpio_irq.c
 *
 *  Created on: Nov 6, 2014
 *      Author: ygz
 */
//#define EXPORT_SYMTAB
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
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/timer.h>
#include <linux/time.h>
#include <linux/types.h>
#include <net/sock.h>
#include <net/netlink.h>

#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio)) //计算io口地址

#define NETLINK_TEST 25
#define MAX_MSGSIZE 1024
#define NAME "gpio"       //设备名

static struct sock *nl_sk = NULL;
static int major = 0;//主设备号
static int gpio_flag = 0;//等待队列标识

static int snd_msg[12];//内部缓冲区

static struct class *gpio_class;//设备类
static struct device *gpio_dev_node;//设备节点
DECLARE_WAIT_QUEUE_HEAD(gpio_q_head);//等待队列

/*
GPIO1_15
GPIO1_14
GPIO1_13
GPIO1_12
GPIO0_27
GPIO0_26
EHRPWM2B	GPIO0_23
EHRPWM2A	GPIO0_22
ALARM       GPIO0_31
*/

/* io口列表 */
static unsigned irq_num_table[][2] = {
		{GPIO_TO_PIN(0, 7), -1},
		{GPIO_TO_PIN(1, 15), -1},
		{GPIO_TO_PIN(1, 14), -1},
		{GPIO_TO_PIN(1, 13), -1},
		{GPIO_TO_PIN(1, 12), -1},
		{GPIO_TO_PIN(0, 27), -1},
		{GPIO_TO_PIN(0, 26), -1},
		{GPIO_TO_PIN(0, 23), -1},
		{GPIO_TO_PIN(0, 22), -1},
		{GPIO_TO_PIN(0, 31), -1},
};

void sendnldata(char *data, int n)
{
    struct sk_buff *skb_1;
    struct nlmsghdr *nlh;
    int len = NLMSG_SPACE(MAX_MSGSIZE);
    if(NULL == data || NULL == nl_sk)
    {
        return ;
    }
	skb_1 = alloc_skb(len,GFP_ATOMIC);
	if(NULL == skb_1)
	{
		printk(KERN_ERR "my_net_link:alloc_skb_1 error\n");
		return;
	}
    nlh = nlmsg_put(skb_1,0,0,0,MAX_MSGSIZE,0);
    if(NULL == nlh) {
    	printk(KERN_INFO "nlmsg_put\n");
    	kfree_skb(skb_1);
    	return;
    }
    NETLINK_CB(skb_1).pid = 0;
    NETLINK_CB(skb_1).dst_group = 0;

    memcpy(NLMSG_DATA(nlh),data, n);
//    printk("my_net_link:send message '%s'.\n",(char *)NLMSG_DATA(nlh));
//    netlink_unicast(nl_sk,skb_1,pid,MSG_DONTWAIT);
    netlink_broadcast(nl_sk, skb_1, 0, 1, GFP_ATOMIC);
}

void nl_data_ready(struct sk_buff *__skb) {
#if 0
	return;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	char str[100];
	struct completion cmpl;
	int i = 10;
	skb = skb_get(__skb);
	if (skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(skb);

		memcpy(str, NLMSG_DATA(nlh), sizeof(str));
		printk("Message received:%s\n", str);
//		pid = nlh->nlmsg_pid;
		while (i--) {
			init_completion(&cmpl);
			wait_for_completion_timeout(&cmpl, 3 * HZ);
//			sendnlmsg("I am from kernel!");
		}
//		flag = 1;
		kfree_skb(skb);
	}
#endif
}

static int netlink_init(void) {
	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, 1, NULL, NULL, THIS_MODULE);
	printk(KERN_INFO "netlink_init initialed ok!\n");
	if (!nl_sk) {
		printk(KERN_INFO "netlink create error!\n");
	}
	return 0;
}

static void netlink_exit(void) {
	if(nl_sk != NULL){
		sock_release(nl_sk->sk_socket);
	}
}

/* 中断处理函数 */
irqreturn_t handler(int name, void *arg) {
//	int rc[2] = {(int)arg, gpio_get_value((int)arg)};
	snd_msg[0] = (int)arg;
	snd_msg[1] = gpio_get_value((int)arg);
//	printk("irq handler name %d rc %d arg %d\n", name, rc, arg);
//	sendnldata((char *)snd_msg, 2 * sizeof(int));
	gpio_flag = 1;
	wake_up_interruptible(&gpio_q_head);
	return IRQ_HANDLED;
}

// 打开设备
int gpio_open(struct inode *inode, struct file *filp) {
//	struct leds_dev *dev;

// 获得设备结构体指针
//	dev = container_of(inode->i_cdev,struct leds_dev,cdev);
// 让设备结构体作为设备的私有信息
//	filp->private_data = dev;
	printk("gpio_open \n");

	return 0;
}

/* 关闭设备 */
int gpio_release(struct inode *inode, struct file *filp) {
	printk("gpio_release \n");
	return 0;
}

/* 读设备接口 */
ssize_t gpio_read(struct file *file, char *buf, size_t len, loff_t *ppos) {
	int rc;
	wait_event_interruptible(gpio_q_head, gpio_flag == 1);
	gpio_flag = 0;
	rc = copy_to_user(buf, snd_msg, 2 * sizeof(int));
	if(rc < 0) {
		return rc;
	}
	return (2 * sizeof(int));
}

/* 设备操作 */
struct file_operations gpio_fops = {
		.owner = THIS_MODULE,
//		.unlocked_ioctl = gpio_ioctl,
		.read = gpio_read,
		.open = gpio_open,
		.release = gpio_release
};


// 模块加载函数
static int gpio_init(void) {
	int rc = -1;
	int i = 0;
	int ret;
	netlink_init();
//	irq_num = gpio_to_irq(GPIO_TO_PIN(0, 7));
//	printk("irq_num %d\n", irq_num);
//	rc = request_irq(irq_num, handler,
//			IRQ_TYPE_EDGE_BOTH, "gpio_test",
//			(void *)GPIO_TO_PIN(0, 7));
	for(;i < sizeof(irq_num_table)/(2*sizeof(int));++i){
		char buf[24];
		sprintf(buf, "do_%d", i);
		irq_num_table[i][1] = gpio_to_irq(irq_num_table[i][0]);
		rc = request_irq(irq_num_table[i][1], handler,
			IRQ_TYPE_EDGE_BOTH, buf,
			(void *)(irq_num_table[i][0]));
		printk(KERN_INFO"gpio_init %d, %d, %d %s\n",
				irq_num_table[i][1], irq_num_table[i][0], rc
				,buf);
	}

	ret = register_chrdev(major, NAME, &gpio_fops);
	if (ret < 0) {
		printk("unable to register leds driver!\n");
		return ret;
	}
	major = (major == 0?ret:major);
	printk("light_init major %d %s\n", major, NAME);
	gpio_class = class_create(THIS_MODULE, "gpio_class"); //注册一个类，使mdev可以在"/dev/"目录下 面建立设备节点
	if(IS_ERR(gpio_class)){
		printk("create class error\n");
	} else {
		//创建一个设备节点，节点名为DEVICE_NAME
		gpio_dev_node = device_create(gpio_class, NULL, MKDEV(major, 0), NULL, "gpio");
	}
	return 0;
}

//模块卸载函数
static void gpio_exit(void) {
//	free_irq(irq_num, (void *)GPIO_TO_PIN(0, 7));
	int rc = -1;
	int i = 0;
	netlink_exit();
	for(;i < sizeof(irq_num_table)/(2*sizeof(int));++i){
		if(-1 != irq_num_table[i][1]) {
			free_irq(irq_num_table[i][1], (void *)(irq_num_table[i][0]));
			printk(KERN_INFO"gpio_init %d, %d, %d\n", irq_num_table[i][1], irq_num_table[i][0], rc);
		}
	}
	device_destroy(gpio_class, major);
	class_destroy(gpio_class);
	unregister_chrdev(major, NAME);
	printk("exit %d %s\n", major, NAME);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_DESCRIPTION("gpio irq driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wzkj");
