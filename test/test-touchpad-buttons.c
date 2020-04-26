/*
 * Copyright © 2014 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#include <check.h>
#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <unistd.h>

#include "libinput-util.h"
#include "litest.h"

START_TEST(touchpad_button)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (!libevdev_has_event_code(dev->evdev, EV_KEY, BTN_LEFT))
		return;

	litest_drain_events(li);

	litest_button_click(dev, BTN_LEFT, true);
	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_click_defaults_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods, method;
	enum libinput_config_status status;

	/* call this test for apple touchpads */

	methods = libinput_device_config_click_get_methods(device);
	ck_assert(methods & LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	ck_assert(methods & LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);

	method = libinput_device_config_click_get_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	method = libinput_device_config_click_get_default_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);

	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_NONE);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(touchpad_click_defaults_btnarea)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods, method;
	enum libinput_config_status status;

	/* call this test for non-apple clickpads */

	methods = libinput_device_config_click_get_methods(device);
	ck_assert(methods & LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	ck_assert(methods & LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);

	method = libinput_device_config_click_get_method(device);
	ck_assert_int_eq(method,  LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	method = libinput_device_config_click_get_default_method(device);
	ck_assert_int_eq(method,  LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);

	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_NONE);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(touchpad_click_defaults_none)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods, method;
	enum libinput_config_status status;

	if (libevdev_get_id_vendor(dev->evdev) == VENDOR_ID_APPLE &&
	    libevdev_get_id_product(dev->evdev) == PRODUCT_ID_APPLE_APPLETOUCH)
		return;

	/* call this test for non-clickpads and non-touchpads */

	methods = libinput_device_config_click_get_methods(device);
	ck_assert_int_eq(methods, 0);

	method = libinput_device_config_click_get_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_CLICK_METHOD_NONE);
	method = libinput_device_config_click_get_default_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_CLICK_METHOD_NONE);

	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
}
END_TEST

START_TEST(touchpad_1fg_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_1fg_clickfinger_no_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_2fg_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_3fg_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 70);
	litest_touch_down(dev, 2, 70, 70);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);

	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_3fg_clickfinger_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) >= 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 70);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_4fg_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	if (litest_slot_count(dev) < 4)
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 70);
	litest_touch_down(dev, 2, 70, 70);
	litest_touch_down(dev, 3, 80, 70);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 3);

	libinput_dispatch(li);

	litest_wait_for_event(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_PRESSED);
	libinput_event_destroy(event);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_4fg_clickfinger_btntool_2slots)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	if (litest_slot_count(dev) >= 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_QUADTAP))
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 70);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_wait_for_event(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_PRESSED);
	libinput_event_destroy(event);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_4fg_clickfinger_btntool_3slots)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	if (litest_slot_count(dev) != 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 70);
	litest_touch_down(dev, 2, 70, 70);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);

	libinput_dispatch(li);

	litest_wait_for_event(li);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_PRESSED);
	libinput_event_destroy(event);
	event = libinput_get_event(li);
	litest_is_button_event(event,
			       BTN_MIDDLE,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_clickfinger_distance)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	double w, h;
	bool small_touchpad = false;
	unsigned int expected_button;

	if (libinput_device_get_size(dev->libinput_device, &w, &h) == 0 &&
	    h < 50.0)
		small_touchpad = true;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 50);
	litest_touch_down(dev, 1, 10, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 50, 5);
	litest_touch_down(dev, 1, 50, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	/* if the touchpad is small enough, we expect all fingers to count
	 * for clickfinger */
	if (small_touchpad)
		expected_button = BTN_RIGHT;
	else
		expected_button = BTN_LEFT;

	litest_assert_button_event(li,
				   expected_button,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   expected_button,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_3fg_clickfinger_distance)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 20);
	litest_touch_down(dev, 1, 10, 15);
	litest_touch_down(dev, 2, 10, 15);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_3fg_clickfinger_distance_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) > 2)
		return;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 15);
	litest_touch_down(dev, 1, 10, 15);
	libinput_dispatch(li);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_2fg_clickfinger_bottom)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	/* this test is run for the T440s touchpad only, makes getting the
	 * mm correct easier */

	libinput_device_config_click_set_method(dev->libinput_device,
						LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);
	litest_drain_events(li);

	/* one above, one below the magic line, vert spread ca 27mm */
	litest_touch_down(dev, 0, 40, 60);
	litest_touch_down(dev, 1, 60, 100);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	/* both below the magic line */
	litest_touch_down(dev, 0, 40, 100);
	litest_touch_down(dev, 1, 60, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	/* one above, one below the magic line, vert spread 17mm */
	litest_touch_down(dev, 0, 50, 75);
	litest_touch_down(dev, 1, 55, 100);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_to_area_method)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_enable_buttonareas(dev);

	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	/* use bottom right corner to catch accidental softbutton right */
	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

}
END_TEST

