
/* drivers/misc/timed_gpio.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/gpio.h>

#include <linux/of_gpio.h>
#include "../staging/android/timed_output.h"
///#define gpio_vib  //lc mike_zhu add 20170905
struct vibrator_gpio_data {

	struct hrtimer vib_timer;
	struct timed_output_dev timed_dev;
	struct work_struct work;
	const char *name;
	unsigned int gpio_one;
#if defined 	gpio_vib
	unsigned int gpio_two;
#endif	
	int		timeout;
	u8		active_low;
	spinlock_t lock;

};

/*static int vibrator_set_gpio(struct vibrator_gpio_data *pdata, int on)
{
	int rc;

	if (on) {
		gpio_set_value(vib->gpio,1);
		rc = pinctrl_select_state(pdata->p_config->pinctrl, pdata->p_config->state);
		gpio_direction_output(pdata->gpio_n,1);
		pr_debug("set pdata on %d\n",gpio_get_value(pdata->gpio));
		
	} else {
		rc = pinctrl_select_state(pdata->p_config->pinctrl, pdata->p_config->state);
		gpio_direction_output(pdata->gpio,0);
		gpio_set_value(pdata->gpio,0);
		pr_debug("set pdata off %d\n",gpio_get_value(vib->gpio));

	}

	return rc;
}
*/
static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct vibrator_gpio_data *pdata =
		container_of(timer, struct vibrator_gpio_data, vib_timer);
	#if defined 	gpio_vib
		pr_err("vibrator_timer_func pdata->gpio_two=%d\n",pdata->gpio_two);
		gpio_direction_output(pdata->gpio_two, pdata->active_low ? 1 : 0);
	#endif 		
	
	pr_err("vibrator_timer_func pdata->gpio_one=%d\n",pdata->gpio_one);
	gpio_direction_output(pdata->gpio_one, pdata->active_low ? 1 : 0);

	return HRTIMER_NORESTART;
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	struct vibrator_gpio_data *pdata;
	struct timeval t;
	pdata = container_of(dev, struct vibrator_gpio_data,
							 timed_dev);

	if (hrtimer_active(&pdata->vib_timer)) 
	{
		t = ktime_to_timeval(hrtimer_get_remaining(&pdata->vib_timer));

		return t.tv_sec * 1000 + t.tv_usec / 1000;
	} 
	else
	
		return 0;
}


static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	struct vibrator_gpio_data *pdata = container_of(dev, struct vibrator_gpio_data,
					 timed_dev);
	unsigned long	flags;
	
	spin_lock_irqsave(&pdata->lock, flags);
		pr_err("vibrator_enable start------\n");
#if 0//temp for test 
	gpio_free(pdata->gpio_one);
	#if defined 	gpio_vib
	gpio_free(pdata->gpio_two);
	#endif
	#if defined 	gpio_vib
	ret = gpio_request(pdata->gpio_two, pdata->name);
	if (ret < 0)
	{
		pr_err("gpio_request gpio_two  fail \n");
		gpio_free(pdata->gpio_two);
		//goto err_out;
		
	}	
		#endif

	ret = gpio_request(pdata->gpio_one, pdata->name);
	if (ret < 0)
	{
		pr_err("gpio_request  gpio_one  fail \n");
		gpio_free(pdata->gpio_one);
		//goto err_out;
		
	}

	ret = timed_output_dev_register(&pdata->timed_dev);
	if (ret < 0)
		{
		gpio_free(pdata->gpio_one);
		#if defined 	gpio_vib
		gpio_free(pdata->gpio_two);
		#endif
		//goto err_out;
		
	}
	gpio_direction_output(pdata->gpio_one, 1);	
	#if defined 	gpio_vib
	gpio_direction_output(pdata->gpio_two, 1);
	#endif
#endif

	
	/* cancel previous timer and set GPIO according to value */
	hrtimer_cancel(&pdata->vib_timer);
	#if defined 	gpio_vib
	gpio_direction_output(pdata->gpio_two, pdata->active_low ? !value : !!value);
	pr_err("gpio_enable data->gpio_two=%d\n",pdata->gpio_two);
	#endif
	gpio_direction_output(pdata->gpio_one, pdata->active_low ? !value : !!value);
	pr_err("gpio_enable data->gpio_one=%d\n",pdata->gpio_one);

	if (value >= 0) 
	{
		if (value > pdata->timeout)
			value = pdata->timeout;

		hrtimer_start(&pdata->vib_timer,
			ktime_set(value / 1000, (value % 1000) * 1000000),
			HRTIMER_MODE_REL);
	}
		pr_err("vibrator_enable end------\n");

	spin_unlock_irqrestore(&pdata->lock, flags);
}
/*
static void vibrator_update(struct work_struct *work)
{
	struct vibrator_gpio_data *pdata = container_of(work, struct vibrator_gpio_data,
					 work);
	vibrator_set_gpio(pdata, pdata->state);
}
*/




