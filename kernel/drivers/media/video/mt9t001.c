/*
 * Driver for MT9T001 CMOS Image Sensor from Micron
 *
 * Copyright (C) 2010, Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * Based on the MT9M001 driver,
 *
 * Copyright (C) 2008, Guennadi Liakhovetski <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>

#include <media/mt9t001.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

/*
 * mt9m001 i2c address 0x5d
 */

#define MT9T001_CHIP_VERSION				0x00
#define		MT9T001_CHIP_ID				0x1621
#define MT9T001_ROW_START				0x01
#define		MT9T001_ROW_START_MIN			0
#define		MT9T001_ROW_START_DEF			20
#define		MT9T001_ROW_START_MAX			1534
#define MT9T001_COLUMN_START				0x02
#define		MT9T001_COLUMN_START_MIN		0
#define		MT9T001_COLUMN_START_DEF		32
#define		MT9T001_COLUMN_START_MAX		2046
#define MT9T001_WINDOW_HEIGHT				0x03
#define		MT9T001_WINDOW_HEIGHT_MIN		1
#define		MT9T001_WINDOW_HEIGHT_DEF		1535
#define		MT9T001_WINDOW_HEIGHT_MAX		1567
#define MT9T001_WINDOW_WIDTH				0x04
#define		MT9T001_WINDOW_WIDTH_MIN		1
#define		MT9T001_WINDOW_WIDTH_DEF		2047
#define		MT9T001_WINDOW_WIDTH_MAX		2111
#define MT9T001_HORIZONTAL_BLANKING			0x05
#define		MT9T001_HORIZONTAL_BLANKING_MIN		21
#define		MT9T001_HORIZONTAL_BLANKING_MAX		1023
#define MT9T001_VERTICAL_BLANKING			0x06
#define		MT9T001_VERTICAL_BLANKING_MIN		3
#define		MT9T001_VERTICAL_BLANKING_MAX		1023
#define MT9T001_OUTPUT_CONTROL				0x07
#define		MT9T001_OUTPUT_CONTROL_SYNC		(1 << 0)
#define		MT9T001_OUTPUT_CONTROL_CHIP_ENABLE	(1 << 1)
#define		MT9T001_OUTPUT_CONTROL_TEST_DATA	(1 << 6)
#define MT9T001_SHUTTER_WIDTH_HIGH			0x08
#define MT9T001_SHUTTER_WIDTH_LOW			0x09
#define		MT9T001_SHUTTER_WIDTH_MIN		1
#define		MT9T001_SHUTTER_WIDTH_DEF		1561
#define		MT9T001_SHUTTER_WIDTH_MAX		(1024 * 1024)
#define MT9T001_PIXEL_CLOCK				0x0a
#define		MT9T001_PIXEL_CLOCK_INVERT		(1 << 15)
#define		MT9T001_PIXEL_CLOCK_SHIFT_MASK		(7 << 8)
#define		MT9T001_PIXEL_CLOCK_SHIFT_SHIFT		8
#define		MT9T001_PIXEL_CLOCK_DIVIDE_MASK		(0x7f << 0)
#define MT9T001_FRAME_RESTART				0x0b
#define MT9T001_SHUTTER_DELAY				0x0c
#define		MT9T001_SHUTTER_DELAY_MAX		2047
#define MT9T001_RESET					0x0d
#define MT9T001_READ_MODE1				0x1e
#define		MT9T001_READ_MODE_SNAPSHOT		(1 << 8)
#define		MT9T001_READ_MODE_STROBE_ENABLE		(1 << 9)
#define		MT9T001_READ_MODE_STROBE_WIDTH		(1 << 10)
#define		MT9T001_READ_MODE_STROBE_OVERRIDE	(1 << 11)
#define MT9T001_READ_MODE2				0x20
#define		MT9T001_READ_MODE_BAD_FRAMES		(1 << 0)
#define		MT9T001_READ_MODE_LINE_VALID_CONTINUOUS	(1 << 9)
#define		MT9T001_READ_MODE_LINE_VALID_FRAME	(1 << 10)
#define MT9T001_READ_MODE3				0x21
#define		MT9T001_READ_MODE_GLOBAL_RESET		(1 << 0)
#define		MT9T001_READ_MODE_GHST_CTL		(1 << 1)
#define MT9T001_ROW_ADDRESS_MODE			0x22
#define		MT9T001_ROW_SKIP_MASK			(7 << 0)
#define		MT9T001_ROW_BIN_MASK			(3 << 3)
#define		MT9T001_ROW_BIN_SHIFT			3
#define MT9T001_COLUMN_ADDRESS_MODE			0x23
#define		MT9T001_COLUMN_SKIP_MASK		(7 << 0)
#define		MT9T001_COLUMN_BIN_MASK			(3 << 3)
#define		MT9T001_COLUMN_BIN_SHIFT		3
#define MT9T001_GREEN1_GAIN				0x2b
#define MT9T001_BLUE_GAIN				0x2c
#define MT9T001_RED_GAIN				0x2d
#define MT9T001_GREEN2_GAIN				0x2e
#define MT9T001_TEST_DATA				0x32
#define MT9T001_GLOBAL_GAIN				0x35
#define		MT9T001_GLOBAL_GAIN_MIN			8
#define		MT9T001_GLOBAL_GAIN_MAX			1024
#define MT9T001_BLACK_LEVEL				0x49
#define MT9T001_ROW_BLACK_DEFAULT_OFFSET		0x4b
#define MT9T001_BLC_DELTA_THRESHOLDS			0x5d
#define MT9T001_CAL_THRESHOLDS				0x5f
#define MT9T001_GREEN1_OFFSET				0x60
#define MT9T001_GREEN2_OFFSET				0x61
#define MT9T001_BLACK_LEVEL_CALIBRATION			0x62
#define		MT9T001_BLACK_LEVEL_OVERRIDE		(1 << 0)
#define		MT9T001_BLACK_LEVEL_DISABLE_OFFSET	(1 << 1)
#define		MT9T001_BLACK_LEVEL_RECALCULATE		(1 << 12)
#define		MT9T001_BLACK_LEVEL_LOCK_RED_BLUE	(1 << 13)
#define		MT9T001_BLACK_LEVEL_LOCK_GREEN		(1 << 14)
#define MT9T001_RED_OFFSET				0x63
#define MT9T001_BLUE_OFFSET				0x64