START_TEST(touchpad_clickfinger_to_area_method_while_down)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_enable_buttonareas(dev);

	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_enable_clickfinger(dev);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_drain_events(li);

	/* use bottom right corner to catch accidental softbutton right */
	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

}
END_TEST

START_TEST(touchpad_area_to_clickfinger_method)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	/* use bottom right corner to catch accidental softbutton right */
	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_enable_buttonareas(dev);

	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

}
END_TEST

START_TEST(touchpad_area_to_clickfinger_method_while_down)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	/* use bottom right corner to catch accidental softbutton right */
	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_enable_buttonareas(dev);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_touch_down(dev, 0, 95, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

}
END_TEST

START_TEST(touchpad_clickfinger_3fg_tool_position)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);
	litest_drain_events(li);

	/* one in thumb area, one in normal area + TRIPLETAP. spread is wide
	 * but any non-palm 3fg touch+click counts as middle */
	litest_touch_down(dev, 0, 20, 99);
	litest_touch_down(dev, 1, 90, 15);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_4fg_tool_position)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 5, 99);
	litest_touch_down(dev, 1, 90, 15);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_appletouch_config)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	uint32_t methods, method;
	enum libinput_config_status status;

	methods = libinput_device_config_click_get_methods(device);
	ck_assert(!(methods & LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS));
	ck_assert(methods & LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);

	method = libinput_device_config_click_get_method(device);
	ck_assert_int_eq(method, LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);

	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_click_set_method(device,
							 LIBINPUT_CONFIG_CLICK_METHOD_NONE);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(touchpad_clickfinger_appletouch_1fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_appletouch_2fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_appletouch_3fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 50, 50);
	litest_touch_down(dev, 2, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_clickfinger_click_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */
	unsigned int button;
	int nslots = litest_slot_count(dev);

	litest_enable_clickfinger(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 50);
	button = BTN_LEFT;

	if (nfingers > 1) {
		if (nslots > 1) {
			litest_touch_down(dev, 1, 50, 50);
		} else {
			litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
		}
		button = BTN_RIGHT;
	}

	if (nfingers > 2) {
		if (nslots > 2) {
			litest_touch_down(dev, 2, 60, 50);
		} else {
			litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
			litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
		}
		button = BTN_MIDDLE;
	}

	litest_button_click(dev, BTN_LEFT, true);

	libinput_dispatch(li);
	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	for (int i = 0; i < 20; i++) {
		litest_push_event_frame(dev);
		switch (nfingers) {
		case 3:
			if (nslots >= nfingers)
				litest_touch_move(dev, 2, 60, 50 + i);
			/* fallthrough */
		case 2:
			if (nslots >= nfingers)
				litest_touch_move(dev, 1, 50, 50 + i);
			/* fallthrough */
		case 1:
			litest_touch_move(dev, 0, 40, 50 + i);
			break;
		}
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_button_click(dev, BTN_LEFT, false);
	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	if (nfingers > 3) {
		if (nslots > 3) {
			litest_touch_up(dev, 2);
		} else {
			litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
			litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
		}
	}

	if (nfingers > 2) {
		if (nslots > 2) {
			litest_touch_up(dev, 1);
		} else {
			litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
		}
	}

	litest_touch_up(dev, 0);


	libinput_dispatch(li);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_btn_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_btn_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);

	litest_drain_events(li);

	/* A clickpad always needs a finger down to tell where the
	   click happens */
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	ck_assert_int_eq(libinput_next_event_type(li), LIBINPUT_EVENT_NONE);
}
END_TEST

