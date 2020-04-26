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

#include "config.h"

#include <math.h>
#include <stdbool.h>

#include "evdev-mt-touchpad.h"

#define DEFAULT_GESTURE_SWITCH_TIMEOUT ms2us(100)
#define DEFAULT_GESTURE_SWIPE_TIMEOUT ms2us(150)
#define DEFAULT_GESTURE_PINCH_TIMEOUT ms2us(150)

static inline const char*
gesture_state_to_str(enum tp_gesture_state state)
{
	switch (state) {
	CASE_RETURN_STRING(GESTURE_STATE_NONE);
	CASE_RETURN_STRING(GESTURE_STATE_UNKNOWN);
	CASE_RETURN_STRING(GESTURE_STATE_SCROLL);
	CASE_RETURN_STRING(GESTURE_STATE_PINCH);
	CASE_RETURN_STRING(GESTURE_STATE_SWIPE);
	}
	return NULL;
}

static struct device_float_coords
tp_get_touches_delta(struct tp_dispatch *tp, bool average)
{
	struct tp_touch *t;
	unsigned int i, nactive = 0;
	struct device_float_coords delta = {0.0, 0.0};

	for (i = 0; i < tp->num_slots; i++) {
		t = &tp->touches[i];

		if (!tp_touch_active_for_gesture(tp, t))
			continue;

		nactive++;

		if (t->dirty) {
			struct device_coords d;

			d = tp_get_delta(t);

			delta.x += d.x;
			delta.y += d.y;
		}
	}

	if (!average || nactive == 0)
		return delta;

	delta.x /= nactive;
	delta.y /= nactive;

	return delta;
}

static void
tp_gesture_init_scroll(struct tp_dispatch *tp)
{
	struct phys_coords zero = {0.0, 0.0};
	tp->scroll.active.h = false;
	tp->scroll.active.v = false;
	tp->scroll.duration.h = 0;
	tp->scroll.duration.v = 0;
	tp->scroll.vector = zero;
	tp->scroll.time_prev = 0;
}

static inline struct device_float_coords
tp_get_combined_touches_delta(struct tp_dispatch *tp)
{
	return tp_get_touches_delta(tp, false);
}

static inline struct device_float_coords
tp_get_average_touches_delta(struct tp_dispatch *tp)
{
	return tp_get_touches_delta(tp, true);
}

static void
tp_gesture_start(struct tp_dispatch *tp, uint64_t time)
{
	const struct normalized_coords zero = { 0.0, 0.0 };

	if (tp->gesture.started)
		return;

	switch (tp->gesture.state) {
	case GESTURE_STATE_NONE:
	case GESTURE_STATE_UNKNOWN:
		evdev_log_bug_libinput(tp->device,
				       "%s in unknown gesture mode\n",
				       __func__);
		break;
	case GESTURE_STATE_SCROLL:
		tp_gesture_init_scroll(tp);
		break;
	case GESTURE_STATE_PINCH:
		gesture_notify_pinch(&tp->device->base, time,
				    LIBINPUT_EVENT_GESTURE_PINCH_BEGIN,
				    tp->gesture.finger_count,
				    &zero, &zero, 1.0, 0.0);
		break;
	case GESTURE_STATE_SWIPE:
		gesture_notify_swipe(&tp->device->base, time,
				     LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN,
				     tp->gesture.finger_count,
				     &zero, &zero);
		break;
	}

	tp->gesture.started = true;
}

static void
tp_gesture_post_pointer_motion(struct tp_dispatch *tp, uint64_t time)
{
	struct device_float_coords raw;
	struct normalized_coords delta;

	/* When a clickpad is clicked, combine motion of all active touches */
	if (tp->buttons.is_clickpad && tp->buttons.state)
		raw = tp_get_combined_touches_delta(tp);
	else
		raw = tp_get_average_touches_delta(tp);

	delta = tp_filter_motion(tp, &raw, time);

	if (!normalized_is_zero(delta) || !device_float_is_zero(raw)) {
		struct device_float_coords unaccel;

		unaccel = tp_scale_to_xaxis(tp, raw);
		pointer_notify_motion(&tp->device->base,
				      time,
				      &delta,
				      &unaccel);
	}
}

