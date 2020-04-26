/*
 * Copyright © 2014-2015 Red Hat, Inc.
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

START_TEST(touchpad_1fg_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_doubletap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime, curtime;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	msleep(10);
	litest_touch_up(dev, 0);
	msleep(10);
	litest_touch_down(dev, 0, 50, 50);
	msleep(10);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_timeout_tap();

	libinput_dispatch(li);
	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	oldtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_lt(oldtime, curtime);

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_lt(oldtime, curtime);
	oldtime = curtime;

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_lt(oldtime, curtime);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}
	litest_timeout_tap();
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 70, 50, 10);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_gt(curtime, oldtime);

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_2fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i,
	    ntaps;

	if (libevdev_has_property(dev->evdev, INPUT_PROP_SEMI_MT))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	msleep(10);
	litest_touch_down(dev, 1, 70, 50);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_gt(curtime, oldtime);

	litest_touch_move_to(dev, 1, 70, 50, 90, 50, 10);

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);
	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_button_click(dev, BTN_LEFT, true);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_touch_up(dev, 0);
	litest_timeout_tap();

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t ptime, rtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		msleep(10);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		ptime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		rtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_lt(ptime, rtime);
	}

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		msleep(10);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);
		oldtime = curtime;
	}

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_gt(curtime, oldtime);

	litest_touch_move_to(dev, 0, 50, 50, 70, 50, 10);

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_drag_high_delay)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		/* Tap timeout is 180ms after a touch or release. Make sure we
		* go over 180ms for touch+release, but stay under 180ms for
		* each single event. */
		litest_touch_down(dev, 0, 50, 50);
		libinput_dispatch(li);
		msleep(100);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(100);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 50, 70, 10);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		litest_assert_button_event(li, BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_button_event(li, BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_RELEASED);
	}

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		msleep(10);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_gt(curtime, oldtime);

	litest_touch_move_to(dev, 0, 50, 50, 70, 50, 10);

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 70, 50);
	litest_touch_up(dev, 0);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_multitap_n_drag_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		msleep(10);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps < range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	event = libinput_get_event(li);
	ptrev = litest_is_button_event(event,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	curtime = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(event);
	ck_assert_int_gt(curtime, oldtime);

	litest_touch_move_to(dev, 0, 50, 50, 70, 50, 10);

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 70, 50);
	litest_button_click(dev, BTN_LEFT, true);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	/* the physical click */
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	litest_enable_tap(dev->libinput_device);
	litest_disable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);

	/* don't use helper functions here, we expect the event be available
	 * immediately, not after a timeout that the helper functions may
	 * trigger.
	 */
	libinput_dispatch(li);
	event = libinput_get_event(li);
	ck_assert_notnull(event);
	litest_is_button_event(event,
			       BTN_LEFT,
			       LIBINPUT_BUTTON_STATE_RELEASED);
	libinput_event_destroy(event);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag_draglock)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	/* lift finger, set down again, should continue dragging */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);
	litest_touch_up(dev, 0);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_timeout_tap();

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag_draglock_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	/* lift finger, set down again, should continue dragging */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag_draglock_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 80, 20);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	libinput_dispatch(li);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_button_click(dev, BTN_LEFT, true);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	/* the physical click */
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag_draglock_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_enable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_tap();

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_assert_empty_queue(li);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_timeout_tapndrag();
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_n_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_disable_drag_lock(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 30, 70);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 30, 70);
	litest_touch_down(dev, 1, 80, 70);
	litest_touch_move_to(dev, 0, 30, 70, 30, 30, 10);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_n_drag_3fg_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 30, 70);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 30, 70);
	litest_touch_down(dev, 1, 80, 90);
	litest_touch_move_to(dev, 0, 30, 70, 30, 30, 5);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	/* Putting down a third finger should end the drag */
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	/* Releasing the fingers should not cause any events */
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_n_drag_3fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 30, 70);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 30, 70);
	litest_touch_down(dev, 1, 80, 90);
	litest_touch_move_to(dev, 0, 30, 70, 30, 30, 10);
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	/* Putting down a third finger should end the drag */
	litest_touch_down(dev, 2, 50, 50);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	/* Releasing the fingers should not cause any events */
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;
	struct libinput_event *ev;
	struct libinput_event_pointer *ptrev;
	uint64_t ptime, rtime;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_RIGHT;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_MIDDLE;
		break;
	default:
		litest_abort_msg("Invalid map range %d", map);
	}

	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       button,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	ptime = libinput_event_pointer_get_time_usec(ptrev);
	libinput_event_destroy(ev);
	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       button,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	rtime = libinput_event_pointer_get_time_usec(ptrev);
	libinput_event_destroy(ev);

	ck_assert_int_lt(ptime, rtime);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_inverted)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;
	struct libinput_event *ev;
	struct libinput_event_pointer *ptrev;
	uint64_t ptime, rtime;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_RIGHT;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_MIDDLE;
		break;
	default:
		litest_abort_msg("Invalid map range %d", map);
	}

	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       button,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	ptime = libinput_event_pointer_get_time_usec(ptrev);
	libinput_event_destroy(ev);
	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       button,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	rtime = libinput_event_pointer_get_time_usec(ptrev);
	libinput_event_destroy(ev);

	ck_assert_int_lt(ptime, rtime);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_move_on_release)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);

	litest_push_event_frame(dev);
	litest_touch_move(dev, 0, 55, 55);
	litest_touch_up(dev, 1);
	litest_pop_event_frame(dev);

	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_n_hold_first)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
	litest_timeout_tap();

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_n_hold_second)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
	litest_timeout_tap();

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_quickrelease)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* Finger down, finger up -> tap button press
	 * Physical button click -> no button press/release
	 * Tap timeout -> tap button release */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_timeout_tap();

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* two fingers down, left button click, fingers up
	   -> one left button, one right button event pair */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_2fg_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* two fingers down, button click, fingers up
	   -> only one button left event pair */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_2fg_tap_click_apple)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* two fingers down, button click, fingers up
	   -> only one button right event pair
	   (apple have clickfinger enabled by default) */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_no_2fg_tap_after_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(dev->libinput);

	/* one finger down, move past threshold,
	   second finger down, first finger up
	   -> no event
	 */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 1, 70, 50);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_no_2fg_tap_after_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(dev->libinput);

	/* one finger down, wait past tap timeout,
	   second finger down, first finger up
	   -> no event
	 */
	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(dev->libinput);
	litest_timeout_tap();
	libinput_dispatch(dev->libinput);
	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 1, 70, 50);
	litest_touch_up(dev, 0);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_no_first_fg_tap_after_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* one finger down, second finger down,
	   second finger moves beyond threshold,
	   first finger up
	   -> no event
	 */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	libinput_dispatch(dev->libinput);
	litest_touch_move_to(dev, 1, 70, 50, 90, 90, 10);
	libinput_dispatch(dev->libinput);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(dev->libinput);

	while ((event = libinput_get_event(li))) {
		ck_assert_int_ne(libinput_event_get_type(event),
				 LIBINPUT_EVENT_POINTER_BUTTON);
		libinput_event_destroy(event);
	}
}
END_TEST