struct mt9t001 {
	struct v4l2_subdev subdev;
	struct media_pad pad;

	struct v4l2_mbus_framefmt format;
	struct v4l2_rect crop;

	struct v4l2_ctrl_handler ctrls;

	u8 output_control;
};

static struct mt9t001 *to_mt9t001(struct v4l2_subdev *sd)
{
	return container_of(sd, struct mt9t001, subdev);
}

static int mt9t001_read(struct i2c_client *client, const u8 reg)
{
	s32 data = i2c_smbus_read_word_data(client, reg);
	return data < 0 ? data : swab16(data);
}

static int mt9t001_write(struct i2c_client *client, const u8 reg,
			 const u16 data)
{
	return i2c_smbus_write_word_data(client, reg, swab16(data));
}

static int mt9t001_set_output_control(struct mt9t001 *mt9t001, u8 clear, u8 set)
{
	struct i2c_client *client = v4l2_get_subdevdata(&mt9t001->subdev);
	u8 value = (mt9t001->output_control & ~clear) | set;
	int ret;

	ret = mt9t001_write(client, MT9T001_OUTPUT_CONTROL, value);
	if (ret < 0)
		return ret;

	mt9t001->output_control = value;
	return 0;
}

/* -----------------------------------------------------------------------------
 * V4L2 subdev video operations
 */

static int mt9t001_s_stream(struct v4l2_subdev *subdev, int enable)
{
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);

	/* Switch to master "normal" mode or stop sensor readout */
	return mt9t001_set_output_control(mt9t001,
		enable ? 0 : MT9T001_OUTPUT_CONTROL_CHIP_ENABLE,
		enable ? MT9T001_OUTPUT_CONTROL_CHIP_ENABLE : 0);
}

static int mt9t001_enum_mbus_code(struct v4l2_subdev *subdev,
				  struct v4l2_subdev_fh *fh,
				  struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index > 0)
		return -EINVAL;

	code->code = V4L2_MBUS_FMT_SGRBG10_1X10;
	return 0;
}

