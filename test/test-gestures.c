/*
 * Copyright © 2015 Red Hat, Inc.
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
#include <libinput.h>

#include "libinput-util.h"
#include "litest.h"

enum cardinal {
	N, NE, E, SE, S, SW, W, NW, NCARDINALS
};

START_TEST(gestures_cap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	if (libevdev_has_property(dev->evdev, INPUT_PROP_SEMI_MT))
		ck_assert(!libinput_device_has_capability(device,
					  LIBINPUT_DEVICE_CAP_GESTURE));
	else
		ck_assert(libinput_device_has_capability(device,
					 LIBINPUT_DEVICE_CAP_GESTURE));
}
END_TEST

START_TEST(gestures_nocap)
{
	struct litest_device *dev = litest_current_device();
	struct libinput_device *device = dev->libinput_device;

	ck_assert(!libinput_device_has_capability(device,
						  LIBINPUT_DEVICE_CAP_GESTURE));
}
END_TEST

START_TEST(gestures_swipe_3fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 3)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	libinput_dispatch(li);
	litest_touch_move_three_touches(dev, 40, 40, 50, 40, 60, 40, dir_x,
					dir_y, 10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 3);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_3fg_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_two_touches(dev, 40, 40, 50, 40, dir_x, dir_y, 10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 3);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_3fg_btntool_pinch_like)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_TRIPLETAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	litest_drain_events(li);

	/* Technically a pinch position + pinch movement, but expect swipe
	 * for nfingers > nslots */
	litest_touch_down(dev, 0, 20, 60);
	litest_touch_down(dev, 1, 50, 20);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_to(dev, 0, 20, 60, 10, 80, 20);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	litest_is_gesture_event(event, LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN, 3);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		litest_is_gesture_event(event,
					LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
					3);
		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_4fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 3 },
		{ 3, 3 },
		{ 3, 0 },
		{ 3, -3 },
		{ 0, -3 },
		{ -3, -3 },
		{ -3, 0 },
		{ -3, 3 },
	};
	int i;

	if (litest_slot_count(dev) < 4)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	litest_touch_down(dev, 3, 70, 40);
	libinput_dispatch(li);

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);

		dir_x += cardinals[cardinal][0];
		dir_y += cardinals[cardinal][1];

		litest_touch_move(dev,
				  0,
				  40 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  2,
				  60 + dir_x,
				  40 + dir_y);
		litest_touch_move(dev,
				  3,
				  70 + dir_x,
				  40 + dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 4);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 3);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_swipe_4fg_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) > 2 ||
	    !libevdev_has_event_code(dev->evdev, EV_KEY, BTN_TOOL_QUADTAP) ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_QUADTAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);

	libinput_dispatch(li);
	litest_touch_move_two_touches(dev, 40, 40, 50, 40, dir_x, dir_y, 10);
	libinput_dispatch(li);

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
						 4);

		dx = libinput_event_gesture_get_dx(gevent);
		dy = libinput_event_gesture_get_dy(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		dx = libinput_event_gesture_get_dx_unaccelerated(gevent);
		dy = libinput_event_gesture_get_dy_unaccelerated(gevent);
		if (dir_x == 0.0)
			ck_assert(dx == 0.0);
		else if (dir_x < 0.0)
			ck_assert(dx < 0.0);
		else if (dir_x > 0.0)
			ck_assert(dx > 0.0);

		if (dir_y == 0.0)
			ck_assert(dy == 0.0);
		else if (dir_y < 0.0)
			ck_assert(dy < 0.0);
		else if (dir_y > 0.0)
			ck_assert(dy > 0.0);

		libinput_event_destroy(event);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_pinch)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 2 ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	/* If the device is too small to provide a finger spread wide enough
	 * to avoid the scroll bias, skip the test */
	if (cardinal == E || cardinal == W) {
		double w = 0, h = 0;
		libinput_device_get_size(dev->libinput_device, &w, &h);
		/* 0.6 because the code below gives us points like 20/y and
		 * 80/y. 45 because the threshold in the code is 40mm */
		if (w * 0.6 < 45)
			return;
	}

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	libinput_dispatch(li);

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 2);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 2);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 2);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_pinch_3fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 3)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	litest_touch_down(dev, 2, 51 - dir_x, 51 - dir_y);
	libinput_dispatch(li);

	for (i = 0; i < 8; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_touch_move(dev,
				  2,
				  51 - dir_x,
				  51 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 3);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 3);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 3);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_pinch_4fg)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 4)
		return;

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	litest_touch_down(dev, 2, 51 - dir_x, 51 - dir_y);
	litest_touch_down(dev, 3, 52 - dir_x, 52 - dir_y);
	libinput_dispatch(li);

	for (i = 0; i < 7; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x -= 2;
		else if (dir_x < 0.0)
			dir_x += 2;
		if (dir_y > 0.0)
			dir_y -= 2;
		else if (dir_y < 0.0)
			dir_y += 2;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_touch_move(dev,
				  2,
				  51 - dir_x,
				  51 - dir_y);
		litest_touch_move(dev,
				  3,
				  52 - dir_x,
				  52 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 4);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 4);

		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);

		ck_assert(scale < oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	litest_touch_up(dev, 2);
	litest_touch_up(dev, 3);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 4);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_spread)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	double dx, dy;
	int cardinal = _i; /* ranged test */
	double dir_x, dir_y;
	int i;
	double scale, oldscale;
	double angle;
	int cardinals[NCARDINALS][2] = {
		{ 0, 30 },
		{ 30, 30 },
		{ 30, 0 },
		{ 30, -30 },
		{ 0, -30 },
		{ -30, -30 },
		{ -30, 0 },
		{ -30, 30 },
	};

	if (litest_slot_count(dev) < 2 ||
	    !libinput_device_has_capability(dev->libinput_device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return;

	/* If the device is too small to provide a finger spread wide enough
	 * to avoid the scroll bias, skip the test */
	if (cardinal == E || cardinal == W) {
		double w = 0, h = 0;
		libinput_device_get_size(dev->libinput_device, &w, &h);
		/* 0.6 because the code below gives us points like 20/y and
		 * 80/y. 45 because the threshold in the code is 40mm */
		if (w * 0.6 < 45)
			return;
	}

	dir_x = cardinals[cardinal][0];
	dir_y = cardinals[cardinal][1];

	litest_drain_events(li);

	litest_touch_down(dev, 0, 50 + dir_x, 50 + dir_y);
	litest_touch_down(dev, 1, 50 - dir_x, 50 - dir_y);
	libinput_dispatch(li);

	for (i = 0; i < 15; i++) {
		litest_push_event_frame(dev);
		if (dir_x > 0.0)
			dir_x += 1;
		else if (dir_x < 0.0)
			dir_x -= 1;
		if (dir_y > 0.0)
			dir_y += 1;
		else if (dir_y < 0.0)
			dir_y -= 1;
		litest_touch_move(dev,
				  0,
				  50 + dir_x,
				  50 + dir_y);
		litest_touch_move(dev,
				  1,
				  50 - dir_x,
				  50 - dir_y);
		litest_pop_event_frame(dev);
		libinput_dispatch(li);
	}

	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
					 2);
	dx = libinput_event_gesture_get_dx(gevent);
	dy = libinput_event_gesture_get_dy(gevent);
	scale = libinput_event_gesture_get_scale(gevent);
	ck_assert(dx == 0.0);
	ck_assert(dy == 0.0);
	ck_assert(scale == 1.0);

	libinput_event_destroy(event);

	while ((event = libinput_get_event(li)) != NULL) {
		gevent = litest_is_gesture_event(event,
						 LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
						 2);
		oldscale = scale;
		scale = libinput_event_gesture_get_scale(gevent);
		ck_assert(scale > oldscale);

		angle = libinput_event_gesture_get_angle_delta(gevent);
		ck_assert_double_le(fabs(angle), 1.0);

		libinput_event_destroy(event);
		libinput_dispatch(li);
	}

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_PINCH_END,
					 2);
	ck_assert(!libinput_event_gesture_get_cancelled(gevent));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_time_usec)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;
	struct libinput_event *event;
	struct libinput_event_gesture *gevent;
	uint64_t time_usec;

	if (litest_slot_count(dev) < 3)
		return;

	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 40);
	litest_touch_down(dev, 1, 50, 40);
	litest_touch_down(dev, 2, 60, 40);
	libinput_dispatch(li);
	litest_touch_move_three_touches(dev, 40, 40, 50, 40, 60, 40, 0, 30,
					30);

	libinput_dispatch(li);
	event = libinput_get_event(li);
	gevent = litest_is_gesture_event(event,
					 LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
					 3);
	time_usec = libinput_event_gesture_get_time_usec(gevent);
	ck_assert_int_eq(libinput_event_gesture_get_time(gevent),
			 (uint32_t) (time_usec / 1000));
	libinput_event_destroy(event);
}
END_TEST