START_TEST(touchpad_1fg_double_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* one finger down, up, down, button click, finger up
	   -> two button left event pairs */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
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
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_1fg_tap_n_drag_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* one finger down, up, down, move, button click, finger up
	   -> two button left event pairs, motion allowed */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 80, 50, 10);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_assert_only_typed_events(li, LIBINPUT_EVENT_POINTER_MOTION);

	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;
	int i;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_MIDDLE;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_RIGHT;
		break;
	default:
		litest_abort_msg("Invalid map range %d", map);
	}

	for (i = 0; i < 3; i++) {
		uint64_t ptime, rtime;
		struct libinput_event *ev;
		struct libinput_event_pointer *ptrev;

		litest_drain_events(li);

		litest_touch_down(dev, 0, 50, 50);
		msleep(5);
		litest_touch_down(dev, 1, 70, 50);
		msleep(5);
		litest_touch_down(dev, 2, 80, 50);
		msleep(10);

		litest_touch_up(dev, (i + 2) % 3);
		litest_touch_up(dev, (i + 1) % 3);
		litest_touch_up(dev, (i + 0) % 3);

		libinput_dispatch(li);

		ev = libinput_get_event(li);
		ptrev = litest_is_button_event(ev,
					       button,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		ptime = libinput_event_pointer_get_time_usec(ptrev);
		libinput_event_destroy(ev);
		ev = libinput_get_event(li);
		ptrev = litest_is_button_event(ev,
					       button,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		rtime = libinput_event_pointer_get_time_usec(ptrev);
		libinput_event_destroy(ev);

		ck_assert_int_lt(ptime, rtime);

	}
}
END_TEST

START_TEST(touchpad_3fg_tap_tap_again)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int i;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_tap(dev->libinput_device);

	uint64_t ptime, rtime;
	struct libinput_event *ev;
	struct libinput_event_pointer *ptrev;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	msleep(5);
	litest_touch_down(dev, 1, 70, 50);
	msleep(5);
	litest_touch_down(dev, 2, 80, 50);
	msleep(10);
	litest_touch_up(dev, 0);
	msleep(10);
	litest_touch_down(dev, 0, 80, 50);
	msleep(10);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);

	libinput_dispatch(li);

	for (i = 0; i < 2; i++) {
		ev = libinput_get_event(li);
		ptrev = litest_is_button_event(ev,
					       BTN_MIDDLE,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		ptime = libinput_event_pointer_get_time_usec(ptrev);
		libinput_event_destroy(ev);
		ev = libinput_get_event(li);
		ptrev = litest_is_button_event(ev,
					       BTN_MIDDLE,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		rtime = libinput_event_pointer_get_time_usec(ptrev);
		libinput_event_destroy(ev);

		ck_assert_int_lt(ptime, rtime);
	}
}
END_TEST

START_TEST(touchpad_3fg_tap_quickrelease)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_touch_down(dev, 2, 80, 50);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 2);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	libinput_dispatch(li);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap_pressure_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) >= 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	/* libinput doesn't export when it uses pressure detection, so we
	 * need to reconstruct this here. Specifically, semi-mt devices are
	 * non-mt in libinput, so if they have ABS_PRESSURE, they'll use it.
	 */
	if (!libevdev_has_event_code(dev->evdev, EV_ABS, ABS_MT_PRESSURE))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_enable_edge_scroll(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);
	litest_drain_events(li);

	/* drop below the pressure threshold in the same frame as starting a
	 * third touch, see
	 *   E: 8713.954784 0001 014e 0001 # EV_KEY / BTN_TOOL_TRIPLETAP   1
	 * in https://bugs.freedesktop.org/attachment.cgi?id=137672
	 */
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 3);
	litest_event(dev, EV_ABS, ABS_PRESSURE, 3);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_push_event_frame(dev);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_pop_event_frame(dev);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);
}
END_TEST