static int mt9t001_enum_frame_size(struct v4l2_subdev *subdev,
				   struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_frame_size_enum *fse)
{
	if (fse->index >= 8 || fse->code != V4L2_MBUS_FMT_SGRBG10_1X10)
		return -EINVAL;

	fse->min_width = (MT9T001_WINDOW_WIDTH_DEF + 1) / fse->index;
	fse->max_width = fse->min_width;
	fse->min_height = (MT9T001_WINDOW_HEIGHT_DEF + 1) / fse->index;
	fse->max_height = fse->min_height;

	return 0;
}

static struct v4l2_mbus_framefmt *
__mt9t001_get_pad_format(struct mt9t001 *mt9t001, struct v4l2_subdev_fh *fh,
			 unsigned int pad, enum v4l2_subdev_format_whence which)
{
	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_format(fh, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &mt9t001->format;
	default:
		return NULL;
	}
}

static struct v4l2_rect *
__mt9t001_get_pad_crop(struct mt9t001 *mt9t001, struct v4l2_subdev_fh *fh,
		       unsigned int pad, enum v4l2_subdev_format_whence which)
{
	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_crop(fh, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &mt9t001->crop;
	default:
		return NULL;
	}
}

static int mt9t001_get_format(struct v4l2_subdev *subdev,
			      struct v4l2_subdev_fh *fh,
			      struct v4l2_subdev_format *format)
{
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);

	format->format = *__mt9t001_get_pad_format(mt9t001, fh, format->pad,
						   format->which);
	return 0;
}

static int mt9t001_set_format(struct v4l2_subdev *subdev,
			      struct v4l2_subdev_fh *fh,
			      struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);
	struct v4l2_mbus_framefmt *__format;
	struct v4l2_rect *__crop;
	unsigned int width;
	unsigned int height;
	unsigned int hratio;
	unsigned int vratio;
	int ret;

	__crop = __mt9t001_get_pad_crop(mt9t001, fh, format->pad,
					format->which);

	/* Clamp the width and height to avoid dividing by zero. */
	width = clamp_t(unsigned int, ALIGN(format->format.width, 2),
			max(__crop->width / 8, MT9T001_WINDOW_HEIGHT_MIN + 1),
			__crop->width);
	height = clamp_t(unsigned int, ALIGN(format->format.height, 2),
			 max(__crop->height / 8, MT9T001_WINDOW_HEIGHT_MIN + 1),
			 __crop->height);

	hratio = DIV_ROUND_CLOSEST(__crop->width, width);
	vratio = DIV_ROUND_CLOSEST(__crop->height, height);

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		ret = mt9t001_write(client, MT9T001_ROW_ADDRESS_MODE,
				    hratio - 1);
		if (ret < 0)
			return ret;

		ret = mt9t001_write(client, MT9T001_COLUMN_ADDRESS_MODE,
				    vratio - 1);
		if (ret < 0)
			return ret;
	}

	__format = __mt9t001_get_pad_format(mt9t001, fh, format->pad,
					    format->which);
	__format->width = __crop->width / hratio;
	__format->height = __crop->height / vratio;

	format->format = *__format;

	return 0;
}

static int mt9t001_get_crop(struct v4l2_subdev *subdev,
			    struct v4l2_subdev_fh *fh,
			    struct v4l2_subdev_crop *crop)
{
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);

	crop->rect = *__mt9t001_get_pad_crop(mt9t001, fh, crop->pad,
					     crop->which);
	return 0;
}