START_TEST(gestures_3fg_buttonarea_scroll)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) < 3)
		return;

	litest_enable_buttonareas(dev);
	litest_enable_2fg_scroll(dev);
	litest_drain_events(li);

	litest_touch_down(dev, 0, 40, 20);
	litest_touch_down(dev, 1, 30, 20);
	/* third finger in btnarea */
	litest_touch_down(dev, 2, 50, 99);
	libinput_dispatch(li);
	litest_touch_move_two_touches(dev, 40, 20, 30, 20, 0, 40, 10);

	litest_touch_up(dev, 0);
	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	litest_assert_scroll(li, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL, 4);
}
END_TEST

START_TEST(gestures_3fg_buttonarea_scroll_btntool)
{
	struct litest_device *dev = litest_current_device();
	struct libinput *li = dev->libinput;

	if (litest_slot_count(dev) > 2)
		return;

	litest_enable_buttonareas(dev);
	litest_enable_2fg_scroll(dev);
	litest_drain_events(li);

	/* first finger in btnarea */
	litest_touch_down(dev, 0, 20, 99);
	litest_touch_down(dev, 1, 30, 20);
	litest_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	litest_event(dev, EV_KEY, BTN_TOOL_TRIPLETAP, 1);
	litest_event(dev, EV_SYN, SYN_REPORT, 0);
	libinput_dispatch(li);
	litest_touch_move_to(dev, 1, 30, 20, 30, 70, 10);

	litest_touch_up(dev, 1);
	libinput_dispatch(li);
	litest_assert_scroll(li, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL, 4);
}
END_TEST

TEST_COLLECTION(gestures)
{
	struct range cardinals = { N, N + NCARDINALS };

	litest_add("gestures:cap", gestures_cap, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add("gestures:cap", gestures_nocap, LITEST_ANY, LITEST_TOUCHPAD);

	litest_add_ranged("gestures:swipe", gestures_swipe_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:swipe", gestures_swipe_3fg_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add("gestures:swipe", gestures_swipe_3fg_btntool_pinch_like, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
	litest_add_ranged("gestures:swipe", gestures_swipe_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:swipe", gestures_swipe_4fg_btntool, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:pinch", gestures_pinch, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:pinch", gestures_pinch_3fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:pinch", gestures_pinch_4fg, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);
	litest_add_ranged("gestures:pinch", gestures_spread, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH, &cardinals);

	litest_add("gestures:swipe", gestures_3fg_buttonarea_scroll, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);
	litest_add("gestures:swipe", gestures_3fg_buttonarea_scroll_btntool, LITEST_CLICKPAD, LITEST_SINGLE_TOUCH);

	litest_add("gestures:time", gestures_time_usec, LITEST_TOUCHPAD, LITEST_SINGLE_TOUCH);
}