static unsigned int
tp_gesture_get_active_touches(const struct tp_dispatch *tp,
			      struct tp_touch **touches,
			      unsigned int count)
{
	unsigned int n = 0;
	struct tp_touch *t;

	memset(touches, 0, count * sizeof(struct tp_touch *));

	tp_for_each_touch(tp, t) {
		if (tp_touch_active_for_gesture(tp, t)) {
			touches[n++] = t;
			if (n == count)
				return count;
		}
	}

	/*
	 * This can happen when the user does .e.g:
	 * 1) Put down 1st finger in center (so active)
	 * 2) Put down 2nd finger in a button area (so inactive)
	 * 3) Put down 3th finger somewhere, gets reported as a fake finger,
	 *    so gets same coordinates as 1st -> active
	 *
	 * We could avoid this by looking at all touches, be we really only
	 * want to look at real touches.
	 */
	return n;
}

static uint32_t
tp_gesture_get_direction(struct tp_dispatch *tp, struct tp_touch *touch)
{
	struct phys_coords mm;
	struct device_float_coords delta;

	delta = device_delta(touch->point, touch->gesture.initial);
	mm = tp_phys_delta(tp, delta);

	return phys_get_direction(mm);
}

static void
tp_gesture_get_pinch_info(struct tp_dispatch *tp,
			  double *distance,
			  double *angle,
			  struct device_float_coords *center)
{
	struct normalized_coords normalized;
	struct device_float_coords delta;
	struct tp_touch *first = tp->gesture.touches[0],
			*second = tp->gesture.touches[1];

	delta = device_delta(first->point, second->point);
	normalized = tp_normalize_delta(tp, delta);
	*distance = normalized_length(normalized);
	*angle = atan2(normalized.y, normalized.x) * 180.0 / M_PI;

	*center = device_average(first->point, second->point);
}

static void
tp_gesture_set_scroll_buildup(struct tp_dispatch *tp)
{
	struct device_float_coords d0, d1;
	struct device_float_coords average;
	struct tp_touch *first = tp->gesture.touches[0],
			*second = tp->gesture.touches[1];

	d0 = device_delta(first->point, first->gesture.initial);
	d1 = device_delta(second->point, second->gesture.initial);

	average = device_float_average(d0, d1);
	tp->device->scroll.buildup = tp_normalize_delta(tp, average);
}

