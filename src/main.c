/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/util.h>

#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

const struct device *dev;

static void process_sample(const struct device *dev)
{
	struct sensor_value temp, hum;
	if (sensor_sample_fetch(dev) < 0) {
		printk("Sensor sample update error\n");
		return;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		printk("Cannot read HTS221 temperature channel\n");
		return;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
		printk("Cannot read HTS221 humidity channel\n");
		return;
	}

	/* display temperature */
	printk("Temperature:%.1f C\n", sensor_value_to_double(&temp));

	/* display humidity */
	printk("Relative Humidity:%.1f%%\n",
	       sensor_value_to_double(&hum));
}

void init_usb_serial(void) {
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return;
	}

	/* Poll if the DTR flag was set */
	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}
	k_sleep(K_MSEC(5000)); //give user time to start 'screen /dev/ttyACM0'
}

void main(void)
{
	init_usb_serial();	//so we can printk to console

    dev = device_get_binding("HTS221");  //string must match string in device tree
 	//dev = device_get_binding(DT_LABEL(DT_INST(0, st_hts221)));

	if (dev == NULL) {
		printk("Could not get HTS221 device\n");
		return;
	}

	while (true) {
		process_sample(dev);
		k_sleep(K_MSEC(2000));
	}
}
