#ifndef __DOIP_TIMER_H_INCLUDED__
#define __DOIP_TIMER_H_INCLUDED__

struct timer_loop;

struct uds_timer {
	unsigned char running;
	unsigned char suspend;
	unsigned char priority;
	unsigned char once;
	unsigned int index;
	unsigned int timerid;
	double timeout;
	double delay;
	double expire_time;
	void *userdata;
	void (*cb)(struct timer_loop *loop, struct uds_timer *timer);
} uds_timer;

struct timer_loop *timer_loop_alloc(unsigned long cap);

struct uds_timer *uds_timer_alloc(void (*cb)(struct timer_loop *loop, struct uds_timer *timer), \
		double timeout, double delay, unsigned char once);

struct uds_timer *uds_timer_init(struct uds_timer *timer, void (*cb)(struct timer_loop *loop, struct uds_timer *timer), \
		double timeout, double delay, unsigned char once);

void uds_timer_set_userdata(struct uds_timer *timer, void *userdata);

void *uds_timer_userdata(struct uds_timer *timer);

void uds_timer_start(struct timer_loop *loop, struct uds_timer *timer);

void uds_timer_stop(struct timer_loop *loop, struct uds_timer *timer);

unsigned char uds_timer_running(struct uds_timer *timer);

void uds_timer_destroy(struct uds_timer *timer);

void uds_timer_suspend(struct uds_timer *timer);

void uds_timer_resume(struct uds_timer *timer);

void uds_timer_loop(struct timer_loop *loop);

#endif