START_TEST(touchpad_3fg_tap_hover_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) >= 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	/* libinput doesn't export when it uses pressure detection, so we
	 * need to reconstruct this here. Specifically, semi-mt devices are
	 * non-mt in libinput, so if they have ABS_PRESSURE, they'll use it.
	 */
	if (libevdev_has_event_code(dev->evdev, EV_ABS, ABS_MT_PRESSURE))
		return;

	if (libevdev_has_property(dev->evdev, INPUT_PROP_SEMI_MT) &&
	    libevdev_has_event_code(dev->evdev, EV_ABS, ABS_PRESSURE))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_enable_edge_scroll(dev);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	libinput_dispatch(li);

	litest_touch_move_to(dev, 0, 50, 50, 50, 70, 10);
	litest_touch_move_to(dev, 1, 70, 50, 50, 70, 10);
	litest_drain_events(li);

	/* drop below the pressure threshold in the same frame as starting a
	 * third touch  */
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);

	litest_push_event_frame(dev);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_pop_event_frame(dev);
	litest_assert_empty_queue(li);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
}
END_TEST

START_TEST(touchpad_3fg_tap_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;

	if (litest_slot_count(dev) >= 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_MIDDLE;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_RIGHT;
		break;
	default:
		litest_abort_msg("Invalid map range %d", map);
	}

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap_btntool_inverted)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;

	if (litest_slot_count(dev) > 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_MIDDLE;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_RIGHT;
		break;
	default:
		litest_abort_msg("invalid map range %d", map);
	}

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap_btntool_pointerjump)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map = _i; /* ranged test */
	unsigned int button = 0;

	if (litest_slot_count(dev) > 3 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device, map);

	switch (map) {
	case LIBINPUT_CONFIG_TAP_MAP_LRM:
		button = BTN_MIDDLE;
		break;
	case LIBINPUT_CONFIG_TAP_MAP_LMR:
		button = BTN_RIGHT;
		break;
	default:
		litest_abort_msg("Invalid map range %d", map);
	}

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	/* Pointer jump should be ignored */
	litest_touch_move_to(dev, 0, 50, 50, 20, 20, 0);
	libinput_dispatch(li);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, button,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap_slot_release_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	/* Synaptics touchpads sometimes end one touch point after
	 * setting BTN_TOOL_TRIPLETAP.
	 * https://gitlab.freedesktop.org/libinput/libinput/issues/99
	 */
	litest_drain_events(li);
	litest_enable_tap(dev->libinput_device);

	/* touch 1 down */
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, 1);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 2200);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 3200);
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 78);
	litest_event(dev, EV_ABS, ABS_X, 2200);
	litest_event(dev, EV_ABS, ABS_Y, 3200);
	litest_event(dev, EV_ABS, ABS_PRESSURE, 78);
	litest_event(dev, EV_KEY, BTN_TOOL_FINGER, 1);
	litest_event(dev, EV_KEY, BTN_TOUCH, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	msleep(2);

	/* touch 2 and TRIPLETAP down */
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, 1);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 2500);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 3800);
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 73);
	litest_event(dev, EV_KEY, BTN_TOOL_FINGER, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	msleep(2);

	/* touch 2 up, coordinate jump + ends slot 1, TRIPLETAP stays */
	litest_disable_log_handler(li);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 2500);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 3800);
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 78);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_X, 2500);
	litest_event(dev, EV_ABS, ABS_Y, 3800);
	litest_event(dev, EV_ABS, ABS_PRESSURE, 78);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	msleep(2);

	/* slot 2 reactivated
	 */
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 2500);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 3800);
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 78);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, 3);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_X, 3500);
	litest_event(dev, EV_ABS, ABS_MT_POSITION_Y, 3500);
	litest_event(dev, EV_ABS, ABS_MT_PRESSURE, 73);
	litest_event(dev, EV_ABS, ABS_X, 2200);
	litest_event(dev, EV_ABS, ABS_Y, 3200);
	litest_event(dev, EV_ABS, ABS_PRESSURE, 78);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_restore_log_handler(li);

	/* now end all three */
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_3fg_tap_after_scroll)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) <= 3)
		return;

	litest_enable_2fg_scroll(dev);
	litest_enable_tap(dev->libinput_device);

	litest_touch_down(dev, 0, 40, 20);
	litest_touch_down(dev, 1, 50, 20);
	litest_drain_events(li);

	/* 2fg scroll */
	litest_touch_move_two_touches(dev, 40, 20, 50, 20, 0, 20, 10);
	litest_drain_events(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	/* third finger tap without the other two fingers moving */
	litest_touch_down(dev, 2, 60, 40);
	libinput_dispatch(li);
	litest_touch_up(dev, 2);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_4fg_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int i;

	if (litest_slot_count(dev) <= 4)
		return;

	litest_enable_tap(dev->libinput_device);

	for (i = 0; i < 4; i++) {
		litest_drain_events(li);

		litest_touch_down(dev, 0, 50, 50);
		litest_touch_down(dev, 1, 70, 50);
		litest_touch_down(dev, 2, 80, 50);
		litest_touch_down(dev, 3, 90, 50);

		litest_touch_up(dev, (i + 3) % 4);
		litest_touch_up(dev, (i + 2) % 4);
		litest_touch_up(dev, (i + 1) % 4);
		litest_touch_up(dev, (i + 0) % 4);

		libinput_dispatch(li);
		litest_assert_empty_queue(li);
		litest_timeout_tap();
		litest_assert_empty_queue(li);
	}
}
END_TEST