static int vibrator_parse_dt(struct device *dev,struct vibrator_gpio_data *pdata)
{
	struct device_node *node = dev->of_node;
	int rc;
	
	u32 temp_val;

		#if defined 	gpio_vib
	pdata->gpio_two = of_get_named_gpio_flags(node,"qcom,vib-gpio_two",0, &temp_val);
	pr_err(" gpio_two=%d flag=%d\n",pdata->gpio_two,temp_val);
	if (!gpio_is_valid(pdata->gpio_two))
	{
		dev_err(dev, "Failed to read vib-gpio_two\n");
		return -EINVAL;
	}
		#endif		

	pdata->gpio_one = of_get_named_gpio_flags(node,"qcom,vib-gpio_one",0, &temp_val);
	pr_err(" gpio_one=%d flag=%d\n",pdata->gpio_one,temp_val);
	if (!gpio_is_valid(pdata->gpio_one))
	{
		dev_err(dev, "Failed to read vib-gpio_one\n");
		return -EINVAL;
	}

		if (NULL == (pdata->name = of_get_property(node, "label", NULL)))
	{
		pr_err("get label error\n");
		return 1;
	}
	pr_err("pdata-name=%s\n",pdata->name);
	rc = of_property_read_u32(node,"qcom,vibrator-timeout-ms", &pdata->timeout);
	
	if (rc) 
	{
		pdata->timeout = 10000;
	} 
		

	return 0;
}



static int vibrator_gpio_probe(struct platform_device *pdev)
{
	
	struct vibrator_gpio_data *pdata;
	
	int  ret;
	pr_err("vibrator gpio probe enter\n");
	
	pdata =devm_kzalloc(&pdev->dev, sizeof(struct vibrator_gpio_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;
	
	
	ret= vibrator_parse_dt(&pdev->dev,pdata);
	if (ret)
		{
			dev_err(&pdev->dev,"DT parsing failed%d\n",ret);
			return ret;
		}
		
	hrtimer_init(&pdata->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	spin_lock_init(&pdata->lock);
	/*INIT_WORK(&pdata->work, vibrator_update);*/
	
	
	pdata->vib_timer.function = vibrator_timer_func;

	pdata->timed_dev.name = pdata->name;
	pdata->timed_dev.get_time = vibrator_get_time;
	pdata->timed_dev.enable = vibrator_enable;

	
	#if defined 	gpio_vib
	ret = gpio_request(pdata->gpio_two, pdata->name);
	if (ret < 0)
	{
		gpio_free(pdata->gpio_two);
		goto err_out;
		
	}	
		#endif

	ret = gpio_request(pdata->gpio_one, pdata->name);
	if (ret < 0)
	{
		gpio_free(pdata->gpio_one);
		goto err_out;
		
	}

	ret = timed_output_dev_register(&pdata->timed_dev);
	if (ret < 0)
		{
		gpio_free(pdata->gpio_one);
		#if defined 	gpio_vib
		gpio_free(pdata->gpio_two);
		#endif
		goto err_out;
		
	}

	pdata->active_low = 0;
	gpio_direction_output(pdata->gpio_one, 0);	
	#if defined 	gpio_vib
	gpio_direction_output(pdata->gpio_two, 0);
	#endif

	platform_set_drvdata(pdev, pdata);
	pr_err("vibrator gpio probe success");
	return 0;
err_out:
		pr_err("timed_gpio_probe Failed \n");
		timed_output_dev_unregister(&pdata->timed_dev);
		kfree(pdata);
	return ret;
}


static int vibrator_gpio_remove(struct platform_device *pdev)
{
	struct vibrator_gpio_data *pdata = platform_get_drvdata(pdev);

	
	
	timed_output_dev_unregister(&pdata->timed_dev);
	gpio_free(pdata->gpio_one);
	#if defined 	gpio_vib
	gpio_free(pdata->gpio_two);
	#endif

	return 0;
}

static  struct of_device_id vibrator_gpio_machine_of_match[]  = {
	{ .compatible = "qcom,vibrator-gpio", },
	{},
};
static struct platform_driver vibrator_gpio_driver = {
	.probe = vibrator_gpio_probe,
	.remove = vibrator_gpio_remove,
	.driver = {
		.name = "vibrator_gpio",
		.of_match_table = vibrator_gpio_machine_of_match,
	},
};



module_platform_driver(vibrator_gpio_driver);

MODULE_AUTHOR("Jason LCT");
MODULE_DESCRIPTION("Vibrator gpio driver");
MODULE_LICENSE("GPL");