static void
tp_gesture_apply_scroll_constraints(struct tp_dispatch *tp,
				  struct device_float_coords *raw,
				  struct normalized_coords *delta,
				  uint64_t time)
{
	uint64_t tdelta = 0;
	struct phys_coords delta_mm, vector;
	double vector_decay, vector_length, slope;

	const uint64_t ACTIVE_THRESHOLD = ms2us(100),
		       INACTIVE_THRESHOLD = ms2us(50),
		       EVENT_TIMEOUT = ms2us(100);

	/* Both axes active == true means free scrolling is enabled */
	if (tp->scroll.active.h && tp->scroll.active.v)
		return;

	/* Determine time delta since last movement event */
	if (tp->scroll.time_prev != 0)
		tdelta = time - tp->scroll.time_prev;
	if (tdelta > EVENT_TIMEOUT)
		tdelta = 0;
	tp->scroll.time_prev = time;

	/* Delta since last movement event in mm */
	delta_mm = tp_phys_delta(tp, *raw);

	/* Old vector data "fades" over time. This is a two-part linear
	 * approximation of an exponential function - for example, for
	 * EVENT_TIMEOUT of 100, vector_decay = (0.97)^tdelta. This linear
	 * approximation allows easier tweaking of EVENT_TIMEOUT and is faster.
	 */
	if (tdelta > 0) {
		double recent, later;
		recent = ((EVENT_TIMEOUT / 2.0) - tdelta) /
			 (EVENT_TIMEOUT / 2.0);
		later = (EVENT_TIMEOUT - tdelta) /
			(EVENT_TIMEOUT * 2.0);
		vector_decay = tdelta <= (0.33 * EVENT_TIMEOUT) ?
			       recent : later;
	} else {
		vector_decay = 0.0;
	}

	/* Calculate windowed vector from delta + weighted historic data */
	vector.x = (tp->scroll.vector.x * vector_decay) + delta_mm.x;
	vector.y = (tp->scroll.vector.y * vector_decay) + delta_mm.y;
	vector_length = hypot(vector.x, vector.y);
	tp->scroll.vector = vector;

	/* We care somewhat about distance and speed, but more about
	 * consistency of direction over time. Keep track of the time spent
	 * primarily along each axis. If one axis is active, time spent NOT
	 * moving much in the other axis is subtracted, allowing a switch of
	 * axes in a single scroll + ability to "break out" and go diagonal.
	 *
	 * Slope to degree conversions (infinity = 90°, 0 = 0°):
	 */
	const double DEGREE_75 = 3.73;
	const double DEGREE_60 = 1.73;
	const double DEGREE_30 = 0.57;
	const double DEGREE_15 = 0.27;
	slope = (vector.x != 0) ? fabs(vector.y / vector.x) : INFINITY;

	/* Ensure vector is big enough (in mm per EVENT_TIMEOUT) to be confident
	 * of direction. Larger = harder to enable diagonal/free scrolling.
	 */
	const double MIN_VECTOR = 0.15;

	if (slope >= DEGREE_30 && vector_length > MIN_VECTOR) {
		tp->scroll.duration.v += tdelta;
		if (tp->scroll.duration.v > ACTIVE_THRESHOLD)
			tp->scroll.duration.v = ACTIVE_THRESHOLD;
		if (slope >= DEGREE_75) {
			if (tp->scroll.duration.h > tdelta)
				tp->scroll.duration.h -= tdelta;
			else
				tp->scroll.duration.h = 0;
		}
	}
	if (slope < DEGREE_60  && vector_length > MIN_VECTOR) {
		tp->scroll.duration.h += tdelta;
		if (tp->scroll.duration.h > ACTIVE_THRESHOLD)
			tp->scroll.duration.h = ACTIVE_THRESHOLD;
		if (slope < DEGREE_15) {
			if (tp->scroll.duration.v > tdelta)
				tp->scroll.duration.v -= tdelta;
			else
				tp->scroll.duration.v = 0;
		}
	}

	if (tp->scroll.duration.h == ACTIVE_THRESHOLD) {
		tp->scroll.active.h = true;
		if (tp->scroll.duration.v < INACTIVE_THRESHOLD)
			tp->scroll.active.v = false;
	}
	if (tp->scroll.duration.v == ACTIVE_THRESHOLD) {
		tp->scroll.active.v = true;
		if (tp->scroll.duration.h < INACTIVE_THRESHOLD)
			tp->scroll.active.h = false;
	}

	/* If vector is big enough in a diagonal direction, always unlock
	 * both axes regardless of thresholds
	 */
	if (vector_length > 5.0 && slope < 1.73 && slope >= 0.57) {
		tp->scroll.active.v = true;
		tp->scroll.active.h = true;
	}

	/* If only one axis is active, constrain motion accordingly. If both
	 * are set, we've detected deliberate diagonal movement; enable free
	 * scrolling for the life of the gesture.
	 */
	if (!tp->scroll.active.h && tp->scroll.active.v)
		delta->x = 0.0;
	if (tp->scroll.active.h && !tp->scroll.active.v)
		delta->y = 0.0;

	/* If we haven't determined an axis, use the slope in the meantime */
	if (!tp->scroll.active.h && !tp->scroll.active.v) {
		delta->x = (slope >= DEGREE_60) ? 0.0 : delta->x;
		delta->y = (slope < DEGREE_30) ? 0.0 : delta->y;
	}
}

static enum tp_gesture_state
tp_gesture_handle_state_none(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *first, *second;
	struct tp_touch *touches[4];
	unsigned int ntouches;
	unsigned int i;

	ntouches = tp_gesture_get_active_touches(tp, touches, 4);
	if (ntouches < 2)
		return GESTURE_STATE_NONE;

	if (!tp->gesture.enabled) {
		if (ntouches == 2)
			return GESTURE_STATE_SCROLL;
		else
			return GESTURE_STATE_NONE;
	}

	first = touches[0];
	second = touches[1];

	/* For 3+ finger gestures we cheat. A human hand's finger
	 * arrangement means that for a 3 or 4 finger swipe gesture, the
	 * fingers are roughly arranged in a horizontal line.
	 * They will all move in the same direction, so we can simply look
	 * at the left and right-most ones only. If we have fake touches, we
	 * just take the left/right-most real touch position, since the fake
	 * touch has the same location as one of those.
	 *
	 * For a 3 or 4 finger pinch gesture, 2 or 3 fingers are roughly in
	 * a horizontal line, with the thumb below and left (right-handed
	 * users) or right (left-handed users). Again, the row of non-thumb
	 * fingers moves identically so we can look at the left and
	 * right-most only and then treat it like a two-finger
	 * gesture.
	 */
	if (ntouches > 2) {
		second = touches[0];

		for (i = 1; i < ntouches && i < tp->num_slots; i++) {
			if (touches[i]->point.x < first->point.x)
				first = touches[i];
			else if (touches[i]->point.x > second->point.x)
				second = touches[i];
		}

		if (first == second)
			return GESTURE_STATE_NONE;

	}

	tp->gesture.initial_time = time;
	first->gesture.initial = first->point;
	second->gesture.initial = second->point;
	tp->gesture.touches[0] = first;
	tp->gesture.touches[1] = second;

	return GESTURE_STATE_UNKNOWN;
}

