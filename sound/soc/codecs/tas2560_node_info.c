#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/firmware.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#define SMARTPA_IC_NAME "TAS2560-Smartpa"


static ssize_t msm_read_smartpa_name(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s\n", SMARTPA_IC_NAME);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(smartpa_name, 0644, msm_read_smartpa_name, NULL);

static struct kobject *msm_smartpa_name;
static int msm_smartpa_name_create_sysfs(void)
{
	int ret ;

	msm_smartpa_name = kobject_create_and_add("android_smartpa", NULL);
	if (msm_smartpa_name == NULL) {
		pr_err("msm_smartpa_name_create_sysfs	failed!\n");
		ret = -ENOMEM;
		return ret ;
	}

	ret = sysfs_create_file(msm_smartpa_name, &dev_attr_smartpa_name.attr);
	if (ret) {
		pr_err("%s failed\n",__func__);
		kobject_del(msm_smartpa_name);
		ret = -ENOMEM;
		return ret ;
	}
	return 0 ;
}

static int __init tas2560_node_init(void)
{
	if(msm_smartpa_name_create_sysfs()){
		pr_err("%s failed!\n",__func__);
		return 1;
	}
	pr_debug("%s success!\n",__func__);
	return 0;
}

module_init(tas2560_node_init);


MODULE_AUTHOR("Texas Instruments Inc.");
MODULE_DESCRIPTION("TAS2560 Node info Smart Amplifier driver");
MODULE_LICENSE("GPLv2");

