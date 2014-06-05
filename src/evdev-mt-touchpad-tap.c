/*
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "evdev-mt-touchpad.h"

#define CASE_RETURN_STRING(a) case a: return #a;

#define DEFAULT_TAP_TIMEOUT_PERIOD 180
#define DEFAULT_TAP_MOVE_THRESHOLD 30

enum tap_event {
	TAP_EVENT_TOUCH = 12,
	TAP_EVENT_MOTION,
	TAP_EVENT_RELEASE,
	TAP_EVENT_BUTTON,
	TAP_EVENT_TIMEOUT,
};

/*****************************************
 * DO NOT EDIT THIS FILE!
 *
 * Look at the state diagram in doc/touchpad-tap-state-machine.svg, or
 * online at
 * https://drive.google.com/file/d/0B1NwWmji69noYTdMcU1kTUZuUVE/edit?usp=sharing
 * (it's a http://draw.io diagram)
 *
 * Any changes in this file must be represented in the diagram.
 */

static inline const char*
tap_state_to_str(enum tp_tap_state state) {

	switch(state) {
	CASE_RETURN_STRING(TAP_STATE_IDLE);
	CASE_RETURN_STRING(TAP_STATE_HOLD);
	CASE_RETURN_STRING(TAP_STATE_TOUCH);
	CASE_RETURN_STRING(TAP_STATE_TAPPED);
	CASE_RETURN_STRING(TAP_STATE_TOUCH_2);
	CASE_RETURN_STRING(TAP_STATE_TOUCH_2_HOLD);
	CASE_RETURN_STRING(TAP_STATE_TOUCH_3);
	CASE_RETURN_STRING(TAP_STATE_TOUCH_3_HOLD);
	CASE_RETURN_STRING(TAP_STATE_DRAGGING);
	CASE_RETURN_STRING(TAP_STATE_DRAGGING_WAIT);
	CASE_RETURN_STRING(TAP_STATE_DRAGGING_OR_DOUBLETAP);
	CASE_RETURN_STRING(TAP_STATE_DRAGGING_2);
	CASE_RETURN_STRING(TAP_STATE_DEAD);
	}
	return NULL;
}

static inline const char*
tap_event_to_str(enum tap_event event) {

	switch(event) {
	CASE_RETURN_STRING(TAP_EVENT_TOUCH);
	CASE_RETURN_STRING(TAP_EVENT_MOTION);
	CASE_RETURN_STRING(TAP_EVENT_RELEASE);
	CASE_RETURN_STRING(TAP_EVENT_TIMEOUT);
	CASE_RETURN_STRING(TAP_EVENT_BUTTON);
	}
	return NULL;
}
#undef CASE_RETURN_STRING

static void
tp_tap_notify(struct tp_dispatch *tp,
	      uint64_t time,
	      int nfingers,
	      enum libinput_button_state state)
{
	int32_t button;

	switch (nfingers) {
	case 1: button = BTN_LEFT; break;
	case 2: button = BTN_RIGHT; break;
	case 3: button = BTN_MIDDLE; break;
	default:
		return;
	}

	pointer_notify_button(&tp->device->base,
			      time,
			      button,
			      state);
}

static void
tp_tap_set_timer(struct tp_dispatch *tp, uint64_t time)
{
	libinput_timer_set(&tp->tap.timer, time + DEFAULT_TAP_TIMEOUT_PERIOD);
}

static void
tp_tap_clear_timer(struct tp_dispatch *tp)
{
	libinput_timer_cancel(&tp->tap.timer);
}

static void
tp_tap_idle_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_TOUCH;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
	case TAP_EVENT_MOTION:
		log_bug_libinput("invalid event, no fingers are down\n");
		break;
	case TAP_EVENT_TIMEOUT:
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_touch_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_TOUCH_2;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_TAPPED;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_PRESSED);
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_TIMEOUT:
	case TAP_EVENT_MOTION:
		tp->tap.state = TAP_STATE_HOLD;
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_hold_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_TOUCH_2;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_IDLE;
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_tapped_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_MOTION:
	case TAP_EVENT_RELEASE:
		log_bug_libinput("invalid event when fingers are up\n");
		break;
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DRAGGING_OR_DOUBLETAP;
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_IDLE;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
tp_tap_touch2_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_TOUCH_3;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_HOLD;
		tp_tap_notify(tp, time, 2, LIBINPUT_BUTTON_STATE_PRESSED);
		tp_tap_notify(tp, time, 2, LIBINPUT_BUTTON_STATE_RELEASED);
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_MOTION:
		tp_tap_clear_timer(tp);
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_TOUCH_2_HOLD;
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_touch2_hold_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_TOUCH_3;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_HOLD;
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_TOUCH_2_HOLD;
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_touch3_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_TOUCH_3_HOLD;
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_TOUCH_2_HOLD;
		tp_tap_notify(tp, time, 3, LIBINPUT_BUTTON_STATE_PRESSED);
		tp_tap_notify(tp, time, 3, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_touch3_hold_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_TOUCH_2_HOLD;
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		break;
	}
}

static void
tp_tap_dragging_or_doubletap_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{
	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DRAGGING_2;
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_IDLE;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_PRESSED);
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_DRAGGING;
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
tp_tap_dragging_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DRAGGING_2;
		break;
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_DRAGGING_WAIT;
		tp_tap_set_timer(tp, time);
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		/* noop */
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
tp_tap_dragging_wait_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DRAGGING;
		tp_tap_clear_timer(tp);
		break;
	case TAP_EVENT_RELEASE:
	case TAP_EVENT_MOTION:
		break;
	case TAP_EVENT_TIMEOUT:
		tp->tap.state = TAP_STATE_IDLE;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