static inline int
tp_gesture_same_directions(int dir1, int dir2)
{
	/*
	 * In some cases (semi-mt touchpads) we may seen one finger move
	 * e.g. N/NE and the other W/NW so we not only check for overlapping
	 * directions, but also for neighboring bits being set.
	 * The ((dira & 0x80) && (dirb & 0x01)) checks are to check for bit 0
	 * and 7 being set as they also represent neighboring directions.
	 */
	return ((dir1 | (dir1 >> 1)) & dir2) ||
		((dir2 | (dir2 >> 1)) & dir1) ||
		((dir1 & 0x80) && (dir2 & 0x01)) ||
		((dir2 & 0x80) && (dir1 & 0x01));
}

static inline void
tp_gesture_init_pinch(struct tp_dispatch *tp)
{
	tp_gesture_get_pinch_info(tp,
				  &tp->gesture.initial_distance,
				  &tp->gesture.angle,
				  &tp->gesture.center);
	tp->gesture.prev_scale = 1.0;
}

static struct phys_coords
tp_gesture_mm_moved(struct tp_dispatch *tp, struct tp_touch *t)
{
	struct device_coords delta;

	delta.x = abs(t->point.x - t->gesture.initial.x);
	delta.y = abs(t->point.y - t->gesture.initial.y);

	return evdev_device_unit_delta_to_mm(tp->device, &delta);
}