START_TEST(touchpad_4fg_tap_quickrelease)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) <= 4)
		return;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 50);
	litest_touch_down(dev, 2, 80, 50);
	litest_touch_down(dev, 3, 90, 50);

	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 2);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 3);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 0);
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_assert_empty_queue(li);
	litest_timeout_tap();
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_move_after_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int nfingers = _i; /* ranged test */

	if (nfingers > litest_slot_count(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* respective number of fingers down */
	switch(nfingers) {
	case 5:
		litest_touch_down(dev, 4, 70, 30);
		/* fallthrough */
	case 4:
		litest_touch_down(dev, 3, 70, 30);
		/* fallthrough */
	case 3:
		litest_touch_down(dev, 2, 60, 30);
		/* fallthrough */
	case 2:
		litest_touch_down(dev, 1, 50, 30);
		/* fallthrough */
	case 1:
		litest_touch_down(dev, 0, 40, 30);
		/* fallthrough */
		break;
	default:
		abort();
	}

	/* move finger 1 */
	libinput_dispatch(li);
	litest_touch_move_to(dev, 0, 70, 30, 70, 60, 10);
	libinput_dispatch(li);

	/* lift finger 1, put it back */
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_touch_down(dev, 0, 40, 30);
	libinput_dispatch(li);

	/* lift fingers up */
	switch(nfingers) {
	case 5:
		litest_touch_up(dev, 4);
		/* fallthrough */
	case 4:
		litest_touch_up(dev, 3);
		/* fallthrough */
	case 3:
		litest_touch_up(dev, 2);
		/* fallthrough */
	case 2:
		litest_touch_up(dev, 1);
		/* fallthrough */
	case 1:
		litest_touch_up(dev, 0);
		/* fallthrough */
		break;
	}
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_no_typed_events(li, LIBINPUT_EVENT_POINTER_BUTTON);
}
END_TEST

START_TEST(touchpad_5fg_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	int i;

	if (litest_slot_count(dev) < 5)
		return;

	litest_enable_tap(dev->libinput_device);

	for (i = 0; i < 5; i++) {
		litest_drain_events(li);

		litest_touch_down(dev, 0, 20, 50);
		litest_touch_down(dev, 1, 30, 50);
		litest_touch_down(dev, 2, 40, 50);
		litest_touch_down(dev, 3, 50, 50);
		litest_touch_down(dev, 4, 60, 50);

		litest_touch_up(dev, (i + 4) % 5);
		litest_touch_up(dev, (i + 3) % 5);
		litest_touch_up(dev, (i + 2) % 5);
		litest_touch_up(dev, (i + 1) % 5);
		litest_touch_up(dev, (i + 0) % 5);

		libinput_dispatch(li);
		litest_assert_empty_queue(li);
		litest_timeout_tap();
		litest_assert_empty_queue(li);
	}
}
END_TEST

START_TEST(touchpad_5fg_tap_quickrelease)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 5)
		return;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 20, 50);
	litest_touch_down(dev, 1, 30, 50);
	litest_touch_down(dev, 2, 40, 50);
	litest_touch_down(dev, 3, 70, 50);
	litest_touch_down(dev, 4, 90, 50);

	litest_event(dev, EV_ABS, ABS_MT_SLOT, 0);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 1);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 2);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 3);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_ABS, ABS_MT_SLOT, 4);
	litest_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	litest_event(dev, EV_KEY, BTN_TOOL_QUINTTAP, 0);
	litest_event(dev, EV_KEY, BTN_TOUCH, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_assert_empty_queue(li);
	litest_timeout_tap();
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(clickpad_1fg_tap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);

	litest_drain_events(dev->libinput);

	/* finger down, button click, finger up
	   -> only one button left event pair */
	litest_touch_down(dev, 0, 50, 50);
	litest_event(dev, EV_KEY, BTN_LEFT, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_event(dev, EV_KEY, BTN_LEFT, 0);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_timeout_tap();

	libinput_dispatch(li);

	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li, BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_is_available)
{
	struct litest_device *dev = litest_current_device();

	ck_assert_int_ge(libinput_device_config_tap_get_finger_count(dev->libinput_device), 1);
}
END_TEST

START_TEST(touchpad_tap_is_not_available)
{
	struct litest_device *dev = litest_current_device();

	ck_assert_int_eq(libinput_device_config_tap_get_finger_count(dev->libinput_device), 0);
	ck_assert_int_eq(libinput_device_config_tap_get_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_TAP_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_set_enabled(dev->libinput_device,
								LIBINPUT_CONFIG_TAP_ENABLED),
			 LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	ck_assert_int_eq(libinput_device_config_tap_set_enabled(dev->libinput_device,
								LIBINPUT_CONFIG_TAP_DISABLED),
			 LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(touchpad_tap_default_disabled)
{
	struct litest_device *dev = litest_current_device();

	/* this test is only run on specific devices */

	ck_assert_int_eq(libinput_device_config_tap_get_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_TAP_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_default_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_TAP_DISABLED);
}
END_TEST

START_TEST(touchpad_tap_default_enabled)
{
	struct litest_device *dev = litest_current_device();

	/* this test is only run on specific devices */

	ck_assert_int_eq(libinput_device_config_tap_get_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_TAP_ENABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_default_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_TAP_ENABLED);
}
END_TEST

START_TEST(touchpad_tap_invalid)
{
	struct litest_device *dev = litest_current_device();

	ck_assert_int_eq(libinput_device_config_tap_set_enabled(dev->libinput_device, 2),
			 LIBINPUT_CONFIG_STATUS_INVALID);
	ck_assert_int_eq(libinput_device_config_tap_set_enabled(dev->libinput_device, -1),
			 LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(touchpad_tap_default_map)
{
	struct litest_device *dev = litest_current_device();
	enum libinput_config_tap_button_map map;

	map = libinput_device_config_tap_get_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LRM);

	map = libinput_device_config_tap_get_default_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LRM);
}
END_TEST

START_TEST(touchpad_tap_map_unsupported)
{
	struct litest_device *dev = litest_current_device();
	enum libinput_config_tap_button_map map;
	enum libinput_config_status status;

	map = libinput_device_config_tap_get_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LRM);
	map = libinput_device_config_tap_get_default_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LRM);

	status = libinput_device_config_tap_set_button_map(dev->libinput_device,
							   LIBINPUT_CONFIG_TAP_MAP_LMR);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_tap_set_button_map(dev->libinput_device,
							   LIBINPUT_CONFIG_TAP_MAP_LRM);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
}
END_TEST

START_TEST(touchpad_tap_set_map)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_tap_button_map map;
	enum libinput_config_status status;

	map = LIBINPUT_CONFIG_TAP_MAP_LRM;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	map = libinput_device_config_tap_get_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LRM);

	map = LIBINPUT_CONFIG_TAP_MAP_LMR;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
	map = libinput_device_config_tap_get_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LMR);

	map = LIBINPUT_CONFIG_TAP_MAP_LRM - 1;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);

	map = LIBINPUT_CONFIG_TAP_MAP_LMR + 1;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(touchpad_tap_set_map_no_tapping)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_tap_button_map map;
	enum libinput_config_status status;

	map = LIBINPUT_CONFIG_TAP_MAP_LRM;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);

	map = LIBINPUT_CONFIG_TAP_MAP_LMR;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);

	map = LIBINPUT_CONFIG_TAP_MAP_LRM - 1;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);

	map = LIBINPUT_CONFIG_TAP_MAP_LMR + 1;
	status = libinput_device_config_tap_set_button_map(device, map);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(touchpad_tap_get_map_no_tapping)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_tap_button_map map;

	map = libinput_device_config_tap_get_button_map(device);
	ck_assert_int_eq(map,  LIBINPUT_CONFIG_TAP_MAP_LRM);

	map = libinput_device_config_tap_get_default_button_map(device);
	ck_assert_int_eq(map,  LIBINPUT_CONFIG_TAP_MAP_LRM);
}
END_TEST

