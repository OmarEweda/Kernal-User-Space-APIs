#ifndef DUMMY_MODULE_H
#define DUMMY_MODULE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h> // For copy_to_user and copy_from_user

#define DEVICE_NAME "DummyDev"
#define CLASS_NAME  "DummyClass"
#define BASE_MINORS     0
#define NR_MINORS       1 
#define FIFO_SIZE       100

static dev_t dev_num;
static struct class *new_class;
static struct device *device;
static struct cdev new_cdev;
static struct kfifo my_fifo;
static DEFINE_MUTEX(read_mutex);
static DEFINE_MUTEX(write_mutex);

static int dummy_open_fifo(struct inode *inode, struct file *filp);
static int dummy_release_fifo(struct inode *inode, struct file *filp);
static ssize_t dummy_read_fifo(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t dummy_write_fifo(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static int __init dummyModule_init(void);
static void __exit dummyModule_exit(void);


struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dummy_open_fifo,
    .release = dummy_release_fifo,
    .read = dummy_read_fifo,
    .write = dummy_write_fifo
};


module_init(dummyModule_init);
module_exit(dummyModule_exit);

MODULE_AUTHOR("Omar Eweda");
MODULE_DESCRIPTION("Dummy Kernel Module");
MODULE_LICENSE("GPL");



#endif // DUMMY_MODULE_H