static enum tp_gesture_state
tp_gesture_handle_state_unknown(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *first = tp->gesture.touches[0],
			*second = tp->gesture.touches[1],
			*thumb;
	uint32_t dir1, dir2;
	struct device_coords delta;
	struct phys_coords first_moved, second_moved, distance_mm;
	double first_mm, second_mm; /* movement since gesture start in mm */
	double thumb_mm, finger_mm;
	double min_move = 1.5; /* min movement threshold in mm - count this touch */
	double max_move = 4.0; /* max movement threshold in mm - ignore other touch */

	/* If we have more fingers than slots, we don't know where the
	 * fingers are. Default to swipe */
	if (tp->gesture.enabled && tp->gesture.finger_count > 2 &&
	    tp->gesture.finger_count > tp->num_slots)
		return GESTURE_STATE_SWIPE;

	/* Need more margin for error when there are more fingers */
	max_move += 2.0 * (tp->gesture.finger_count - 2);
	min_move += 0.5 * (tp->gesture.finger_count - 2);

	first_moved = tp_gesture_mm_moved(tp, first);
	first_mm = hypot(first_moved.x, first_moved.y);

	second_moved = tp_gesture_mm_moved(tp, second);
	second_mm = hypot(second_moved.x, second_moved.y);

	delta.x = abs(first->point.x - second->point.x);
	delta.y = abs(first->point.y - second->point.y);
	distance_mm = evdev_device_unit_delta_to_mm(tp->device, &delta);

	/* If both touches moved less than a mm, we cannot decide yet */
	if (first_mm < 1 && second_mm < 1)
		return GESTURE_STATE_UNKNOWN;

	/* Pick the thumb as the lowest point on the touchpad */
	if (first->point.y > second->point.y) {
		thumb = first;
		thumb_mm = first_mm;
		finger_mm = second_mm;
	} else {
		thumb = second;
		thumb_mm = second_mm;
		finger_mm = first_mm;
	}

	/* If both touches are within 7mm vertically and 40mm horizontally
	 * past the timeout, assume scroll/swipe */
	if ((!tp->gesture.enabled ||
	     (distance_mm.x < 40.0 && distance_mm.y < 7.0)) &&
	    time > (tp->gesture.initial_time + DEFAULT_GESTURE_SWIPE_TIMEOUT)) {
		if (tp->gesture.finger_count == 2) {
			tp_gesture_set_scroll_buildup(tp);
			return GESTURE_STATE_SCROLL;
		} else {
			return GESTURE_STATE_SWIPE;
		}
	}

	/* If one touch exceeds the max_move threshold while the other has not
	 * yet passed the min_move threshold, there is either a resting thumb,
	 * or the user is doing "one-finger-scroll," where one touch stays in
	 * place while the other moves.
	 */
	if (first_mm >= max_move || second_mm >= max_move) {
		/* If thumb detection is enabled, and thumb is still while
		 * finger moves, cancel gestures and mark lower as thumb.
		 * This applies to all gestures (2, 3, 4+ fingers), but allows
		 * more thumb motion on >2 finger gestures during detection.
		 */
		if (tp->thumb.detect_thumbs && thumb_mm < min_move) {
			tp_thumb_suppress(tp, thumb);
			return GESTURE_STATE_NONE;
		}

		/* If gestures detection is disabled, or if finger is still
		 * while thumb moves, assume this is "one-finger scrolling."
		 * This applies only to 2-finger gestures.
		 */
		if ((!tp->gesture.enabled || finger_mm < min_move) &&
		    tp->gesture.finger_count == 2) {
			tp_gesture_set_scroll_buildup(tp);
			return GESTURE_STATE_SCROLL;
		}

		/* If more than 2 fingers are involved, and the thumb moves
		 * while the fingers stay still, assume a pinch if eligible.
		 */
		if (finger_mm < min_move &&
		    tp->gesture.finger_count > 2 &&
		    tp->gesture.enabled &&
		    tp->thumb.pinch_eligible) {
			tp_gesture_init_pinch(tp);
			return GESTURE_STATE_PINCH;
		}
	}

	/* If either touch is still below the min_move threshold, we can't
	 * tell what kind of gesture this is.
	 */
	if ((first_mm < min_move) || (second_mm < min_move))
		return GESTURE_STATE_UNKNOWN;

	/* Both touches have exceeded the min_move threshold, so we have a
	 * valid gesture. Update gesture initial time and get directions so
	 * we know if it's a pinch or swipe/scroll.
	 */
	dir1 = tp_gesture_get_direction(tp, first);
	dir2 = tp_gesture_get_direction(tp, second);

	/* If we can't accurately detect pinches, or if the touches are moving
	 * the same way, this is a scroll or swipe.
	 */
	if (tp->gesture.finger_count > tp->num_slots ||
	    tp_gesture_same_directions(dir1, dir2)) {
		if (tp->gesture.finger_count == 2) {
			tp_gesture_set_scroll_buildup(tp);
			return GESTURE_STATE_SCROLL;
		} else if (tp->gesture.enabled) {
			return GESTURE_STATE_SWIPE;
		}
	}

	/* If the touches are moving away from each other, this is a pinch */
	tp_gesture_init_pinch(tp);
	return GESTURE_STATE_PINCH;
}

static enum tp_gesture_state
tp_gesture_handle_state_scroll(struct tp_dispatch *tp, uint64_t time)
{
	struct device_float_coords raw;
	struct normalized_coords delta;

	if (tp->scroll.method != LIBINPUT_CONFIG_SCROLL_2FG)
		return GESTURE_STATE_SCROLL;

	raw = tp_get_average_touches_delta(tp);

	/* scroll is not accelerated */
	delta = tp_filter_motion_unaccelerated(tp, &raw, time);

	if (normalized_is_zero(delta))
		return GESTURE_STATE_SCROLL;

	tp_gesture_start(tp, time);
	tp_gesture_apply_scroll_constraints(tp, &raw, &delta, time);
	evdev_post_scroll(tp->device,
			  time,
			  LIBINPUT_POINTER_AXIS_SOURCE_FINGER,
			  &delta);

	return GESTURE_STATE_SCROLL;
}

