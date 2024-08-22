#ifndef CHAR_DRVS_H
#define CHAR_DRVS_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>


#define DRIVER_NAME					"CharDrvs"
#define CLASS_NAME					"CharDrvsClass"
#define BASE_MINORS						0
#define NR_MINORS						1
#define FIFO_SIZE						16
#define MAX_NR_USERS					2
#define CHARDRVS_IOC_MAGIC		   		'c'
#define CHARDRVS_IOC_MAX_NR				3
#define CHARDRVS_IOC_SET_NR_USERS		_IOW(CHARDRVS_IOC_MAGIC, 1, int *) /* cmd 1: Set NR of users from UserSpace */
#define CHARDRVS_IOC_GET_NR_USERS		_IOR(CHARDRVS_IOC_MAGIC, 2, int *) /* cmd 2: Get NR of users from KernelSpace */
#define CHARDRVS_IOC_QUERY_AVAIL_SIZE	_IOR(CHARDRVS_IOC_MAGIC, 3, int *) /* cmd 3: Query-Get Fifo size from KernelSpace */


struct chardrvs_priv_dev {
    dev_t dev_num;
    struct class *new_class;
    struct device *device;
    struct cdev new_cdev;
    struct kfifo my_fifo;
    struct mutex read_mutex;
    struct mutex write_mutex;
	struct mutex lock_mutex;
	int usrs_cnt;
	unsigned int avail;
	atomic_t ref_count;
	wait_queue_head_t wq_f;
};

static int chardrvs_open(struct inode *inode, struct file *filp);
static int chardrvs_release(struct inode *inode, struct file *filp);
static ssize_t chardrvs_read_fifo(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t chardrvs_write_fifo(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static long chardrvs_ioctl (struct file *, unsigned int, unsigned long);
static __poll_t chardrvs_poll (struct file *, struct poll_table_struct *);

static int setup_chardrvs(struct chardrvs_priv_dev *);
static void uninstall_chardrvs(struct chardrvs_priv_dev *);
static int __init chardrvs_init(void);
static void __exit chardrvs_exit(void);


struct file_operations chardrvs_fops = {
	.owner = THIS_MODULE,
	.open = chardrvs_open,
	.release = chardrvs_release,
	.read = chardrvs_read_fifo,
	.write = chardrvs_write_fifo,
	.unlocked_ioctl = chardrvs_ioctl,
	.poll = chardrvs_poll,
	.llseek = no_llseek
};

static struct chardrvs_priv_dev *chardrvs_ptr;

module_init(chardrvs_init);
module_exit(chardrvs_exit);

MODULE_AUTHOR("Omar Eweda");
MODULE_DESCRIPTION("Dummy Kernel Module");
MODULE_LICENSE("GPL");

#endif