START_TEST(clickpad_click_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	libinput_dispatch(li);
	ck_assert_int_eq(libinput_next_event_type(li), LIBINPUT_EVENT_NONE);

	/* now put a second finger down */
	litest_touch_down(dev, 1, 70, 70);
	litest_touch_move_to(dev, 1, 70, 70, 80, 50, 5);
	litest_touch_up(dev, 1);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_finger_pin)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libevdev *evdev = dev->evdev;
	const struct input_absinfo *abs;
	double w, h;
	double dist;

	abs = libevdev_get_abs_info(evdev, ABS_MT_POSITION_X);
	ck_assert_notnull(abs);
	if (abs->resolution == 0)
		return;

	if (libinput_device_get_size(dev->libinput_device, &w, &h) != 0)
		return;

	dist = 100.0/max(w, h);

	litest_drain_events(li);

	/* make sure the movement generates pointer events when
	   not pinned */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 52, 52, 10);
	litest_touch_move_to(dev, 0, 52, 52, 48, 48, 10);
	litest_touch_move_to(dev, 0, 48, 48, 50, 50, 10);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_button_click(dev, BTN_LEFT, true);
	litest_drain_events(li);

	litest_touch_move_to(dev, 0, 50, 50, 50 + dist, 50 + dist, 10);
	litest_touch_move_to(dev, 0, 50 + dist, 50 + dist, 50, 50, 10);
	litest_touch_move_to(dev, 0, 50, 50, 50 - dist, 50 - dist, 10);

	litest_assert_empty_queue(li);

	litest_button_click(dev, BTN_LEFT, false);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_BUTTON);

	/* still pinned after release */
	litest_touch_move_to(dev, 0, 50, 50, 50 + dist, 50 + dist, 10);
	litest_touch_move_to(dev, 0, 50 + dist, 50 + dist, 50, 50, 10);
	litest_touch_move_to(dev, 0, 50, 50, 50 - dist, 50 - dist, 10);

	litest_assert_empty_queue(li);

	/* move to unpin */
	litest_touch_move_to(dev, 0, 50, 50, 70, 70, 10);
	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);
}
END_TEST

