#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define MYDEV_NAME "mycdrv"

#define ramdisk_size (size_t) (16 * PAGE_SIZE) 

#define CDRV_IOC_MAGIC 'Z'

#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)


static int NUM_DEVICES = 3;


struct asp_mycdev {
	struct list_head list;
	struct cdev cdev;
	char *ramDisk;
    unsigned long buffer_size;
	struct semaphore sem;
	int devNo;
};


module_param(NUM_DEVICES, int, S_IRUGO);
static unsigned int mycdev_major = 0;
static struct class *class_device = NULL;
LIST_HEAD(DeviceListHead);

int driver_open(struct inode *inode, struct file *filp)
{
        unsigned int mj = imajor(inode);
        unsigned int mn = iminor(inode);
	    struct list_head *pos = NULL;
        struct asp_mycdev *device = NULL;

	if (mj != mycdev_major || mn < 0 || mn >= NUM_DEVICES) {
                printk("No device found!!\n");
                return -ENODEV;
        }

	list_for_each(pos, &DeviceListHead) {
		device = list_entry(pos, struct asp_mycdev, list);
		if(device->devNo == mn) {
			break;
		}
	}

        filp->private_data = device; 

        return 0;
}

int driver_release(struct inode *inode, struct file *filp)
{
        return 0;
}

ssize_t driver_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	ssize_t retval = 0;
	unsigned char *temp_buff = NULL;
	loff_t i;

	down_interruptible(&(dev->sem));

	if (*f_pos >= dev->buffer_size) { /* EOF */
		up(&(dev->sem));
	        return retval;
        }

	temp_buff = (unsigned char*) kmalloc(count, GFP_KERNEL);
	memset(temp_buff, 0, count);	

	for (i = 0; i < count; i++) {
		temp_buff[i] = dev->ramDisk[*f_pos + i];
	}
	*f_pos += count;

	if (copy_to_user(buf, temp_buff, count) != 0)
	{
		retval = -EFAULT;
		kfree(temp_buff);
                up(&(dev->sem));
                return retval;
	}

	retval = count;

    kfree(temp_buff);
	up(&(dev->sem));
	return retval;
}

ssize_t driver_write(struct file *filp, const char __user *buf, size_t count, 
		loff_t *f_pos)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	ssize_t retval = 0;
	unsigned char *temp_buff = NULL;
	loff_t i;

	down_interruptible(&(dev->sem));

	if (*f_pos >= dev->buffer_size) {
		retval = -EINVAL;
		up(&(dev->sem));
		return retval;
	} 

	temp_buff = (unsigned char*) kmalloc(count, GFP_KERNEL);
    memset(temp_buff, 0, count);

	if (copy_from_user(temp_buff, buf, count) != 0)
	{
		retval = -EFAULT;
		kfree(temp_buff);
                up(&(dev->sem));
	        return retval;
	}

	for (i = 0; i < count; i++) {
		dev->ramDisk[*f_pos + i] = temp_buff[i];
	}
	*f_pos += count;

	retval = count;

	kfree(temp_buff);
	up(&(dev->sem));
	return retval;
}

long driver_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
        struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;

	if (cmd != ASP_CLEAR_BUF) {
		pr_info("Wrong command\n");
		return -1;
	}

	down_interruptible(&(dev->sem));

        memset((volatile void *)dev->ramDisk, 0, dev->buffer_size);  
        filp->f_pos = 0;						

	up(&(dev->sem));
	return 1;
}