static int mt9t001_set_crop(struct v4l2_subdev *subdev,
			    struct v4l2_subdev_fh *fh,
			    struct v4l2_subdev_crop *crop)
{
	struct i2c_client *client = v4l2_get_subdevdata(subdev);
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);
	struct v4l2_mbus_framefmt *__format;
	struct v4l2_rect *__crop;
	struct v4l2_rect rect;
	int ret;

	/* Clamp the crop rectangle boundaries and align them to a multiple of 2
	 * pixels.
	 */
	rect.left = clamp(ALIGN(crop->rect.left, 2),
			  MT9T001_COLUMN_START_MIN,
			  MT9T001_COLUMN_START_MAX);
	rect.top = clamp(ALIGN(crop->rect.top, 2),
			 MT9T001_ROW_START_MIN,
			 MT9T001_ROW_START_MAX);
	rect.width = clamp(ALIGN(crop->rect.width, 2),
			   MT9T001_WINDOW_WIDTH_MIN + 1,
			   MT9T001_WINDOW_WIDTH_MAX + 1 - rect.left);
	rect.height = clamp(ALIGN(crop->rect.height, 2),
			    MT9T001_WINDOW_HEIGHT_MIN + 1,
			    MT9T001_WINDOW_HEIGHT_MAX + 1 - rect.top);

	if (crop->which == V4L2_SUBDEV_FORMAT_TRY) {
		ret = mt9t001_write(client, MT9T001_COLUMN_START, rect.left);
		if (ret < 0)
			return ret;

		ret = mt9t001_write(client, MT9T001_ROW_START, rect.top);
		if (ret < 0)
			return ret;

		ret = mt9t001_write(client, MT9T001_WINDOW_WIDTH,
				    rect.width - 1);
		if (ret < 0)
			return ret;

		ret = mt9t001_write(client, MT9T001_WINDOW_HEIGHT,
				    rect.height - 1);
		if (ret < 0)
			return ret;
	}

	__crop = __mt9t001_get_pad_crop(mt9t001, fh, crop->pad, crop->which);

	if (rect.width != __crop->width || rect.height != __crop->height) {
		/* Reset the output image size if the crop rectangle size has
		 * been modified.
		 */
		__format = __mt9t001_get_pad_format(mt9t001, fh, crop->pad,
						    crop->which);
		__format->width = rect.width;
		__format->height = rect.height;

		if (crop->which == V4L2_SUBDEV_FORMAT_TRY) {
			ret = mt9t001_write(client, MT9T001_ROW_ADDRESS_MODE,
					    0);
			if (ret < 0)
				return ret;

			ret = mt9t001_write(client, MT9T001_COLUMN_ADDRESS_MODE,
					    0);
			if (ret < 0)
				return ret;
		}
	}

	*__crop = rect;
	crop->rect = rect;

	return 0;
}

/* -----------------------------------------------------------------------------
 * V4L2 subdev controls
 */

#define V4L2_CID_TEST_PATTERN          (V4L2_CID_USER_BASE | 0x1001)

static int mt9t001_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct mt9t001 *mt9t001 =
			container_of(ctrl->handler, struct mt9t001, ctrls);
	struct i2c_client *client = v4l2_get_subdevdata(&mt9t001->subdev);
	int data;
	int ret;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		/* Gain is controlled by 2 analog stages and a digital state.
		 * Valid values for the 3 stages are
		 *
		 * Stage		Min	Max	Step
		 * ------------------------------------------
		 * First analog stage	x1	x2	1
		 * Second analog stage	x1	x4	0.125
		 * Digital stage	x1	x16	0.125
		 *
		 * To minimize noise, the gain stages should be used in the
		 * second analog stage, first analog stage, digital stage order.
		 * Gain from a previous stage should be pushed to its maximum
		 * value before the next stage is used.
		 */
		if (ctrl->val <= 32) {
			data = ctrl->val;
		} else if (ctrl->val <= 64) {
			ctrl->val &= ~1;
			data = (1 << 6) | (ctrl->val >> 1);
		} else {
			ctrl->val &= ~7;
			data = ((ctrl->val - 64) << 5) | (1 << 6) | 32;
		}

		ret = mt9t001_write(client, MT9T001_GLOBAL_GAIN, data);
		if (ret < 0)
			return -EIO;

		break;

	case V4L2_CID_EXPOSURE:
		ret = mt9t001_write(client, MT9T001_SHUTTER_WIDTH_LOW,
				    ctrl->val & 0xffff);
		if (ret < 0)
			return -EIO;

		ret = mt9t001_write(client, MT9T001_SHUTTER_WIDTH_HIGH,
				    ctrl->val >> 16);
		if (ret < 0)
			return -EIO;

		break;

	case V4L2_CID_TEST_PATTERN:
		ret = mt9t001_set_output_control(mt9t001,
			ctrl->val ? 0 : MT9T001_OUTPUT_CONTROL_TEST_DATA,
			ctrl->val ? MT9T001_OUTPUT_CONTROL_TEST_DATA : 0);
		if (ret < 0)
			return -EIO;

		ret = mt9t001_write(client, MT9T001_TEST_DATA, ctrl->val << 2);
		if (ret < 0)
			return -EIO;

		break;

	case V4L2_CID_BLACK_LEVEL:
		ret = mt9t001_write(client, MT9T001_BLACK_LEVEL_CALIBRATION,
				    MT9T001_BLACK_LEVEL_RECALCULATE);
		break;
	}

	return 0;
}

