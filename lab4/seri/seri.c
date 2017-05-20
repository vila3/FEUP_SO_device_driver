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
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include "serial_reg.h"
MODULE_LICENSE("Dual BSD/GPL");

struct  seri_dev {
	struct cdev cdev;
	int cnt;
	int check_timeout;
	int sem;
	struct kfifo *read_buffer;
	spinlock_t *read_lock;
	struct kfifo *write_buffer;
	spinlock_t *write_lock;
	wait_queue_head_t wq;
	int read_flag;
	int write_flag;
	int idle;
};

static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);

#define UART_BASE	0x03F8	// UART's base I/O port-address (COM1)
#define SERI_COUNT	1
#define KFIFO_SIZE	4

dev_t dev_buffer;
struct  seri_dev *dev;
const char driver_name[] = " seri";

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write
};

static int device_open(struct inode *inodep, struct file *filep)
{
	struct  seri_dev *mdev;
	mdev = container_of(inodep->i_cdev, struct  seri_dev, cdev);
	filep->private_data = mdev;
	printk(KERN_ALERT " -k Device was open.\n");
	nonseekable_open(inodep, filep);

	init_waitqueue_head(&mdev->wq);
	mdev->read_flag = 0;
	mdev->write_flag = 0;
	mdev->check_timeout = 0;

	// empty the UART buffer if exist
	while(inb(UART_BASE + UART_LSR) & (UART_LSR_DR | UART_LSR_OE)) inb(UART_BASE + UART_RX);
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	struct  seri_dev *mdev;
	mdev = filep->private_data;
	mdev->read_flag = 0;
	mdev->write_flag = 0;
	filep->private_data = NULL;
	printk(KERN_ALERT " -k Device was release.\n");
	return 0;
}

static irqreturn_t int_handler(int irq, void *dev_id) {
	unsigned char iir, c;
	struct seri_dev *mdev = dev_id;
	// printk(" -k Interrupt\n");
	iir = inb(UART_BASE + UART_IIR);
	// read
	if (iir & UART_IIR_RDI) {
		c = inb(UART_BASE + UART_RX);
		// printk(KERN_ALERT "%c\n", c);
		kfifo_put(mdev->read_buffer, &c, 1);
		mdev->read_flag = 1;
		mdev->check_timeout = 1;
		wake_up_interruptible(&mdev->wq);
		return IRQ_HANDLED;
	}
	// write
	if (iir & UART_IIR_THRI) {
		if (kfifo_len(mdev->write_buffer)) {
			kfifo_get(mdev->write_buffer, &c, 1);
			outb(c, UART_BASE + UART_TX);
			mdev->idle = 0;
			wake_up_interruptible(&mdev->wq);
		} else {
			mdev->idle = 1;
		}
		mdev->write_flag = 1;
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}
// ---------------------
ssize_t device_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	struct  seri_dev *mdev;
	char *mBuff = (char *)kzalloc(sizeof(char)*count+1, GFP_KERNEL);
	unsigned char c, irq_flags;
	int n = copy_from_user(mBuff, buff, count);
	int i = 0, cnt=0;
	if(n!=0) {
		printk(KERN_ALERT " -k Ups! Couldn't get data to write! (%d)", n);
		return 0;
	}
	mdev = filep->private_data;

	if ((count - mdev->write_buffer->size) > 0) {
		kfifo_put(mdev->write_buffer, mBuff, mdev->write_buffer->size);
		i = mdev->write_buffer->size;
	} else {
		kfifo_put(mdev->write_buffer, mBuff, count);
		i = count;
	}
	printk(KERN_ALERT " -k i=%d | size=%d | count=%d\n", i, mdev->write_buffer->size, count);

	while (i < count) {
		if(mdev->idle) {
			kfifo_get(mdev->write_buffer, &c, 1);
			outb(c, UART_BASE + UART_TX);
			printk(KERN_ALERT "i=%d\n", i);
			cnt++;
		}
		if (kfifo_len(mdev->write_buffer) == mdev->write_buffer->size) {
			if(wait_event_interruptible(mdev->wq, mdev->write_flag != 0) != 0)
				return -ERESTARTSYS;
			mdev->write_flag = 0;
			continue;
		}
		if (kfifo_put(mdev->write_buffer, mBuff + i, 1))
			i++;
	}
	printk(KERN_ALERT " -k i=%d | cnt=%d\n", i, cnt);
	kfree(mBuff);
	return i;
}

ssize_t device_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	struct  seri_dev *mdev;
	char *mBuff = kzalloc(sizeof(char)*count, GFP_KERNEL);
	int n=0, result;
	if (!mBuff) {
		return -1;
	}
	mdev = filep->private_data;

	while (kfifo_len(mdev->read_buffer) == 0) {
		result = wait_event_interruptible_timeout(mdev->wq, mdev->read_flag != 0, 100);
		switch (result) {
			case -ERESTARTSYS:
				return -ERESTARTSYS;
			case 0:
				if(mdev->check_timeout == 1)
					return 0;
			default:
				mdev->read_flag = 0;
		}
	}

	do {
		kfifo_get(mdev->read_buffer, mBuff + n, 1);
		n++;
	} while (kfifo_len(mdev->read_buffer) > 0);

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
	spinlock_t *tmp_lock;
	vft = alloc_chrdev_region(&dev_buffer, 0, SERI_COUNT, driver_name);
	if (vft < 0) {
	  printk(KERN_INFO " -k Major number allocation is failed\n");
	  return vft;
  	}
	printk(KERN_ALERT " -k Device number: <%d, %d>\n", MAJOR(dev_buffer), MINOR(dev_buffer));

	dev = kzalloc(sizeof(struct  seri_dev)*SERI_COUNT, GFP_KERNEL);

	// in this case only care about one device
	cdev_init(&dev[0].cdev, &fops);
	dev[0].cdev.ops = &fops;
	dev[0].cdev.owner = THIS_MODULE;
	dev[0].check_timeout = 0;
	dev[0].idle = 0;
	spin_lock_init(dev[0].read_lock);
	spin_lock_init(dev[0].write_lock);
	// initialize buffers
	spin_lock_init(tmp_lock);
	dev[0].read_buffer = kfifo_alloc(KFIFO_SIZE, GFP_KERNEL, dev[0].read_lock);
	tmp_lock = NULL;
	spin_lock_init(tmp_lock);
	dev[0].write_buffer = kfifo_alloc(KFIFO_SIZE, GFP_KERNEL, dev[0].write_lock);

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
	outb(UART_IER_RDI | UART_IER_THRI, UART_BASE + UART_IER);

	request_irq(4, int_handler, SA_INTERRUPT, driver_name, &dev[0]);

	printk(KERN_ALERT " -k Connection established successfully!\n");

	return 0;
}

static void echo_exit(void)
{
	int major_num = MAJOR(dev_buffer);
	free_irq(4, &dev[0]);
	release_region(UART_BASE, 1);
	kfree(dev[0].write_buffer->lock);
	kfifo_free(dev[0].write_buffer);
	kfree(dev[0].write_lock);
	kfree(dev[0].read_buffer->lock);
	kfifo_free(dev[0].read_buffer);
	kfree(dev[0].read_lock);
	cdev_del(&dev[0].cdev);
	kfree(dev);
	unregister_chrdev_region(dev_buffer, 1);
	printk(KERN_ALERT "Unregisted device driver %d (%s)\n", major_num, driver_name);
}

module_init(echo_init);
module_exit(echo_exit);
