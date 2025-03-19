#include "uds_timer.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define is_pow_of_two(n) !((n) & (n -1))

#define MIN_DOIP_LOOP_CAP (32)

struct timer_loop {
	unsigned long cap;
	unsigned long num;
	void *userdata;
	struct uds_timer **timers;
} timer_loop;

static unsigned long roundup_to_power_of_two(unsigned long n)
{
	unsigned long val = 1;

	if (n <= 1) {
		return 1;
	}

	while (val < n) {
		val <<= 1;
	}

	return val;
}

static double get_system_runtime()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1e3 + ts.tv_nsec * 1e-6);
}

static void up(struct timer_loop *loop, unsigned long index)
{
	unsigned long child = index/2;
	struct uds_timer *temp = NULL;

	while (child) {
		if (loop->timers[child]->expire_time <= loop->timers[index]->expire_time) {
			break;
		}
		/* swap */
		temp = loop->timers[child];
		loop->timers[child] = loop->timers[index];
		loop->timers[child]->index = child;
		loop->timers[index] = temp;
		loop->timers[index]->index = index;
		index = child;
		child = index/2;
	}
}

static void down(struct timer_loop *loop, unsigned long index)
{
	unsigned long father = 2*index;
	struct uds_timer *temp = NULL;

	while (father <= loop->num) {
		if ((father < loop->num) && (loop->timers[father]->expire_time > loop->timers[father + 1]->expire_time)) {
			++father;
		}
		if (loop->timers[father]->expire_time >= loop->timers[index]->expire_time) {
			break;
		}
		/* swap */
		temp = loop->timers[father];
		loop->timers[father] = loop->timers[index];
		loop->timers[father]->index = father;
		loop->timers[index] = temp;
		loop->timers[index]->index = index;
		index = father;
		father *= 2;
	}
}

struct timer_loop *timer_loop_alloc(unsigned long cap)
{
	struct timer_loop *loop = NULL;

	loop = calloc(1, sizeof(*loop));
	if (!loop) {
		return NULL;
	}

	loop->cap = cap < MIN_DOIP_LOOP_CAP ? MIN_DOIP_LOOP_CAP : cap;
	loop->cap = is_pow_of_two(loop->cap) ? loop->cap : roundup_to_power_of_two(loop->cap);
	loop->num = 0;
	loop->userdata = NULL;
	loop->timers = calloc(loop->cap, sizeof(void *));
	if (!loop->timers) {
		goto nomem;
	}

	return loop;

nomem:
	free(loop);
	return NULL;
}

struct uds_timer *uds_timer_alloc(void (*cb)(struct timer_loop *loop, struct uds_timer *timer), \
		double timeout, double delay, unsigned char once)
{
	struct uds_timer *timer = NULL;

	timer = calloc(1, sizeof(*timer));
	if (!timer) {
		return NULL;
	}

	timer->cb = cb;
	timer->timeout = timeout;
	timer->delay = delay;
	timer->once = once;
	return timer;
}

struct uds_timer *uds_timer_init(struct uds_timer *timer, void (*cb)(struct timer_loop *loop, struct uds_timer *timer), \
		double timeout, double delay, unsigned char once)
{
	if (timer) {
		timer->cb = cb;
		timer->timeout = timeout;
		timer->delay = delay;
		timer->once = once;
	}

	return timer;
}

void uds_timer_set_userdata(struct uds_timer *timer, void *userdata)
{
	if (timer) {
		timer->userdata = userdata;
	}
}

void *uds_timer_userdata(struct uds_timer *timer)
{
	if (timer) {
		return timer->userdata;
	}

	return NULL;
}

void uds_timer_start(struct timer_loop *loop, struct uds_timer *timer)
{
	if (!(loop && timer)) {
		return;
	}

	if (timer->running) {
		timer->expire_time = timer->timeout + timer->delay + get_system_runtime();
		down(loop, timer->index);
		return;
	}

	if ((loop->num + 1) == loop->cap) {
		unsigned long cap = loop->cap < 1024 ? loop->cap * 2 : loop->cap + 1024;
		struct uds_timer **timers = calloc(cap, sizeof(void *));
		memset(timers, 0, cap * sizeof(void *));
		memcpy(timers, loop->timers, loop->cap * sizeof(void *));
		free(loop->timers);
		loop->cap = cap;
		loop->timers = timers;
	}

	++loop->num;
	loop->timers[loop->num] = timer;
	timer->running = 1;
	timer->index = loop->num;
	timer->timerid = loop->num;
	timer->expire_time = timer->timeout + timer->delay + get_system_runtime();
	up(loop, loop->num);
}

void uds_timer_stop(struct timer_loop *loop, struct uds_timer *timer)
{
	if (!(loop && timer && timer->running && (loop->num >= timer->index))) {
		return;
	}

	loop->timers[timer->index] = loop->timers[loop->num];
	loop->timers[timer->index]->index = timer->index;
	--loop->num;
	down(loop, timer->index);
	timer->running = 0;
}

unsigned char uds_timer_running(struct uds_timer *timer)
{
	if (timer) {
		return !!(timer->running);
	}

	return 0;
}

void uds_timer_destroy(struct uds_timer *timer)
{
	if (timer) {
		free(timer);
	}
}

void uds_timer_suspend(struct uds_timer *timer)
{
	if (timer) {
		timer->suspend = 1;
	}
}

void uds_timer_resume(struct uds_timer *timer)
{
	if (timer) {
		timer->suspend = 0;
	}
}

void uds_timer_loop(struct timer_loop *loop)
{
	double current;
	struct uds_timer *timer;

	current = get_system_runtime();
	for (; ;) {
		/* no timer running */
		if (loop->num == 0) {
			break;
		}
		timer = loop->timers[1];
		/* no timers timeout */
		if (timer->expire_time > current) {
			break;
		}
		if (timer->once) {
			uds_timer_stop(loop, timer);
		}
		else {
			timer->expire_time = timer->timeout + current;
			down(loop, timer->index);
		}
		if (timer->cb && !timer->suspend) {
			timer->cb(loop, timer);	
		}
	}
}
