/*
 * Samsung Exynos5 SoC series Sensor driver
 *
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <mach/exynos-fimc-is-sensor.h>

#include "../fimc-is-core.h"
#include "../fimc-is-device-sensor.h"
#include "../fimc-is-resourcemgr.h"
#include "../fimc-is-hw.h"
#include "fimc-is-device-3h7_sunny.h"

#define SENSOR_NAME "S5K3H7_SUNNY"

static struct fimc_is_sensor_cfg config_3h7_sunny[] = {
	/* 3264x2448@30fps */
	FIMC_IS_SENSOR_CFG(3264, 2448, 30, 15, 0),
};

static int sensor_3h7_sunny_init(struct v4l2_subdev *subdev, u32 val)
{
	int ret = 0;
	struct fimc_is_module_enum *module;

	BUG_ON(!subdev);

	module = (struct fimc_is_module_enum *)v4l2_get_subdevdata(subdev);

	pr_info("[MOD:D:%d] %s(%d)\n", module->id, __func__, val);

	return ret;
}

static const struct v4l2_subdev_core_ops core_ops = {
	.init = sensor_3h7_sunny_init
};

static const struct v4l2_subdev_ops subdev_ops = {
	.core = &core_ops
};

int sensor_3h7_sunny_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct fimc_is_core *core;
	struct v4l2_subdev *subdev_module;
	struct fimc_is_module_enum *module;
	struct fimc_is_device_sensor *device;
	struct sensor_open_extended *ext;

	BUG_ON(!fimc_is_dev);

	core = (struct fimc_is_core *)dev_get_drvdata(fimc_is_dev);
	if (!core) {
		err("core device is not yet probed");
		return -EPROBE_DEFER;
	}

	device = &core->sensor[SENSOR_S5K3H7_SUNNY_INSTANCE];

	subdev_module = kzalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
	if (!subdev_module) {
		err("subdev_module is NULL");
		ret = -ENOMEM;
		goto p_err;
	}

	module = &device->module_enum[atomic_read(&core->resourcemgr.rsccount_module)];
	atomic_inc(&core->resourcemgr.rsccount_module);
	module->id = SENSOR_NAME_S5K3H7_SUNNY;
	module->subdev = subdev_module;
	module->device = SENSOR_S5K3H7_SUNNY_INSTANCE;
	module->client = client;
	module->active_width = 3248;
	module->active_height = 2438;
	module->pixel_width = module->active_width + 16;
	module->pixel_height = module->active_height + 10;
	module->max_framerate = 30;
	module->position = SENSOR_POSITION_REAR;
	module->mode = CSI_MODE_CH0_ONLY;
	module->lanes = CSI_DATA_LANES_4;
	module->setfile_name = "setfile_3h7_sunny.bin";
	module->cfgs = ARRAY_SIZE(config_3h7_sunny);
	module->cfg = config_3h7_sunny;
	module->ops = NULL;
	module->private_data = NULL;

	ext = &module->ext;
	ext->mipi_lane_num = module->lanes;
	ext->I2CSclk = I2C_L0;

	ext->sensor_con.product_name = 0;
	ext->sensor_con.peri_type = SE_I2C;
	ext->sensor_con.peri_setting.i2c.channel = SENSOR_CONTROL_I2C0;
	ext->sensor_con.peri_setting.i2c.slave_address = 0x20;
	ext->sensor_con.peri_setting.i2c.speed = 400000;

	ext->actuator_con.product_name = ACTUATOR_NAME_DWXXXX;
	ext->actuator_con.peri_type = SE_I2C;
	ext->actuator_con.peri_setting.i2c.channel = SENSOR_CONTROL_I2C0;
	ext->actuator_con.peri_setting.i2c.slave_address = 0x20;
	ext->actuator_con.peri_setting.i2c.speed = 400000;

#if defined(CONFIG_EXYNOS5260_XYREF_REV0)
	ext->flash_con.product_name = FLADRV_NAME_NOTHING;
#else
	ext->flash_con.product_name = FLADRV_NAME_AS3643;
#endif
	ext->flash_con.peri_type = SE_I2C;
	ext->flash_con.peri_setting.gpio.first_gpio_port_no = 5;
#if defined(CONFIG_MACH_XYREF4415)
	ext->flash_con.peri_setting.gpio.second_gpio_port_no = 6;
#else
	ext->flash_con.peri_setting.gpio.second_gpio_port_no = 8;
#endif

	ext->from_con.product_name = FROMDRV_NAME_NOTHING;

	ext->companion_con.product_name = COMPANION_NAME_NOTHING;

	if (client)
		v4l2_i2c_subdev_init(subdev_module, client, &subdev_ops);
	else
		v4l2_subdev_init(subdev_module, &subdev_ops);

	v4l2_set_subdevdata(subdev_module, module);
	v4l2_set_subdev_hostdata(subdev_module, device);
	snprintf(subdev_module->name, V4L2_SUBDEV_NAME_SIZE, "sensor-subdev.%d", module->id);

p_err:
	info("%s(%d)\n", __func__, ret);
	return ret;
}