tp_tap_dragging2_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_RELEASE:
		tp->tap.state = TAP_STATE_DRAGGING;
		break;
	case TAP_EVENT_TOUCH:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
		/* noop */
		break;
	case TAP_EVENT_BUTTON:
		tp->tap.state = TAP_STATE_DEAD;
		tp_tap_notify(tp, time, 1, LIBINPUT_BUTTON_STATE_RELEASED);
		break;
	}
}

static void
tp_tap_dead_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{

	switch (event) {
	case TAP_EVENT_RELEASE:
		if (tp->nfingers_down == 0)
			tp->tap.state = TAP_STATE_IDLE;
		break;
	case TAP_EVENT_TOUCH:
	case TAP_EVENT_MOTION:
	case TAP_EVENT_TIMEOUT:
	case TAP_EVENT_BUTTON:
		break;
	}
}

static void
tp_tap_handle_event(struct tp_dispatch *tp, enum tap_event event, uint64_t time)
{
	enum tp_tap_state current;
	if (!tp->tap.enabled)
		return;

	current = tp->tap.state;

	switch(tp->tap.state) {
	case TAP_STATE_IDLE:
		tp_tap_idle_handle_event(tp, event, time);
		break;
	case TAP_STATE_TOUCH:
		tp_tap_touch_handle_event(tp, event, time);
		break;
	case TAP_STATE_HOLD:
		tp_tap_hold_handle_event(tp, event, time);
		break;
	case TAP_STATE_TAPPED:
		tp_tap_tapped_handle_event(tp, event, time);
		break;
	case TAP_STATE_TOUCH_2:
		tp_tap_touch2_handle_event(tp, event, time);
		break;
	case TAP_STATE_TOUCH_2_HOLD:
		tp_tap_touch2_hold_handle_event(tp, event, time);
		break;
	case TAP_STATE_TOUCH_3:
		tp_tap_touch3_handle_event(tp, event, time);
		break;
	case TAP_STATE_TOUCH_3_HOLD:
		tp_tap_touch3_hold_handle_event(tp, event, time);
		break;
	case TAP_STATE_DRAGGING_OR_DOUBLETAP:
		tp_tap_dragging_or_doubletap_handle_event(tp, event, time);
		break;
	case TAP_STATE_DRAGGING:
		tp_tap_dragging_handle_event(tp, event, time);
		break;
	case TAP_STATE_DRAGGING_WAIT:
		tp_tap_dragging_wait_handle_event(tp, event, time);
		break;
	case TAP_STATE_DRAGGING_2:
		tp_tap_dragging2_handle_event(tp, event, time);
		break;
	case TAP_STATE_DEAD:
		tp_tap_dead_handle_event(tp, event, time);
		break;
	}

	if (tp->tap.state == TAP_STATE_IDLE || tp->tap.state == TAP_STATE_DEAD)
		tp_tap_clear_timer(tp);

	log_debug("%s → %s → %s\n", tap_state_to_str(current), tap_event_to_str(event), tap_state_to_str(tp->tap.state));
}

static bool
tp_tap_exceeds_motion_threshold(struct tp_dispatch *tp, struct tp_touch *t)
{
	int threshold = DEFAULT_TAP_MOVE_THRESHOLD;
	double dx, dy;

	tp_get_delta(t, &dx, &dy);

	return dx * dx + dy * dy > threshold * threshold;
}

int
tp_tap_handle_state(struct tp_dispatch *tp, uint64_t time)
{
	struct tp_touch *t;
	int filter_motion = 0;

	if (tp->queued & TOUCHPAD_EVENT_BUTTON_PRESS)
		tp_tap_handle_event(tp, TAP_EVENT_BUTTON, time);

	tp_for_each_touch(tp, t) {
		if (!t->dirty || t->state == TOUCH_NONE)
			continue;

		if (t->state == TOUCH_BEGIN)
			tp_tap_handle_event(tp, TAP_EVENT_TOUCH, time);
		else if (t->state == TOUCH_END)
			tp_tap_handle_event(tp, TAP_EVENT_RELEASE, time);
		else if (tp->tap.state != TAP_STATE_IDLE &&
			 tp_tap_exceeds_motion_threshold(tp, t))
			tp_tap_handle_event(tp, TAP_EVENT_MOTION, time);
	}

	/**
	 * In any state where motion exceeding the move threshold would
	 * move to the next state, filter that motion until we actually
	 * exceed it. This prevents small motion events while we're waiting
	 * on a decision if a tap is a tap.
	 */
	switch (tp->tap.state) {
	case TAP_STATE_TOUCH:
	case TAP_STATE_TAPPED:
	case TAP_STATE_DRAGGING_OR_DOUBLETAP:
	case TAP_STATE_TOUCH_2:
	case TAP_STATE_TOUCH_3:
		filter_motion = 1;
		break;

	default:
		break;

	}

	return filter_motion;
}

static void
tp_tap_handle_timeout(uint64_t time, void *data)
{
	struct tp_dispatch *tp = data;

	tp_tap_handle_event(tp, TAP_EVENT_TIMEOUT, time);
}

int
tp_init_tap(struct tp_dispatch *tp)
{
	tp->tap.state = TAP_STATE_IDLE;

	libinput_timer_init(&tp->tap.timer,
			    tp->device->base.seat->libinput,
			    tp_tap_handle_timeout, tp);

	tp->tap.enabled = 1; /* FIXME */

	return 0;
}

void
tp_destroy_tap(struct tp_dispatch *tp)
{
	libinput_timer_cancel(&tp->tap.timer);
}