static enum tp_gesture_state
tp_gesture_handle_state_swipe(struct tp_dispatch *tp, uint64_t time)
{
	struct device_float_coords raw;
	struct normalized_coords delta, unaccel;

	raw = tp_get_average_touches_delta(tp);
	delta = tp_filter_motion(tp, &raw, time);

	if (!normalized_is_zero(delta) || !device_float_is_zero(raw)) {
		unaccel = tp_normalize_delta(tp, raw);
		tp_gesture_start(tp, time);
		gesture_notify_swipe(&tp->device->base, time,
				     LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE,
				     tp->gesture.finger_count,
				     &delta, &unaccel);
	}

	return GESTURE_STATE_SWIPE;
}

static enum tp_gesture_state
tp_gesture_handle_state_pinch(struct tp_dispatch *tp, uint64_t time)
{
	double angle, angle_delta, distance, scale;
	struct device_float_coords center, fdelta;
	struct normalized_coords delta, unaccel;

	tp_gesture_get_pinch_info(tp, &distance, &angle, &center);

	scale = distance / tp->gesture.initial_distance;

	angle_delta = angle - tp->gesture.angle;
	tp->gesture.angle = angle;
	if (angle_delta > 180.0)
		angle_delta -= 360.0;
	else if (angle_delta < -180.0)
		angle_delta += 360.0;

	fdelta = device_float_delta(center, tp->gesture.center);
	tp->gesture.center = center;

	delta = tp_filter_motion(tp, &fdelta, time);

	if (normalized_is_zero(delta) && device_float_is_zero(fdelta) &&
	    scale == tp->gesture.prev_scale && angle_delta == 0.0)
		return GESTURE_STATE_PINCH;

	unaccel = tp_normalize_delta(tp, fdelta);
	tp_gesture_start(tp, time);
	gesture_notify_pinch(&tp->device->base, time,
			     LIBINPUT_EVENT_GESTURE_PINCH_UPDATE,
			     tp->gesture.finger_count,
			     &delta, &unaccel, scale, angle_delta);

	tp->gesture.prev_scale = scale;

	return GESTURE_STATE_PINCH;
}

static void
tp_gesture_post_gesture(struct tp_dispatch *tp, uint64_t time)
{
	enum tp_gesture_state oldstate = tp->gesture.state;

	if (tp->gesture.state == GESTURE_STATE_NONE)
		tp->gesture.state =
			tp_gesture_handle_state_none(tp, time);

	if (tp->gesture.state == GESTURE_STATE_UNKNOWN)
		tp->gesture.state =
			tp_gesture_handle_state_unknown(tp, time);

	if (tp->gesture.state == GESTURE_STATE_SCROLL)
		tp->gesture.state =
			tp_gesture_handle_state_scroll(tp, time);

	if (tp->gesture.state == GESTURE_STATE_SWIPE)
		tp->gesture.state =
			tp_gesture_handle_state_swipe(tp, time);

	if (tp->gesture.state == GESTURE_STATE_PINCH)
		tp->gesture.state =
			tp_gesture_handle_state_pinch(tp, time);

	if (oldstate != tp->gesture.state)
		evdev_log_debug(tp->device,
				"gesture state: %s → %s\n",
				gesture_state_to_str(oldstate),
				gesture_state_to_str(tp->gesture.state));
}

void
tp_gesture_post_events(struct tp_dispatch *tp, uint64_t time)
{
	if (tp->gesture.finger_count == 0)
		return;

	/* When tap-and-dragging, force 1fg mode. On clickpads, if the
	 * physical button is down, don't allow gestures unless the button
	 * is held down by a *thumb*, specifically.
	 */
	if (tp_tap_dragging(tp) ||
	    (tp->buttons.is_clickpad && tp->buttons.state &&
	     tp->thumb.state == THUMB_STATE_FINGER)) {
		tp_gesture_cancel(tp, time);
		tp->gesture.finger_count = 1;
		tp->gesture.finger_count_pending = 0;
	}

	/* Don't send events when we're unsure in which mode we are */
	if (tp->gesture.finger_count_pending)
		return;

	switch (tp->gesture.finger_count) {
	case 1:
		if (tp->queued & TOUCHPAD_EVENT_MOTION)
			tp_gesture_post_pointer_motion(tp, time);
		break;
	case 2:
	case 3:
	case 4:
		tp_gesture_post_gesture(tp, time);
		break;
	}
}