START_TEST(touchpad_tap_map_delayed)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	enum libinput_config_tap_button_map map;

	litest_enable_tap(dev->libinput_device);
	litest_set_tap_map(dev->libinput_device,
			   LIBINPUT_CONFIG_TAP_MAP_LRM);
	litest_drain_events(dev->libinput);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 70, 70);
	libinput_dispatch(li);

	litest_set_tap_map(dev->libinput_device,
			   LIBINPUT_CONFIG_TAP_MAP_LMR);
	map = libinput_device_config_tap_get_button_map(dev->libinput_device);
	ck_assert_int_eq(map, LIBINPUT_CONFIG_TAP_MAP_LMR);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);

	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li, BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_drag_default_disabled)
{
	struct litest_device *dev = litest_current_device();

	/* this test is only run on specific devices */

	ck_assert_int_eq(libinput_device_config_tap_get_default_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_DISABLED);
}
END_TEST

START_TEST(touchpad_drag_default_enabled)
{
	struct litest_device *dev = litest_current_device();

	/* this test is only run on specific devices */

	ck_assert_int_eq(libinput_device_config_tap_get_default_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_ENABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_ENABLED);
}
END_TEST

START_TEST(touchpad_drag_config_invalid)
{
	struct litest_device *dev = litest_current_device();

	ck_assert_int_eq(libinput_device_config_tap_set_drag_enabled(dev->libinput_device, 2),
			 LIBINPUT_CONFIG_STATUS_INVALID);
	ck_assert_int_eq(libinput_device_config_tap_set_drag_enabled(dev->libinput_device, -1),
			 LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(touchpad_drag_config_unsupported)
{
	struct litest_device *dev = litest_current_device();
	enum libinput_config_status status;

	ck_assert_int_eq(libinput_device_config_tap_get_default_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_drag_enabled(dev->libinput_device),
			 LIBINPUT_CONFIG_DRAG_DISABLED);
	status = libinput_device_config_tap_set_drag_enabled(dev->libinput_device,
							     LIBINPUT_CONFIG_DRAG_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);
	status = libinput_device_config_tap_set_drag_enabled(dev->libinput_device,
							     LIBINPUT_CONFIG_DRAG_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);
}
END_TEST

START_TEST(touchpad_drag_config_enabledisable)
{
	struct litest_device *dev = litest_current_device();
	enum libinput_config_drag_state state;

	litest_enable_tap(dev->libinput_device);

	litest_disable_tap_drag(dev->libinput_device);
	state = libinput_device_config_tap_get_drag_enabled(dev->libinput_device);
	ck_assert_int_eq(state, LIBINPUT_CONFIG_DRAG_DISABLED);

	litest_enable_tap_drag(dev->libinput_device);
	state = libinput_device_config_tap_get_drag_enabled(dev->libinput_device);
	ck_assert_int_eq(state, LIBINPUT_CONFIG_DRAG_ENABLED);

	/* same thing with tapping disabled */
	litest_enable_tap(dev->libinput_device);

	litest_disable_tap_drag(dev->libinput_device);
	state = libinput_device_config_tap_get_drag_enabled(dev->libinput_device);
	ck_assert_int_eq(state, LIBINPUT_CONFIG_DRAG_DISABLED);

	litest_enable_tap_drag(dev->libinput_device);
	state = libinput_device_config_tap_get_drag_enabled(dev->libinput_device);
	ck_assert_int_eq(state, LIBINPUT_CONFIG_DRAG_ENABLED);
}
END_TEST

START_TEST(touchpad_drag_disabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	litest_enable_tap(dev->libinput_device);
	litest_disable_tap_drag(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 90, 90, 10);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);

}
END_TEST

START_TEST(touchpad_drag_disabled_immediate)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *ev;
	struct libinput_event_pointer *ptrev;
	uint64_t press_time, release_time;

	litest_enable_tap(dev->libinput_device);
	litest_disable_tap_drag(dev->libinput_device);

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	msleep(10); /* to force a time difference */
	libinput_dispatch(li);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_PRESSED);
	press_time = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(ev);

	ev = libinput_get_event(li);
	ptrev = litest_is_button_event(ev,
				       BTN_LEFT,
				       LIBINPUT_BUTTON_STATE_RELEASED);
	release_time = libinput_event_pointer_get_time(ptrev);
	libinput_event_destroy(ev);

	ck_assert_int_gt(release_time, press_time);
}
END_TEST

