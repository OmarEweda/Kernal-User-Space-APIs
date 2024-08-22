#include "charDrvs.h"

static int chardrvs_open(struct inode *inode, struct file *filp)
{
    if(atomic_read(&chardrvs_ptr->ref_count) >= MAX_NR_USERS){
        pr_err("Too many users open \n");
        return -EMFILE;
    }

    atomic_inc(&chardrvs_ptr->ref_count);
    filp->private_data = chardrvs_ptr;
    pr_info("Open Char Driver \n");
    return 0;
}

static int chardrvs_release(struct inode *inode, struct file *filp)
{
    if(atomic_read(&chardrvs_ptr->ref_count) > 0){
        atomic_dec(&chardrvs_ptr->ref_count);
    }
    pr_info("Release Char Driver \n");
    return 0;
}

static ssize_t chardrvs_read_fifo(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copiedout;

    if (mutex_lock_interruptible(&chardrvs_ptr->read_mutex)) {
        pr_info("Mutex lock interrupted by a signal\n");
        return -ERESTARTSYS;
    }

    ret = kfifo_to_user(&chardrvs_ptr->my_fifo, buf, count, &copiedout);
    
    wake_up_interruptible(&chardrvs_ptr->wq_f);
    mutex_unlock(&chardrvs_ptr->read_mutex);
    pr_info("The data copiedout: %d\n", copiedout);

    mutex_lock(&chardrvs_ptr->lock_mutex);
    chardrvs_ptr->avail = kfifo_avail(&chardrvs_ptr->my_fifo);
    mutex_unlock(&chardrvs_ptr->lock_mutex);

    return copiedout;
}

static ssize_t chardrvs_write_fifo(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copiedin;
    
    if (mutex_lock_interruptible(&chardrvs_ptr->write_mutex)) {
        pr_info("Mutex lock interrupted by a signal\n");
        return -ERESTARTSYS;
    }

    if(wait_event_interruptible(chardrvs_ptr->wq_f, (chardrvs_ptr->avail >= count)))
    {
        return -ERESTARTSYS;
    }

   pr_info("FIFO available space: %d\n", kfifo_avail(&chardrvs_ptr->my_fifo));

    ret = kfifo_from_user(&chardrvs_ptr->my_fifo, buf, count, &copiedin);
    mutex_unlock(&chardrvs_ptr->write_mutex);
    pr_info("Data copied to FIFO: %u bytes\n", copiedin);

    mutex_lock(&chardrvs_ptr->lock_mutex);
    chardrvs_ptr->avail = kfifo_avail(&chardrvs_ptr->my_fifo);
    mutex_unlock(&chardrvs_ptr->lock_mutex);

    return copiedin;
}

static long chardrvs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if(_IOC_TYPE(cmd)!= CHARDRVS_IOC_MAGIC){
        return -ENOTTY;
    }
    if(_IOC_NR(cmd)!= CHARDRVS_IOC_MAX_NR){
        return -ENOTTY;
    }

    switch (cmd)
    {
        case CHARDRVS_IOC_SET_NR_USERS:
            if(! capable(CAP_SYS_ADMIN))
                return -EPERM;
            ret = __get_user(chardrvs_ptr->usrs_cnt, &arg);
            break;
    
        case CHARDRVS_IOC_GET_NR_USERS:
            ret = __put_user(chardrvs_ptr->usrs_cnt, &arg);
            break;
    
        case CHARDRVS_IOC_QUERY_AVAIL_SIZE:
            ret = __put_user(chardrvs_ptr->avail, &arg);
            break;
    }
    return ret;
}

static __poll_t chardrvs_poll(struct file * file, struct poll_table_struct *wait)
{
    __poll_t mask = 0;
    mutex_lock(&chardrvs_ptr->lock_mutex);
    poll_wait(file, &chardrvs_ptr->wq_f, wait);
    mutex_unlock(&chardrvs_ptr->lock_mutex);
    if(chardrvs_ptr->avail){
        mask |= POLLOUT;
    }
    return mask;
}

