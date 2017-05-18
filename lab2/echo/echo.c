/*
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include "echo.h"
MODULE_LICENSE("Dual BSD/GPL");

static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);

dev_t dev_buffer = MKDEV(ECHO_MAJOR, 0);
static struct cdev *mcdev;
char *driver_name = "echo";

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write
};

static int device_open(struct inode *inodep, struct file *filep)
{
	filep->private_data = mcdev;
	printk(KERN_ALERT "Device was open.\n");
	nonseekable_open(inodep, filep);
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_ALERT "Device was release.\n");
	return 0;
}

ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	char *mBuff = (char *)kmalloc(sizeof(char)*count, GFP_KERNEL);
	int n = copy_from_user(mBuff, buff, count);
	if (mBuff[count-1] != '\0') {
		kfree(mBuff);
		char *mBuff = (char *)kmalloc(sizeof(char)*count+1, GFP_KERNEL);
		n = copy_from_user(mBuff, buff, count);
		mBuff[count] = '\0';
	}
	if(n!=0) {
		printk(KERN_ALERT "Ups! Couldn't get data to write! (%d)", n);
		return 0;
	}
	int size = printk(KERN_ALERT "%s", mBuff);
	kfree(mBuff);
	return size;
}

ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

static int echo_init(void)
{
	int vft;
	vft = alloc_chrdev_region(&dev_buffer, 0, ECHO_DEVS, driver_name);
	if (vft < 0) {
	  printk(KERN_INFO "Major number allocation is failed\n");
	  return vft;
  	}
	printk(KERN_ALERT "Device number: <%d, %d>\n", MAJOR(dev_buffer), MINOR(dev_buffer));

	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	vft = cdev_add(mcdev, dev_buffer, ECHO_DEVS);
	if (vft<0) {
		printk(KERN_ALERT "Device adding to the kerknel failed\n");
        return vft	;
	}
	printk(KERN_INFO "Device additin to the kernel succesful\n");

	return 0;
}

static void echo_exit(void)
{
	int major_num = MAJOR(dev_buffer);
	cdev_del(mcdev);
	unregister_chrdev_region(dev_buffer, ECHO_DEVS);
	printk(KERN_ALERT "Unregisted device driver %d (%s)\n", major_num, driver_name);
}

module_init(echo_init);
module_exit(echo_exit);
