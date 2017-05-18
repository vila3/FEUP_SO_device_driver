/*
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
MODULE_LICENSE("Dual BSD/GPL");

dev_t **dev_buffer;

static int hello_init(void)
{
	int ret;
	printk(KERN_ALERT "Hello, world\n");
	dev_buffer = kmalloc(sizeof(dev_t*), GFP_KERNEL);
	*dev_buffer = kmalloc(sizeof(dev_t), GFP_KERNEL);
	ret = alloc_chrdev_region(*dev_buffer, 0, 1, "echo");
	if (ret < 0) {
	  printk(KERN_INFO "Major number allocation is failed\n");
	  return ret;
  	}
	return 0;
}

static void hello_exit(void)
{
	kfree(*dev_buffer);
	kfree(dev_buffer);
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
