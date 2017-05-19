/*
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include "serial_reg.h"
MODULE_LICENSE("Dual BSD/GPL");

struct serp_dev {
	struct cdev cdev;
	int cnt;
	int check_timeout;
};

static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);

#define UART_BASE	0x03F8	// UART's base I/O port-address (COM1)
#define SERP_COUNT	1

dev_t dev_buffer;
struct serp_dev *dev;
const char driver_name[] = "serp";

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write
};

int sendMessage(char *message, int count)
{
	int i = 0;
	if (count <= 0) {
		count = 128;
	}
	while (1) {
		if (message[i] == '\0' || (i >= count))
			break;
		while ((inb(UART_BASE + UART_LSR) & UART_LSR_THRE) == 0)
			msleep_interruptible(1);
		outb(message[i++], UART_BASE + UART_TX);
	}
	return i;
}

static int device_open(struct inode *inodep, struct file *filep)
{
	struct serp_dev *mdev;
	mdev = container_of(inodep->i_cdev, struct serp_dev, cdev);
	filep->private_data = mdev;
	printk(KERN_ALERT " -k Device was open.\n");
	nonseekable_open(inodep, filep);

	// empty the UART buffer if exist
	while((inb(UART_BASE + UART_LSR) & (UART_LSR_DR | UART_LSR_OE)) != 0) inb(UART_BASE + UART_RX);
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	struct serp_dev *mdev;
	mdev = filep->private_data;
	mdev->check_timeout = 0;
	filep->private_data = NULL;
	printk(KERN_ALERT " -k Device was release.\n");
	return 0;
}

ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	char *mBuff = (char *)kzalloc(sizeof(char)*count+1, GFP_KERNEL);
	int n = copy_from_user(mBuff, buff, count);
	int i = 0;
	if(n!=0) {
		printk(KERN_ALERT " -k Ups! Couldn't get data to write! (%d)", n);
		return 0;
	}
	while (1) {
		if (mBuff[i] == '\0' || (i >= count))
			break;
		while ((inb(UART_BASE + UART_LSR) & UART_LSR_THRE) == 0)
			msleep_interruptible(1);
		outb(mBuff[i++], UART_BASE + UART_TX);
	}
	kfree(mBuff);
	return i;
}

ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	struct serp_dev *mdev;
	char *mBuff = kzalloc(sizeof(char)*count, GFP_KERNEL);
	unsigned char line_status;
	int n = 0, n_nothing = 0;
	if (!mBuff) {
		return -1;
	}
	mdev = filep->private_data;

	while (1) {
		line_status = inb(UART_BASE + UART_LSR);
		if ((line_status & UART_LSR_DR) == 0) {
			// printk(KERN_ALERT "Waiting...");
			msleep_interruptible(1);
			n_nothing++;
			// printk(KERN_ALERT "(%d, %d)\n", n_nothing, mdev->check_timeout);
			if (n_nothing > 100 && mdev->check_timeout) {
				if (n==0) {
					printk(KERN_ALERT " -k Waiting too long for a char, exiting!\n");
					return 0;
				}
				break;
			}
		} else {
			if ((line_status & UART_LSR_OE) != 0) {
				kfree(mBuff);
				printk(KERN_ALERT "Overrun error!");
				return -EIO;
            }
			mBuff[n++] = inb(UART_BASE + UART_RX);
			// printk(KERN_ALERT "%c", mBuff[n-1]);
			if (n == count)
				break;
			n_nothing = 0;
			mdev->check_timeout = 1;
		}
	}
	if (copy_to_user(buff, mBuff, n) > 0) {
		printk(KERN_ALERT " -k Error copy data to user...\n");
		return -1;
	}
	kfree(mBuff);
	return n;
}

static int echo_init(void)
{
	int vft;
	struct resource *rsc;
	unsigned char lcr;
	vft = alloc_chrdev_region(&dev_buffer, 0, SERP_COUNT, driver_name);
	if (vft < 0) {
	  printk(KERN_INFO " -k Major number allocation is failed\n");
	  return vft;
  	}
	printk(KERN_ALERT " -k Device number: <%d, %d>\n", MAJOR(dev_buffer), MINOR(dev_buffer));

	dev = kzalloc(sizeof(struct serp_dev)*SERP_COUNT, GFP_KERNEL);

	// in this case only care about one device
	cdev_init(&dev[0].cdev, &fops);
	dev[0].cdev.ops = &fops;
	dev[0].cdev.owner = THIS_MODULE;
	dev[0].check_timeout = 0;

	vft = cdev_add(&dev[0].cdev, dev_buffer, 1);
	if (vft<0) {
		printk(KERN_ALERT " -k Device adding to the kerknel failed\n");
        return vft	;
	}
	printk(KERN_INFO " -k Device additin to the kernel succesful\n");

	printk(KERN_ALERT " -k Requestting control over serial port (COM1)...\n");
	rsc = request_region(UART_BASE, 1, driver_name);
	if (rsc == NULL)
	{
		printk(KERN_ALERT " -k Could not get control over serial port (COM1)!\n");
		cdev_del(&dev[0].cdev);
		unregister_chrdev_region(dev_buffer, 1);
		return -1;
	}
	printk(KERN_ALERT " -k Gained control over serial port (COM1)!\n");

	lcr = UART_LCR_DLAB | UART_LCR_WLEN8 | UART_LCR_STOP | UART_LCR_EPAR;
	outb(lcr, UART_BASE + UART_LCR);
	outb(0x60, UART_BASE + UART_DLL);
	lcr &= ~UART_LCR_DLAB;
	outb(lcr, UART_BASE + UART_LCR);

	sendMessage("Connection established successfully!\n", 0);

	printk(KERN_ALERT " -k Connection established successfully!\n");

	return 0;
}

static void echo_exit(void)
{
	int major_num = MAJOR(dev_buffer);
	release_region(UART_BASE, 1);
	cdev_del(&dev[0].cdev);
	kfree(dev);
	unregister_chrdev_region(dev_buffer, 1);
	printk(KERN_ALERT "Unregisted device driver %d (%s)\n", major_num, driver_name);
}

module_init(echo_init);
module_exit(echo_exit);