static int setup_chardrvs(struct chardrvs_priv_dev *chardrvs_ptr)
{
    mutex_init(&chardrvs_ptr->read_mutex);
    mutex_init(&chardrvs_ptr->write_mutex);
    mutex_init(&chardrvs_ptr->lock_mutex);

    chardrvs_ptr->usrs_cnt = MAX_NR_USERS;

    int ret = kfifo_alloc(&chardrvs_ptr->my_fifo, FIFO_SIZE, GFP_KERNEL);
    if(ret){
        return ret;
    }
    else{
        chardrvs_ptr->avail = FIFO_SIZE;
    }
    return ret;
}

static void uninstall_chardrvs(struct chardrvs_priv_dev * chardrvs_ptr)
{
    kfifo_free(&chardrvs_ptr->my_fifo);
}

static int __init chardrvs_init(void)
{
    int ret;
    chardrvs_ptr = kmalloc(sizeof(struct chardrvs_priv_dev), GFP_KERNEL);

    if(chardrvs_ptr == NULL){
        ret = -ENOMEM;
        pr_err("Failed to allocate memory: %d\n", ret);
        return ret;
    }
    memset((void*)chardrvs_ptr, 0, sizeof(struct chardrvs_priv_dev));

    // Allocate major number
    ret = alloc_chrdev_region(&chardrvs_ptr->dev_num, BASE_MINORS, NR_MINORS, DRIVER_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number: %d\n", ret);
		goto err_free_dev;
    }

    // Create class
    chardrvs_ptr->new_class = class_create(CLASS_NAME);
    if (IS_ERR(chardrvs_ptr->new_class)) {
        ret = PTR_ERR(chardrvs_ptr->new_class);
        pr_err("Failed to create class: %d\n", ret);
        goto err_class_create;
    }

    // Create device
    chardrvs_ptr->device = device_create(chardrvs_ptr->new_class, NULL, chardrvs_ptr->dev_num, NULL, DRIVER_NAME);
    if (IS_ERR(chardrvs_ptr->device)) {
        ret = PTR_ERR(chardrvs_ptr->device);
        pr_err("Failed to create device: %d\n", ret);
        goto err_dev_create;
    }

    // Initialize cdev
    cdev_init(&chardrvs_ptr->new_cdev, &chardrvs_fops);

    ret = cdev_add(&chardrvs_ptr->new_cdev, chardrvs_ptr->dev_num, NR_MINORS);
    if (ret) {
        pr_err("Failed to add cdev: %d\n", ret);
        goto err_char_dev_add;
    }

	ret = setup_chardrvs(chardrvs_ptr);
	if (ret) {
		pr_err("Could not allocate FIFO\n");
		goto err_unregister_cdev;
	}

	pr_info("chardrvs driver registered with major %d\n", MAJOR(chardrvs_ptr->dev_num));
	return 0;

err_unregister_cdev:
    cdev_del(&chardrvs_ptr->new_cdev);
err_char_dev_add:
    device_destroy(chardrvs_ptr->new_class, chardrvs_ptr->dev_num);
err_dev_create:
    class_destroy(chardrvs_ptr->new_class);
err_class_create:
    unregister_chrdev_region(chardrvs_ptr->dev_num, NR_MINORS);
err_free_dev:
    kfree(chardrvs_ptr);
    return ret;
}

static void __exit chardrvs_exit(void)
{
    cdev_del(&chardrvs_ptr->new_cdev);
    device_destroy(chardrvs_ptr->new_class, chardrvs_ptr->dev_num);
    class_destroy(chardrvs_ptr->new_class);
    unregister_chrdev_region(chardrvs_ptr->dev_num, NR_MINORS);
    uninstall_chardrvs(chardrvs_ptr);
    kfree(chardrvs_ptr);
	pr_info("Removing chardrvs driver\n");
}