loff_t driver_llseek(struct file *filp, loff_t off, int whence)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	loff_t newpos = 0;       

	down_interruptible(&(dev->sem));

	switch(whence) {
		case 0: //seek set
			newpos = off;
			break;

		case 1: //seek current
			newpos = filp->f_pos + off;
			break;

		case 2: //seek end
			newpos = dev->buffer_size + off;
			break;

		default: //wrong option
			newpos = -EINVAL;
	
	}

        
        unsigned char* extensionbuffer;
	if (newpos > dev->buffer_size) {
                extensionbuffer = (unsigned char*)kmalloc(newpos, GFP_KERNEL);
                memset(extensionbuffer, 0, newpos);
                memcpy(extensionbuffer, dev->ramDisk, dev->buffer_size);
                kfree(dev->ramDisk);
                dev->ramDisk = extensionbuffer;
                dev->buffer_size = newpos;
                
	}

	filp->f_pos = newpos;

	up(&(dev->sem));
	return newpos;
}

struct file_operations mycdev_fops = {
        .owner =    THIS_MODULE,
        .read =     driver_read,
        .write =    driver_write,
        .open =     driver_open,
        .release =  driver_release,
        .llseek =   driver_llseek,
	.unlocked_ioctl = driver_ioctl,
};

static int controldev(struct asp_mycdev *dev, int minor, 
        struct class *class)
{
        int err = 0;
        dev_t devno = MKDEV(mycdev_major, minor);
        struct device *device = NULL;

        dev->buffer_size = ramdisk_size;
	dev->devNo = minor;
	dev->ramDisk = NULL;

	sema_init(&(dev->sem),1);
        
        cdev_init(&dev->cdev, &mycdev_fops);
        dev->cdev.owner = THIS_MODULE;

        dev->ramDisk = (unsigned char*)kmalloc(dev->buffer_size, GFP_KERNEL);
        memset(dev->ramDisk, 0, dev->buffer_size);

        err = cdev_add(&dev->cdev, devno, 1);

        device = device_create(class, NULL, devno, NULL,
				MYDEV_NAME "%d", minor);
        if (IS_ERR(device)) {
                err = PTR_ERR(device);
                printk("Error %d while trying to create %s%d", err,
			MYDEV_NAME, minor);
                cdev_del(&dev->cdev);
                return err;
        }

        return 0;
}

static void cleanup(void)
{
        int i = 0;
        struct list_head *pos = NULL;
        struct asp_mycdev *device = NULL;

again: 
	list_for_each(pos, &DeviceListHead) {
		device = list_entry(pos, struct asp_mycdev, list);
		device_destroy(class_device, MKDEV(mycdev_major, i));
        	cdev_del(&device->cdev);
        	kfree(device->ramDisk);
		list_del(&(device->list));
                kfree(device);
		i++;
        	goto again;
        }

	if (class_device)
		class_destroy(class_device);

	unregister_chrdev_region(MKDEV(mycdev_major, 0), NUM_DEVICES);
	return;
}

static int __init my_init(void)
{
        int err = 0;
        int i = 0;
        dev_t device = 0;
        struct asp_mycdev *mycdev_device = NULL;

        if ( alloc_chrdev_region(&device, 0, NUM_DEVICES, MYDEV_NAME) < 0 ) {
                printk("alloc_chrdev_region() failed\n");
                return err;
        }

        mycdev_major = MAJOR(device);
        class_device = class_create(THIS_MODULE, MYDEV_NAME);
        if (IS_ERR(class_device)) {
                err = PTR_ERR(class_device);
                cleanup();
                return err;
        }
 
	for (i = 0; i < NUM_DEVICES; ++i) 
        {     
		mycdev_device = (struct asp_mycdev *)kmalloc(sizeof(struct asp_mycdev), 
						GFP_KERNEL);
		memset(mycdev_device, 0, sizeof(struct asp_mycdev));

		if (mycdev_device == NULL) {
			err = -ENOMEM;
			cleanup();
                        return err;
		}

		err = controldev(mycdev_device, i, class_device);

                if (err) {
                        cleanup();
                        return err;
                }

		INIT_LIST_HEAD(&(mycdev_device->list));
		list_add(&(mycdev_device->list), &DeviceListHead);
        }

        return 0;

}

static void __exit my_exit(void)
{
        cleanup();
        return;
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Niraja");
MODULE_LICENSE("GPL v2");