START_TEST(touchpad_drag_disabled_multitap_no_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_pointer *ptrev;
	uint32_t oldtime = 0,
		 curtime;
	int range = _i, /* looped test */
	    ntaps;

	litest_enable_tap(dev->libinput_device);
	litest_disable_tap_drag(dev->libinput_device);

	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	libinput_dispatch(li);
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 70, 50, 10);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_PRESSED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_gt(curtime, oldtime);

		event = libinput_get_event(li);
		ptrev = litest_is_button_event(event,
					       BTN_LEFT,
					       LIBINPUT_BUTTON_STATE_RELEASED);
		curtime = libinput_event_pointer_get_time(ptrev);
		libinput_event_destroy(event);
		ck_assert_int_ge(curtime, oldtime);
		oldtime = curtime;
	}

	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_drag_lock_default_disabled)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;

	ck_assert_int_eq(libinput_device_config_tap_get_drag_lock_enabled(device),
			 LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_default_drag_lock_enabled(device),
			 LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  3);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

START_TEST(touchpad_drag_lock_default_unavailable)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;
	enum libinput_config_status status;

	ck_assert_int_eq(libinput_device_config_tap_get_drag_lock_enabled(device),
			 LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
	ck_assert_int_eq(libinput_device_config_tap_get_default_drag_lock_enabled(device),
			 LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_ENABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_UNSUPPORTED);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_SUCCESS);

	status = libinput_device_config_tap_set_drag_lock_enabled(device,
								  3);
	ck_assert_int_eq(status, LIBINPUT_CONFIG_STATUS_INVALID);
}
END_TEST

static inline bool
touchpad_has_palm_pressure(struct litest_device *dev)
{
	struct libevdev *evdev = dev->evdev;

	if (libevdev_has_event_code(evdev, EV_ABS, ABS_MT_PRESSURE))
		return true;

	return false;
}

START_TEST(touchpad_tap_palm_on_idle)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* Finger down is immediately palm */

	litest_touch_down_extended(dev, 0, 50, 50, axes);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* Finger down is palm after touch begin */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_hold_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* Finger down is palm after tap timeout */

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_hold_move)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* Finger down is palm after tap move threshold */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to(dev, 0, 50, 50, 60, 60, 10);
	litest_drain_events(li);

	litest_touch_move_to_extended(dev, 0, 60, 60, 60, 60, axes, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_tapped)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* tap + palm down */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_tapped_palm_down)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* tap + palm down */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down_extended(dev, 0, 50, 50, axes);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);

	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_tapped_2fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* tap + palm down + tap with second finger */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);

	libinput_dispatch(li);

	litest_touch_down(dev, 1, 50, 50);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);

	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_drag)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* tap + finger down (->drag), finger turns into palm */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_drag_2fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 2,
	    other = (which + 1) % 2;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* tap + finger down, 2nd finger down, finger turns to palm */

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down(dev, this, 50, 50);
	litest_touch_down(dev, other, 60, 50);
	libinput_dispatch(li);

	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);

	litest_touch_move_to(dev, other, 60, 50, 65, 50, 10);
	litest_assert_only_typed_events(li,
					LIBINPUT_EVENT_POINTER_MOTION);
	litest_touch_up(dev, other);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_touch_up(dev, this);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_2)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 2,
	    other = (which + 1) % 2;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* 2fg tap with one finger detected as palm */
	litest_touch_down(dev, 0, 50, 50);
	litest_touch_down(dev, 1, 60, 60);
	litest_drain_events(li);
	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);

	litest_touch_up(dev, this);
	litest_touch_up(dev, other);

	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_2_retouch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 2,
	    other = (which + 1) % 2;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* 2fg tap with one finger detected as palm, that finger is lifted
	 * and taps again as not-palm  */
	litest_touch_down(dev, this, 50, 50);
	litest_touch_down(dev, other, 60, 60);
	litest_drain_events(li);
	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, this);
	libinput_dispatch(li);

	litest_touch_down(dev, this, 70, 70);
	litest_touch_up(dev, this);
	litest_touch_up(dev, other);

	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_3)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 3;

	if (litest_slot_count(dev) < 3)
		return;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* 3fg tap with one finger detected as palm, that finger is lifted,
	   other two fingers lifted cause 2fg tap */
	litest_touch_down(dev, this, 50, 50);
	litest_touch_down(dev, (this + 1) % 3, 60, 50);
	litest_touch_down(dev, (this + 2) % 3, 70, 50);
	litest_drain_events(li);
	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, this);
	libinput_dispatch(li);

	litest_touch_up(dev, (this + 1) % 3);
	litest_touch_up(dev, (this + 2) % 3);

	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_RIGHT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_3_retouch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 3;

	if (litest_slot_count(dev) < 3)
		return;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* 3fg tap with one finger detected as palm, that finger is lifted,
	   then put down again as normal finger -> 3fg tap */
	litest_touch_down(dev, this, 50, 50);
	litest_touch_down(dev, (this + 1) % 3, 60, 50);
	litest_touch_down(dev, (this + 2) % 3, 70, 50);
	litest_drain_events(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, this);
	libinput_dispatch(li);

	litest_touch_down(dev, this, 50, 50);
	litest_touch_up(dev, this);
	litest_touch_up(dev, (this + 1) % 3);
	litest_touch_up(dev, (this + 2) % 3);

	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_MIDDLE,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_on_touch_4)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int which = _i; /* ranged test */
	int this = which % 4;

	if (litest_slot_count(dev) < 4)
		return;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	/* 3fg tap with one finger detected as palm, that finger is lifted,
	   other two fingers lifted cause 2fg tap */
	litest_touch_down(dev, this, 50, 50);
	litest_touch_down(dev, (this + 1) % 4, 60, 50);
	litest_touch_down(dev, (this + 2) % 4, 70, 50);
	litest_touch_down(dev, (this + 3) % 4, 80, 50);
	litest_drain_events(li);
	litest_touch_move_to_extended(dev, this, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, this);
	libinput_dispatch(li);

	litest_touch_up(dev, (this + 1) % 4);
	litest_touch_up(dev, (this + 2) % 4);
	litest_touch_up(dev, (this + 3) % 4);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_after_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	libinput_dispatch(li);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_timeout_tap();
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_multitap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int range = _i, /* ranged test */
	    ntaps;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_RELEASED);
	}

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_multitap_timeout)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int range = _i, /* ranged test */
	    ntaps;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_RELEASED);
	}

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_multitap_down_again)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int range = _i, /* ranged test */
	    ntaps;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);

	/* keep palm finger down */
	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 1, 50, 50);
		litest_touch_up(dev, 1);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_timeout_tap();
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= 2 * range + 1; ntaps++) {
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_RELEASED);
	}

	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_multitap_click)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};
	int range = _i, /* ranged test */
	    ntaps;

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_touch_down(dev, 0, 50, 50);
		litest_touch_up(dev, 0);
		libinput_dispatch(li);
		msleep(10);
	}

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);
	/* keep palm finger down */

	litest_button_click(dev, BTN_LEFT, true);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);

	for (ntaps = 0; ntaps <= range; ntaps++) {
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_PRESSED);
		litest_assert_button_event(li,
					   BTN_LEFT,
					   LIBINPUT_BUTTON_STATE_RELEASED);
	}

	/* the click */
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);
	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_click_then_tap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	litest_touch_down_extended(dev, 0, 50, 50, axes);
	libinput_dispatch(li);

	litest_button_click(dev, BTN_LEFT, true);
	litest_button_click(dev, BTN_LEFT, false);
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_touch_up(dev, 0);
	litest_assert_empty_queue(li);

	litest_touch_down(dev, 0, 50, 50);
	litest_touch_up(dev, 0);
	libinput_dispatch(li);
	litest_timeout_tap();
	libinput_dispatch(li);

	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_PRESSED);
	litest_assert_button_event(li,
				   BTN_LEFT,
				   LIBINPUT_BUTTON_STATE_RELEASED);

	litest_assert_empty_queue(li);
}
END_TEST