static struct v4l2_ctrl_ops mt9t001_ctrl_ops = {
	.s_ctrl = mt9t001_s_ctrl,
};

static const struct v4l2_ctrl_config mt9t001_ctrls[] = {
	{
		.id		= V4L2_CID_TEST_PATTERN,
		.type		= V4L2_CTRL_TYPE_INTEGER,
		.name		= "Test pattern",
		.min		= 0,
		.max		= 1023,
		.step		= 1,
		.def		= 0,
		.flags		= 0,
	},
};

/* -----------------------------------------------------------------------------
 * V4L2 subdev file operations
 */

static int mt9t001_open(struct v4l2_subdev *subdev, struct v4l2_subdev_fh *fh)
{
	struct v4l2_mbus_framefmt *format;
	struct v4l2_rect *crop;

	crop = v4l2_subdev_get_try_crop(fh, 0);
	crop->left = MT9T001_COLUMN_START_DEF;
	crop->top = MT9T001_ROW_START_DEF;
	crop->width = MT9T001_WINDOW_WIDTH_DEF + 1;
	crop->height = MT9T001_WINDOW_HEIGHT_DEF + 1;

	format = v4l2_subdev_get_try_format(fh, 0);
	format->code = V4L2_MBUS_FMT_SGRBG10_1X10;
	format->width = MT9T001_WINDOW_WIDTH_DEF + 1;
	format->height = MT9T001_WINDOW_HEIGHT_DEF + 1;
	format->field = V4L2_FIELD_NONE;
	format->colorspace = V4L2_COLORSPACE_SRGB;

	return 0;
}

static struct v4l2_subdev_file_ops mt9t001_subdev_file_ops = {
	.open		= mt9t001_open,
};

static struct v4l2_subdev_video_ops mt9t001_subdev_video_ops = {
	.s_stream	= mt9t001_s_stream,
};

static struct v4l2_subdev_pad_ops mt9t001_subdev_pad_ops = {
	.enum_mbus_code = mt9t001_enum_mbus_code,
	.enum_frame_size = mt9t001_enum_frame_size,
	.get_fmt = mt9t001_get_format,
	.set_fmt = mt9t001_set_format,
	.get_crop = mt9t001_get_crop,
	.set_crop = mt9t001_set_crop,
};

static struct v4l2_subdev_ops mt9t001_subdev_ops = {
	.file	= &mt9t001_subdev_file_ops,
	.video	= &mt9t001_subdev_video_ops,
	.pad	= &mt9t001_subdev_pad_ops,
};

static int mt9t001_video_probe(struct i2c_client *client)
{
	struct mt9t001_platform_data *pdata = client->dev.platform_data;
	s32 data;
	int ret;

	dev_info(&client->dev, "Probing MT9T001 at address 0x%02x\n",
		 client->addr);

	/* Reset the chip and stop data read out */
	ret = mt9t001_write(client, MT9T001_RESET, 1);
	if (ret < 0)
		return ret;

	ret = mt9t001_write(client, MT9T001_RESET, 0);
	if (ret < 0)
		return ret;

	ret  = mt9t001_write(client, MT9T001_OUTPUT_CONTROL, 0);
	if (ret < 0)
		return ret;

	/* Configure the pixel clock polarity */
	if (pdata && pdata->clk_pol) {
		ret  = mt9t001_write(client, MT9T001_PIXEL_CLOCK,
				     MT9T001_PIXEL_CLOCK_INVERT);
		if (ret < 0)
			return ret;
	}

	/* Read and check the sensor version */
	data = mt9t001_read(client, MT9T001_CHIP_VERSION);
	if (data != MT9T001_CHIP_ID) {
		dev_err(&client->dev, "MT9T001 not detected, wrong version "
			"0x%04x\n", data);
		return -ENODEV;
	}

	dev_info(&client->dev, "MT9T001 detected at address 0x%02x\n",
		 client->addr);

	return ret;
}