void
tp_gesture_stop_twofinger_scroll(struct tp_dispatch *tp, uint64_t time)
{
	if (tp->scroll.method != LIBINPUT_CONFIG_SCROLL_2FG)
		return;

	evdev_stop_scroll(tp->device,
			  time,
			  LIBINPUT_POINTER_AXIS_SOURCE_FINGER);
}

static void
tp_gesture_end(struct tp_dispatch *tp, uint64_t time, bool cancelled)
{
	enum tp_gesture_state state = tp->gesture.state;

	tp->gesture.state = GESTURE_STATE_NONE;

	if (!tp->gesture.started)
		return;

	switch (state) {
	case GESTURE_STATE_NONE:
	case GESTURE_STATE_UNKNOWN:
		evdev_log_bug_libinput(tp->device,
				       "%s in unknown gesture mode\n",
				       __func__);
		break;
	case GESTURE_STATE_SCROLL:
		tp_gesture_stop_twofinger_scroll(tp, time);
		break;
	case GESTURE_STATE_PINCH:
		gesture_notify_pinch_end(&tp->device->base, time,
					 tp->gesture.finger_count,
					 tp->gesture.prev_scale,
					 cancelled);
		break;
	case GESTURE_STATE_SWIPE:
		gesture_notify_swipe_end(&tp->device->base,
					 time,
					 tp->gesture.finger_count,
					 cancelled);
		break;
	}

	tp->gesture.started = false;
}

void
tp_gesture_cancel(struct tp_dispatch *tp, uint64_t time)
{
	tp_gesture_end(tp, time, true);
}

void
tp_gesture_stop(struct tp_dispatch *tp, uint64_t time)
{
	tp_gesture_end(tp, time, false);
}

static void
tp_gesture_finger_count_switch_timeout(uint64_t now, void *data)
{
	struct tp_dispatch *tp = data;

	if (!tp->gesture.finger_count_pending)
		return;

	tp_gesture_cancel(tp, now); /* End current gesture */
	tp->gesture.finger_count = tp->gesture.finger_count_pending;
	tp->gesture.finger_count_pending = 0;
}

void
tp_gesture_handle_state(struct tp_dispatch *tp, uint64_t time)
{
	unsigned int active_touches = 0;
	struct tp_touch *t;

	tp_for_each_touch(tp, t) {
		if (tp_touch_active_for_gesture(tp, t))
			active_touches++;
	}

	if (active_touches != tp->gesture.finger_count) {
		/* If all fingers are lifted immediately end the gesture */
		if (active_touches == 0) {
			tp_gesture_stop(tp, time);
			tp->gesture.finger_count = 0;
			tp->gesture.finger_count_pending = 0;
		/* Immediately switch to new mode to avoid initial latency */
		} else if (!tp->gesture.started) {
			tp->gesture.finger_count = active_touches;
			tp->gesture.finger_count_pending = 0;
			/* If in UNKNOWN state, go back to NONE to
			 * re-evaluate leftmost and rightmost touches
			 */
			tp->gesture.state = GESTURE_STATE_NONE;
		/* Else debounce finger changes */
		} else if (active_touches != tp->gesture.finger_count_pending) {
			tp->gesture.finger_count_pending = active_touches;
			libinput_timer_set(&tp->gesture.finger_count_switch_timer,
				time + DEFAULT_GESTURE_SWITCH_TIMEOUT);
		}
	} else {
		 tp->gesture.finger_count_pending = 0;
	}
}

void
tp_init_gesture(struct tp_dispatch *tp)
{
	char timer_name[64];

	/* two-finger scrolling is always enabled, this flag just
	 * decides whether we detect pinch. semi-mt devices are too
	 * unreliable to do pinch gestures. */
	tp->gesture.enabled = !tp->semi_mt && tp->num_slots > 1;

	tp->gesture.state = GESTURE_STATE_NONE;

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s gestures",
		 evdev_device_get_sysname(tp->device));
	libinput_timer_init(&tp->gesture.finger_count_switch_timer,
			    tp_libinput_context(tp),
			    timer_name,
			    tp_gesture_finger_count_switch_timeout, tp);
}

void
tp_remove_gesture(struct tp_dispatch *tp)
{
	libinput_timer_cancel(&tp->gesture.finger_count_switch_timer);
}