START_TEST(touchpad_tap_palm_dwt_tap)
{
	struct litest_device *dev = litest_current_device();
	struct litest_device *keyboard;
	struct libinput *li = dev->libinput;
	struct axis_replacement axes[] = {
		{ ABS_MT_PRESSURE, 75 },
		{ -1, 0 }
	};

	if (!touchpad_has_palm_pressure(dev))
		return;

	keyboard = litest_add_device(li, LITEST_KEYBOARD);

	litest_enable_tap(dev->libinput_device);
	litest_drain_events(li);

	litest_keyboard_key(keyboard, KEY_A, true);
	litest_keyboard_key(keyboard, KEY_B, true);
	litest_keyboard_key(keyboard, KEY_A, false);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 50, 50);
	libinput_dispatch(li);

	litest_keyboard_key(keyboard, KEY_B, false);
	litest_drain_events(li);
	litest_timeout_dwt_long();
	libinput_dispatch(li);

	/* Changes to palm after dwt timeout */
	litest_touch_move_to_extended(dev, 0, 50, 50, 50, 50, axes, 1);
	libinput_dispatch(li);

	litest_touch_up(dev, 0);
	libinput_dispatch(li);

	litest_assert_empty_queue(li);

	litest_delete_device(keyboard);
}
END_TEST