static int mt9t001_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct mt9t001 *mt9t001;
	unsigned int i;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_warn(&client->adapter->dev,
			 "I2C-Adapter doesn't support I2C_FUNC_SMBUS_WORD\n");
		return -EIO;
	}

	ret = mt9t001_video_probe(client);
	if (ret < 0)
		return ret;

	mt9t001 = kzalloc(sizeof(*mt9t001), GFP_KERNEL);
	if (!mt9t001)
		return -ENOMEM;

	v4l2_ctrl_handler_init(&mt9t001->ctrls, ARRAY_SIZE(mt9t001_ctrls) + 3);

	v4l2_ctrl_new_std(&mt9t001->ctrls, &mt9t001_ctrl_ops,
			  V4L2_CID_GAIN, MT9T001_GLOBAL_GAIN_MIN,
			  MT9T001_GLOBAL_GAIN_MAX, 1, MT9T001_GLOBAL_GAIN_MIN);
	v4l2_ctrl_new_std(&mt9t001->ctrls, &mt9t001_ctrl_ops,
			  V4L2_CID_EXPOSURE, MT9T001_SHUTTER_WIDTH_MIN,
			  MT9T001_SHUTTER_WIDTH_MAX, 1,
			  MT9T001_SHUTTER_WIDTH_DEF);
	v4l2_ctrl_new_std(&mt9t001->ctrls, &mt9t001_ctrl_ops,
			  V4L2_CID_BLACK_LEVEL, 1, 1, 1, 1);

	for (i = 0; i < ARRAY_SIZE(mt9t001_ctrls); ++i)
		v4l2_ctrl_new_custom(&mt9t001->ctrls, &mt9t001_ctrls[i], NULL);

	mt9t001->subdev.ctrl_handler = &mt9t001->ctrls;

	if (mt9t001->ctrls.error)
		printk(KERN_INFO "%s: control initialization error %d\n",
		       __func__, mt9t001->ctrls.error);

	mt9t001->crop.left = MT9T001_COLUMN_START_DEF;
	mt9t001->crop.top = MT9T001_ROW_START_DEF;
	mt9t001->crop.width = MT9T001_WINDOW_WIDTH_DEF + 1;
	mt9t001->crop.height = MT9T001_WINDOW_HEIGHT_DEF + 1;

	mt9t001->format.code = V4L2_MBUS_FMT_SGRBG10_1X10;
	mt9t001->format.width = MT9T001_WINDOW_WIDTH_DEF + 1;
	mt9t001->format.height = MT9T001_WINDOW_HEIGHT_DEF + 1;
	mt9t001->format.field = V4L2_FIELD_NONE;
	mt9t001->format.colorspace = V4L2_COLORSPACE_SRGB;

	v4l2_i2c_subdev_init(&mt9t001->subdev, client, &mt9t001_subdev_ops);
	mt9t001->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	mt9t001->pad.flags = MEDIA_PAD_FLAG_OUTPUT;
	ret = media_entity_init(&mt9t001->subdev.entity, 1, &mt9t001->pad, 0);
	if (ret < 0)
		kfree(mt9t001);

	return ret;
}

static int mt9t001_remove(struct i2c_client *client)
{
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	struct mt9t001 *mt9t001 = to_mt9t001(subdev);

	v4l2_device_unregister_subdev(subdev);
	media_entity_cleanup(&subdev->entity);
	kfree(mt9t001);
	return 0;
}

static const struct i2c_device_id mt9t001_id[] = {
	{ "mt9t001", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mt9t001_id);

static struct i2c_driver mt9t001_driver = {
	.driver = {
		.name = "mt9t001",
	},
	.probe		= mt9t001_probe,
	.remove		= mt9t001_remove,
	.id_table	= mt9t001_id,
};

static int __init mt9t001_init(void)
{
	return i2c_add_driver(&mt9t001_driver);
}

static void __exit mt9t001_exit(void)
{
	i2c_del_driver(&mt9t001_driver);
}

module_init(mt9t001_init);
module_exit(mt9t001_exit);

MODULE_DESCRIPTION("Micron MT9T001 Camera driver");
MODULE_AUTHOR("Laurent Pinchart <laurent.pinchart@ideasonboard.com>");
MODULE_LICENSE("GPL");