START_TEST(clickpad_softbutton_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 10, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_middle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_right)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
			    LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
			    LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_left_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	/* Tap in left button area, then finger down, button click
		-> expect left button press/release and left button press
	   Release button, finger up
		-> expect right button release
	 */
	litest_touch_down(dev, 0, 20, 90);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 20, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
			    LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
			    LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_button_event(li,
				   BTN_LEFT,
			    LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
			    LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_right_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	/* Tap in right button area, then finger down, button click
		-> expect left button press/release and right button press
	   Release button, finger up
		-> expect right button release
	 */
	litest_touch_down(dev, 0, 90, 90);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 90, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_left_1st_fg_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	double x = 0, y = 0;
	int nevents = 0;

	litest_drain_events(li);

	/* One finger down in the left button area, button press
		-> expect a button event
	   Move finger up out of the area, wait for timeout
	   Move finger around diagonally down left
		-> expect motion events down left
	   Release finger
		-> expect a button event */

	/* finger down, press in left button */
	litest_touch_down(dev, 0, 20, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	/* move out of the area, then wait for softbutton timer */
	litest_touch_move_to(dev, 0, 20, 90, 50, 50, 20);
	libinput_dispatch(li);
	litest_timeout_softbuttons();
	libinput_dispatch(li);
	litest_drain_events(li);

	/* move down left, expect motion */
	litest_touch_move_to(dev, 0, 50, 50, 20, 90, 20);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_notnull(event);
	while (event) {
		struct libinput_event_pointer *p;

		ck_assert_int_eq(libinput_event_get_type(event),
				 LIBINPUT_EVENT_POINTER_MOTION);
		p = libinput_event_get_pointer_event(event);

		/* we moved up/right, now down/left so the pointer accel
		   code may lag behind with the dx/dy vectors. Hence, add up
		   the x/y movements and expect that on average we moved
		   left and down */
		x += libinput_event_pointer_get_dx(p);
		y += libinput_event_pointer_get_dy(p);
		nevents++;

		libinput_event_destroy(event);
		libinput_dispatch(li);
		event = libinput_get_event(li);
	}

	ck_assert(x/nevents < 0);
	ck_assert(y/nevents > 0);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_left_2nd_fg_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	litest_drain_events(li);

	/* One finger down in the left button area, button press
		-> expect a button event
	   Put a second finger down in the area, move it right, release
		-> expect motion events right
	   Put a second finger down in the area, move it down, release
		-> expect motion events down
	   Release second finger, release first finger
		-> expect a button event */
	litest_touch_down(dev, 0, 20, 90);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_touch_down(dev, 1, 20, 20);
	litest_touch_move_to(dev, 1, 20, 20, 80, 20, 10);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_notnull(event);
	while (event) {
		struct libinput_event_pointer *p;
		double x, y;

		ck_assert_int_eq(libinput_event_get_type(event),
				 LIBINPUT_EVENT_POINTER_MOTION);
		p = libinput_event_get_pointer_event(event);

		x = libinput_event_pointer_get_dx(p);
		y = libinput_event_pointer_get_dy(p);

		/* Ignore events only containing an unaccelerated motion
		 * vector. */
		if (x != 0 || y != 0) {
			ck_assert(x > 0);
			ck_assert(y == 0);
		}

		libinput_event_destroy(event);
		libinput_dispatch(li);
		event = libinput_get_event(li);
	}
	litest_touch_up(dev, 1);

	/* second finger down */
	litest_touch_down(dev, 1, 20, 20);
	litest_touch_move_to(dev, 1, 20, 20, 20, 80, 10);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_notnull(event);
	while (event) {
		struct libinput_event_pointer *p;
		double x, y;

		ck_assert_int_eq(libinput_event_get_type(event),
				 LIBINPUT_EVENT_POINTER_MOTION);
		p = libinput_event_get_pointer_event(event);

		x = libinput_event_pointer_get_dx(p);
		y = libinput_event_pointer_get_dy(p);

		ck_assert(x == 0);
		ck_assert(y > 0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
		event = libinput_get_event(li);
	}

	litest_touch_up(dev, 1);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_left_to_right)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	/* One finger down in left software button area,
	   move to right button area immediately, click
		-> expect right button event
	*/

	litest_touch_down(dev, 0, 30, 90);
	litest_touch_move_to(dev, 0, 30, 90, 90, 90, 10);
	litest_drain_events(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_right_to_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	/* One finger down in right software button area,
	   move to left button area immediately, click
		-> expect left button event
	*/

	litest_touch_down(dev, 0, 80, 90);
	litest_touch_move_to(dev, 0, 80, 90, 30, 90, 10);
	litest_drain_events(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_softbutton_hover_into_buttons)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_hover_start(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_hover_move_to(dev, 0, 50, 50, 90, 90, 10);
	libinput_dispatch(li);

	litest_touch_move_to(dev, 0, 90, 90, 91, 91, 1);

	litest_button_click(dev, BTN_LEFT, true);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_button_click(dev, BTN_LEFT, false);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 10, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_right)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_middle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_empty_queue(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_move_out_leftclick_before_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	/* Finger down in top right button area, wait past enter timeout
	   Move into main area, wait past leave timeout
	   Click
	     -> expect left click
	 */

	litest_drain_events(li);

	litest_touch_down(dev, 0, 80, 5);
	libinput_dispatch(li);
	litest_timeout_softbuttons();
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	litest_touch_move_to(dev, 0, 80, 5, 80, 90, 20);
	libinput_dispatch(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_touch_up(dev, 0);

	litest_assert_button_event(li, BTN_RIGHT, LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT, LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_move_out_leftclick)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	/* Finger down in top right button area, wait past enter timeout
	   Move into main area, wait past leave timeout
	   Click
	     -> expect left click
	 */

	litest_drain_events(li);

	litest_touch_down(dev, 0, 80, 5);
	libinput_dispatch(li);
	litest_timeout_softbuttons();
	libinput_dispatch(li);
	litest_assert_empty_queue(li);

	litest_touch_move_to(dev, 0, 80, 5, 80, 90, 20);
	libinput_dispatch(li);
	litest_timeout_softbuttons();
	libinput_dispatch(li);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_touch_up(dev, 0);

	litest_assert_button_event(li, BTN_LEFT, LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT, LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_clickfinger)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_clickfinger(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 90, 5);
	litest_touch_down(dev, 1, 80, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(clickpad_topsoftbuttons_clickfinger_dev_disabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct litest_device *trackpoint = litest_add_device(li,
							     LITEST_TRACKPOINT);

	libinput_device_config_send_events_set_mode(dev->libinput_device,
						    LIBINPUT_CONFIG_SEND_EVENTS_DISABLED);
	litest_enable_clickfinger(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 90, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 90, 5);
	litest_touch_down(dev, 1, 10, 5);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_delete_device(trackpoint);
}
END_TEST

START_TEST(clickpad_middleemulation_config_delayed)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	struct libinput *li = dev->libinput;
	enum libinput_config_status status;
	int enabled;

	enabled = libinput_device_config_middle_emulation_get_enabled(device);
	ck_assert(!enabled);

	litest_touch_down(dev, 0, 30, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	/* actual config is delayed, but status is immediate */
	status = libinput_device_config_middle_emulation_set_enabled(device,
				LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	enabled = libinput_device_config_middle_emulation_get_enabled(device);
	ck_assert(enabled);

	status = libinput_device_config_middle_emulation_set_enabled(device,
				LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	enabled = libinput_device_config_middle_emulation_get_enabled(device);
	ck_assert(!enabled);
}
END_TEST

START_TEST(clickpad_middleemulation_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);
	litest_enable_middleemu(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 30, 95);
	litest_touch_down(dev, 1, 80, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_middleemulation_click_middle_left)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);
	litest_enable_middleemu(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 49, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_middleemulation_click_middle_right)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);
	litest_enable_middleemu(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 51, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_middleemulation_click_enable_while_down)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 49, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_enable_middleemu(dev);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 49, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);
}
END_TEST

START_TEST(clickpad_middleemulation_click_disable_while_down)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_buttonareas(dev);
	litest_enable_middleemu(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 30, 95);
	litest_touch_down(dev, 1, 70, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_disable_middleemu(dev);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_dispatch(li);

	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 49, 95);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);
}
END_TEST

TEST_COLLECTION(touchpad_buttons)
{
	struct range finger_count = {1, 4};

	litest_add("touchpad:button", touchpad_button, LITEST_TOUCHPAD, LITEST_CLICKPAD);

	litest_add("touchpad:clickfinger", touchpad_1fg_clickfinger, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_1fg_clickfinger_no_touch, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_2fg_clickfinger, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_3fg_clickfinger, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_3fg_clickfinger_btntool, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_4fg_clickfinger, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_4fg_clickfinger_btntool_2slots, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_4fg_clickfinger_btntool_3slots, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_2fg_clickfinger_distance, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_3fg_clickfinger_distance, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_3fg_clickfinger_distance_btntool, LITEST_CLICKPAD, LITEST_ANY);
	litest_add_for_device("touchpad:clickfinger", touchpad_2fg_clickfinger_bottom, LITEST_SYNAPTICS_TOPBUTTONPAD);
	litest_add("touchpad:clickfinger", touchpad_clickfinger_to_area_method, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger",
		   touchpad_clickfinger_to_area_method_while_down, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger", touchpad_area_to_clickfinger_method, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:clickfinger",
		   touchpad_area_to_clickfinger_method_while_down, LITEST_CLICKPAD, LITEST_ANY);
	/* run those two for the T440 one only so we don't have to worry
	 * about small touchpads messing with thumb detection expectations */
	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_3fg_tool_position, LITEST_SYNAPTICS_TOPBUTTONPAD);
	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_4fg_tool_position, LITEST_SYNAPTICS_TOPBUTTONPAD);

	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_appletouch_config, LITEST_APPLETOUCH);
	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_appletouch_1fg, LITEST_APPLETOUCH);
	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_appletouch_2fg, LITEST_APPLETOUCH);
	litest_add_for_device("touchpad:clickfinger", touchpad_clickfinger_appletouch_3fg, LITEST_APPLETOUCH);

	litest_add_ranged("touchpad:clickfinger", touchpad_clickfinger_click_drag, LITEST_CLICKPAD, LITEST_ANY, &finger_count);

	litest_add("touchpad:click", touchpad_click_defaults_clickfinger, LITEST_APPLE_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:click", touchpad_click_defaults_btnarea, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:click", touchpad_click_defaults_none, LITEST_TOUCHPAD, LITEST_CLICKPAD);
	litest_add("touchpad:click", touchpad_click_defaults_none, LITEST_ANY, LITEST_TOUCHPAD);

	litest_add("touchpad:click", touchpad_btn_left, LITEST_TOUCHPAD|LITEST_BUTTON, LITEST_CLICKPAD);
	litest_add("touchpad:click", clickpad_btn_left, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:click", clickpad_click_n_drag, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);
	litest_add("touchpad:click", clickpad_finger_pin, LITEST_CLICKPAD, LITEST_ANY);

	litest_add("touchpad:softbutton", clickpad_softbutton_left, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_middle, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_right, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_left_tap_n_drag, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_right_tap_n_drag, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_left_1st_fg_move, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_left_2nd_fg_move, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_left_to_right, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_right_to_left, LITEST_CLICKPAD, LITEST_APPLE_CLICKPAD);
	litest_add("touchpad:softbutton", clickpad_softbutton_hover_into_buttons, LITEST_CLICKPAD|LITEST_HOVER, LITEST_APPLE_CLICKPAD);

	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_left, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_right, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_middle, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_move_out_leftclick, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_move_out_leftclick_before_timeout, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_clickfinger, LITEST_TOPBUTTONPAD, LITEST_ANY);
	litest_add("touchpad:topsoftbuttons", clickpad_topsoftbuttons_clickfinger_dev_disabled, LITEST_TOPBUTTONPAD, LITEST_ANY);

	litest_add("touchpad:middleemulation", clickpad_middleemulation_config_delayed, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:middleemulation", clickpad_middleemulation_click, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:middleemulation", clickpad_middleemulation_click_middle_left, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:middleemulation", clickpad_middleemulation_click_middle_right, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:middleemulation", clickpad_middleemulation_click_enable_while_down, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("touchpad:middleemulation", clickpad_middleemulation_click_disable_while_down, LITEST_CLICKPAD, LITEST_ANY);
}
