#include "dummyModule.h"

static int dummy_open_fifo(struct inode *inode, struct file *filp)
{
    pr_info("Dummy driver: FIFO opened\n");
    return 0;
}

static int dummy_release_fifo(struct inode *inode, struct file *filp)
{
    pr_info("Dummy driver: FIFO closed\n");
    return 0;
}

static ssize_t dummy_read_fifo(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copiedout;

    if (mutex_lock_interruptible(&read_mutex)) {
        pr_info("Mutex lock interrupted by a signal\n");
        return -ERESTARTSYS;
    }

    ret = kfifo_to_user(&my_fifo, buf, count, &copiedout);

    mutex_unlock(&read_mutex);

    return copiedout;
}

static ssize_t dummy_write_fifo(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copiedin;
    
    if (mutex_lock_interruptible(&write_mutex)) {
        pr_info("Mutex lock interrupted by a signal\n");
        return -ERESTARTSYS;
    }

    pr_info("FIFO available space: %d\n", kfifo_avail(&my_fifo));

    ret = kfifo_from_user(&my_fifo, buf, count, &copiedin);
    mutex_unlock(&write_mutex);
    pr_info("Data copied to FIFO: %u bytes\n", copiedin);

    return copiedin;
}

static int __init dummyModule_init(void)
{
    int ret;

    // Allocate major number
    ret = alloc_chrdev_region(&dev_num, BASE_MINORS, NR_MINORS, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number: %d\n", ret);
        return ret;
    }

    // Create class
    new_class = class_create(CLASS_NAME);
    if (IS_ERR(new_class)) {
        ret = PTR_ERR(new_class);
        pr_err("Failed to create class: %d\n", ret);
        goto err_class_create;
    }

    // Create device
    device = device_create(new_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        pr_err("Failed to create device: %d\n", ret);
        goto err_dev_create;
    }

    // Initialize cdev
    cdev_init(&new_cdev, &fops);
    new_cdev.owner = THIS_MODULE;
    ret = cdev_add(&new_cdev, dev_num, NR_MINORS);
    if (ret) {
        pr_err("Failed to add cdev: %d\n", ret);
        goto err_char_dev_add;
    }

    // Allocate FIFO
    ret = kfifo_alloc(&my_fifo, FIFO_SIZE, GFP_KERNEL);
    if (ret) {
        pr_err("Failed to allocate kfifo: %d\n", ret);
        goto err_alloc_fifo;
    }

    pr_info("Dummy module registered with major %d\n", MAJOR(dev_num));
    return 0;

err_alloc_fifo:
    cdev_del(&new_cdev);
err_char_dev_add:
    device_destroy(new_class, dev_num);
err_dev_create:
    class_destroy(new_class);
err_class_create:
    unregister_chrdev_region(dev_num, NR_MINORS);

    return ret;
}

static void __exit dummyModule_exit(void)
{
    cdev_del(&new_cdev);
    device_destroy(new_class, dev_num);
    class_destroy(new_class);
    unregister_chrdev_region(dev_num, NR_MINORS);
    kfifo_free(&my_fifo);
    pr_info("Dummy module removed\n");
}
