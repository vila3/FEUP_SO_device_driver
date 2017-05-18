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

static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);

#define UART_BASE	0x03F8	// UART's base I/O port-address (COM1)

dev_t dev_buffer;
static struct cdev *mcdev;
int check_end = 0;
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
	filep->private_data = mcdev;
	printk(KERN_ALERT "Device was open.\n");
	nonseekable_open(inodep, filep);
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	filep->private_data = NULL;
	printk(KERN_ALERT "Device was release 1.\n");
	return 0;
}

ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	char *mBuff = (char *)kzalloc(sizeof(char)*count+1, GFP_KERNEL);
	int n = copy_from_user(mBuff, buff, count);
	int i = 0;
	if(n!=0) {
		printk(KERN_ALERT "Ups! Couldn't get data to write! (%d)", n);
		return 0;
	}
	while (1) {
		if (message[i] == '\0' || (i >= count))
			break;
		while ((inb(UART_BASE + UART_LSR) & UART_LSR_THRE) == 0)
			msleep_interruptible(1);
		outb(message[i++], UART_BASE + UART_TX);
	}
	kfree(mBuff);
	return i;
}

ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	char *mBuff = kzalloc(sizeof(char)*count, GFP_KERNEL);
	unsigned char line_status;
	int n = 0, n_nothing = 0;
	if (!mBuff) {
		return -1;
	}
	while (1) {
		line_status = inb(UART_BASE + UART_LSR);
		if ((line_status & UART_LSR_OE) != 0) {
			kfree(mBuff);
			printk(KERN_ALERT "Overrun error!");
			return -EIO;
		} else if ((line_status & UART_LSR_DR) == 0) {
			// printk(KERN_ALERT "Waiting...");
			msleep_interruptible(1);
			n_nothing++;
			// printk(KERN_ALERT "(%d, %d)\n", n_nothing, n);
			if (n_nothing > 100 && check_end) {
				// printk(KERN_ALERT "Waiting too long for a char, exiting!\n");
				check_end = 0;
				break;
			}
		} else {
			mBuff[n++] = inb(UART_BASE + UART_RX);
			mBuff[n] = '\0';
			if (n == count)
				break;
			n_nothing = 0;
			check_end = 1;
		}
	}
	if (copy_to_user(buff, mBuff, n) > 0) {
		printk(KERN_ALERT "Error copy data to user...\n");
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
	vft = alloc_chrdev_region(&dev_buffer, 0, 1, driver_name);
	if (vft < 0) {
	  printk(KERN_INFO "Major number allocation is failed\n");
	  return vft;
  	}
	printk(KERN_ALERT "Device number: <%d, %d>\n", MAJOR(dev_buffer), MINOR(dev_buffer));

	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	vft = cdev_add(mcdev, dev_buffer, 1);
	if (vft<0) {
		printk(KERN_ALERT "Device adding to the kerknel failed\n");
        return vft	;
	}
	printk(KERN_INFO "Device additin to the kernel succesful\n");

	printk(KERN_ALERT "Requestting control over serial port (COM1)...\n");
	rsc = request_region(UART_BASE, 1, driver_name);
	if (rsc == NULL)
	{
		printk(KERN_ALERT "Could not get control over serial port (COM1)!\n");
		cdev_del(mcdev);
		unregister_chrdev_region(dev_buffer, 1);
		return -1;
	}
	printk(KERN_ALERT "Gained control over serial port (COM1)!\n");

	lcr = UART_LCR_DLAB | UART_LCR_WLEN8 | UART_LCR_STOP | UART_LCR_EPAR;
	outb(lcr, UART_BASE + UART_LCR);
	outb(0x60, UART_BASE + UART_DLL);
	lcr &= ~UART_LCR_DLAB;
	outb(lcr, UART_BASE + UART_LCR);

	sendMessage("Connection established successfully!\n", 0);

	printk(KERN_ALERT "Connection established successfully!\n");

	return 0;
}

static void echo_exit(void)
{
	int major_num = MAJOR(dev_buffer);
	release_region(UART_BASE, 1);
	cdev_del(mcdev);
	unregister_chrdev_region(dev_buffer, 1);
	printk(KERN_ALERT "Unregisted device driver %d (%s)\n", major_num, driver_name);
}

module_init(echo_init);
module_exit(echo_exit);
