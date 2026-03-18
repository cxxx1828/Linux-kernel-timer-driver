#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#define REG_CONTROL     0x00
#define REG_COUNTER_LO  0x04
#define REG_COUNTER_HI  0x08
#define REG_COMPARE(n)  (0x0c + (n) * 4)

#define MAX_TIMER       3
#define DEFAULT_TIMER   1

#define TIMER_IOCTL_SET_INTERVAL _IOW('t',1,int)

struct timer_dev {
	void __iomem *regs;
	struct miscdevice miscdev;
	int irq[MAX_TIMER+1];

	unsigned char timer;
	unsigned timer_freq;
	unsigned timer_ticks;

	unsigned int interval;

	char buffer[64];

	wait_queue_head_t timer_wait;

	struct mutex lock;
};

static unsigned int reg_read(struct timer_dev *dev, int offset)
{
	return readl(dev->regs + offset);
}

static void reg_write(struct timer_dev *dev, int value, int offset)
{
	writel(value, dev->regs + offset);
}

static irqreturn_t timer_irq(int irq, void *dev_id)
{
	struct timer_dev *dev = dev_id;
	unsigned int old_value;

	reg_write(dev,(1<<dev->timer),REG_CONTROL);

	old_value = reg_read(dev,REG_COMPARE(dev->timer));

	reg_write(dev,
		  old_value + dev->timer_ticks,
		  REG_COMPARE(dev->timer));

	wake_up_interruptible(&dev->timer_wait);

	return IRQ_HANDLED;
}

static ssize_t timer_read(struct file *file,char __user *buf,
			     size_t sz,loff_t *ppos)
{
	struct timer_dev *dev;
	DEFINE_WAIT(wait);
	unsigned int len,to_copy;

	dev = container_of(file->private_data,
			   struct timer_dev,
			   miscdev);

	prepare_to_wait(&dev->timer_wait,
			&wait,
			TASK_INTERRUPTIBLE);

	schedule();

	finish_wait(&dev->timer_wait,
		    &wait);

	mutex_lock(&dev->lock);

	len = strlen(dev->buffer);

	to_copy = min(sz,len);

	if(copy_to_user(buf,dev->buffer,to_copy)){
		mutex_unlock(&dev->lock);
		return -EFAULT;
	}

	mutex_unlock(&dev->lock);

	return to_copy;
}

static ssize_t timer_write(struct file *file,
			   const char __user *buf,
			   size_t sz,
			   loff_t *ppos)
{
	struct timer_dev *dev;

	dev = container_of(file->private_data,
			   struct timer_dev,
			   miscdev);

	mutex_lock(&dev->lock);

	if(copy_from_user(dev->buffer,
	                  buf,
	                  min(sz,sizeof(dev->buffer)))){
		mutex_unlock(&dev->lock);
		return -EFAULT;
	}

	mutex_unlock(&dev->lock);

	return sz;
}

static long timer_ioctl(struct file *file,
			unsigned int cmd,
			unsigned long arg)
{
	struct timer_dev *dev;

	dev = container_of(file->private_data,
			   struct timer_dev,
			   miscdev);

	switch(cmd){

	case TIMER_IOCTL_SET_INTERVAL:

		mutex_lock(&dev->lock);

		if(copy_from_user(&dev->interval,
		                  (int __user *)arg,
		                  sizeof(int))){
			mutex_unlock(&dev->lock);
			return -EFAULT;
		}

		dev->timer_ticks =
		    dev->timer_freq * dev->interval;

		mutex_unlock(&dev->lock);

		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations timer_fops = {
	.owner = THIS_MODULE,
	.write = timer_write,
	.read = timer_read,
	.unlocked_ioctl = timer_ioctl,
};

static int timer_probe(struct platform_device *pdev)
{
	struct timer_dev *dev;
	struct resource *res;
	unsigned int freq;
	int i;

	dev = devm_kzalloc(&pdev->dev,
			   sizeof(struct timer_dev),
			   GFP_KERNEL);

	if(!dev)
		return -ENOMEM;

	res = platform_get_resource(pdev,
				    IORESOURCE_MEM,
				    0);

	if(!res)
		return -ENODEV;

	dev->regs = devm_ioremap_resource(&pdev->dev,
					  res);

	if(!dev->regs)
		return -ENOMEM;

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	of_property_read_u32(pdev->dev.of_node,
			     "clock-frequency",
			     &freq);

	dev->timer_freq = freq;
	dev->timer = DEFAULT_TIMER;
	dev->timer_ticks = dev->timer_freq;

	init_waitqueue_head(&dev->timer_wait);

	mutex_init(&dev->lock);

	for(i=0;i<=MAX_TIMER;i++)
		dev->irq[i] = platform_get_irq(pdev,i);

	reg_write(dev,
		  (1<<dev->timer),
		  REG_CONTROL);

	devm_request_irq(&pdev->dev,
			 dev->irq[dev->timer],
			 timer_irq,
			 0,
			 "linuxtimer",
			 dev);

	reg_write(dev,
		  reg_read(dev,REG_COUNTER_LO)
		  + dev->timer_ticks,
		  REG_COMPARE(dev->timer));

	dev->miscdev.minor = MISC_DYNAMIC_MINOR;

	dev->miscdev.name =
	devm_kasprintf(&pdev->dev,
		       GFP_KERNEL,
		       "timer-%x",
		       res->start);

	dev->miscdev.fops = &timer_fops;

	misc_register(&dev->miscdev);

	platform_set_drvdata(pdev,dev);

	return 0;
}

static int timer_remove(struct platform_device *pdev)
{
	struct timer_dev *dev =
	platform_get_drvdata(pdev);

	misc_deregister(&dev->miscdev);

	pm_runtime_disable(&pdev->dev);

	return 0;
}

static struct of_device_id timer_dt_match[] = {
	{ .compatible = "linux,timer" },
	{ },
};

MODULE_DEVICE_TABLE(of,timer_dt_match);

static struct platform_driver timer_driver = {
	.driver = {
		.name = "linuxtimer",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(timer_dt_match),
	},
	.probe = timer_probe,
	.remove = timer_remove,
};

module_platform_driver(timer_driver);

MODULE_LICENSE("GPL");