TEST_COLLECTION(touchpad_tap)
{
	struct range any_tap_range = {1, 4};
	struct range multitap_range = {3, 5};
	struct range tap_map_range = { LIBINPUT_CONFIG_TAP_MAP_LRM,
				       LIBINPUT_CONFIG_TAP_MAP_LMR + 1 };
	struct range range_2fg = {0, 2};
	struct range range_3fg = {0, 3};
	struct range range_4fg = {0, 4};
	struct range range_multifinger = {2, 5};

	litest_add("tap:1fg", touchpad_1fg_tap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:1fg", touchpad_1fg_doubletap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add_ranged("tap:1fg", touchpad_1fg_tap_drag_high_delay, LITEST_TOUCHPAD, LITEST_ANY, &any_tap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_timeout, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_timeout, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_tap, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_move, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_2fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &multitap_range);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_click, LITEST_CLICKPAD, LITEST_ANY, &multitap_range);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag_draglock, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag_draglock_tap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag_draglock_timeout, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:2fg", touchpad_2fg_tap_n_drag, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:2fg", touchpad_2fg_tap_n_drag_3fg_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_APPLE_CLICKPAD);
	litest_add("tap:2fg", touchpad_2fg_tap_n_drag_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add_ranged("tap:2fg", touchpad_2fg_tap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT, &tap_map_range);
	litest_add_ranged("tap:2fg", touchpad_2fg_tap_inverted, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &tap_map_range);
	litest_add("tap:2fg", touchpad_2fg_tap_move_on_release, LITEST_TOUCHPAD|LITEST_SEMI_MT, LITEST_SINGLE_TOUCH);
	litest_add("tap:2fg", touchpad_2fg_tap_n_hold_first, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:2fg", touchpad_2fg_tap_n_hold_second, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:2fg", touchpad_2fg_tap_quickrelease, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:2fg", touchpad_1fg_tap_click, LITEST_TOUCHPAD|LITEST_BUTTON, LITEST_CLICKPAD);
	litest_add("tap:2fg", touchpad_2fg_tap_click, LITEST_TOUCHPAD|LITEST_BUTTON, LITEST_SINGLE_TOUCH|LITEST_CLICKPAD);

	litest_add("tap:2fg", touchpad_2fg_tap_click_apple, LITEST_APPLE_CLICKPAD, LITEST_ANY);
	litest_add("tap:2fg", touchpad_no_2fg_tap_after_move, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:2fg", touchpad_no_2fg_tap_after_timeout, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:2fg", touchpad_no_first_fg_tap_after_move, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add_ranged("tap:3fg", touchpad_3fg_tap_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &tap_map_range);
	litest_add_ranged("tap:3fg", touchpad_3fg_tap_btntool_inverted, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &tap_map_range);
	litest_add_ranged("tap:3fg", touchpad_3fg_tap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &tap_map_range);
	litest_add("tap:3fg", touchpad_3fg_tap_tap_again, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:3fg", touchpad_3fg_tap_quickrelease, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:3fg", touchpad_3fg_tap_hover_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:3fg", touchpad_3fg_tap_pressure_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add_for_device("tap:3fg", touchpad_3fg_tap_btntool_pointerjump, LITEST_SYNAPTICS_TOPBUTTONPAD);
	litest_add_for_device("tap:3fg", touchpad_3fg_tap_slot_release_btntool, LITEST_SYNAPTICS_TOPBUTTONPAD);
	litest_add("tap:3fg", touchpad_3fg_tap_after_scroll, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);

	litest_add("tap:4fg", touchpad_4fg_tap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:4fg", touchpad_4fg_tap_quickrelease, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:5fg", touchpad_5fg_tap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);
	litest_add("tap:5fg", touchpad_5fg_tap_quickrelease, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);

	litest_add_ranged("tap:multifinger", touchpad_move_after_touch, LITEST_TOUCHPAD, LITEST_ANY, &range_multifinger);

	/* Real buttons don't interfere with tapping, so don't run those for
	   pads with buttons */
	litest_add("tap:1fg", touchpad_1fg_double_tap_click, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag_click, LITEST_CLICKPAD, LITEST_ANY);
	litest_add_ranged("tap:1fg", touchpad_1fg_multitap_n_drag_tap_click, LITEST_CLICKPAD, LITEST_ANY, &multitap_range);
	litest_add("tap:1fg", touchpad_1fg_tap_n_drag_draglock_tap_click, LITEST_CLICKPAD, LITEST_ANY);

	litest_add("tap:config", touchpad_tap_default_disabled, LITEST_TOUCHPAD|LITEST_BUTTON, LITEST_ANY);
	litest_add("tap:config", touchpad_tap_default_enabled, LITEST_TOUCHPAD, LITEST_BUTTON);
	litest_add("tap:config", touchpad_tap_invalid, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:config", touchpad_tap_is_available, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:config", touchpad_tap_is_not_available, LITEST_ANY, LITEST_TOUCHPAD);

	litest_add("tap:config", touchpad_tap_default_map, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:config", touchpad_tap_map_unsupported, LITEST_ANY, LITEST_TOUCHPAD);
	litest_add("tap:config", touchpad_tap_set_map, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:config", touchpad_tap_set_map_no_tapping, LITEST_ANY, LITEST_TOUCHPAD);
	litest_add("tap:config", touchpad_tap_get_map_no_tapping, LITEST_ANY, LITEST_TOUCHPAD);
	litest_add("tap:config", touchpad_tap_map_delayed, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH|LITEST_SEMI_MT);

	litest_add("tap:1fg", clickpad_1fg_tap_click, LITEST_CLICKPAD, LITEST_ANY);
	litest_add("tap:2fg", clickpad_2fg_tap_click, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH|LITEST_APPLE_CLICKPAD);

	litest_add("tap:draglock", touchpad_drag_lock_default_disabled, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:draglock", touchpad_drag_lock_default_unavailable, LITEST_ANY, LITEST_TOUCHPAD);

	litest_add("tap:drag", touchpad_drag_default_disabled, LITEST_ANY, LITEST_TOUCHPAD);
	litest_add("tap:drag", touchpad_drag_default_enabled, LITEST_TOUCHPAD, LITEST_BUTTON);
	litest_add("tap:drag", touchpad_drag_config_invalid, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:drag", touchpad_drag_config_unsupported, LITEST_ANY, LITEST_TOUCHPAD);
	litest_add("tap:drag", touchpad_drag_config_enabledisable, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:drag", touchpad_drag_disabled, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:drag", touchpad_drag_disabled_immediate, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add_ranged("tap:1fg", touchpad_drag_disabled_multitap_no_drag, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);

	litest_add("tap:palm", touchpad_tap_palm_on_idle, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_touch, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_touch_hold_timeout, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_touch_hold_move, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_tapped, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_tapped_palm_down, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_on_tapped_2fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("tap:palm", touchpad_tap_palm_on_drag, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_drag_2fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_2fg);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_touch_2, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_2fg);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_touch_2_retouch, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_2fg);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_touch_3, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_3fg);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_touch_3_retouch, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_3fg);
	litest_add_ranged("tap:palm", touchpad_tap_palm_on_touch_4, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &range_4fg);
	litest_add("tap:palm", touchpad_tap_palm_after_tap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add_ranged("tap:palm", touchpad_tap_palm_multitap, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:palm", touchpad_tap_palm_multitap_timeout, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add_ranged("tap:palm", touchpad_tap_palm_multitap_down_again, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &multitap_range);
	litest_add_ranged("tap:palm", touchpad_tap_palm_multitap_click, LITEST_TOUCHPAD, LITEST_ANY, &multitap_range);
	litest_add("tap:palm", touchpad_tap_palm_click_then_tap, LITEST_TOUCHPAD, LITEST_ANY);
	litest_add("tap:palm", touchpad_tap_palm_dwt_tap, LITEST_TOUCHPAD, LITEST_ANY);
}
