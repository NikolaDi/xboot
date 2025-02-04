/*
 * driver/cs-armv7-timer.c
 *
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <clocksource/clocksource.h>

static inline u64_t armv7_timer_frequecy(void)
{
	u32_t v;

	__asm__ __volatile__("mrc p15, 0, %0, c14, c0, 0" : "=r" (v));
	return (v != 0) ? (u64_t)v : 1000000;
}

static inline u64_t armv7_timer_read(void)
{
	u32_t l, h;

	__asm__ __volatile__("mrrc p15, 0, %0, %1, c14" : "=r" (l), "=r" (h));
	return ((u64_t)h << 32) | l;
}

static u64_t cs_armv7_timer_read(struct clocksource_t * cs)
{
	return armv7_timer_read();
}

static struct device_t * cs_armv7_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clocksource_t * cs;
	struct device_t * dev;
	s64_t rate = (s64_t)dt_read_long(n, "clock-frequency", -1);
	if(rate <= 0)
		rate = armv7_timer_frequecy();

	cs = malloc(sizeof(struct clocksource_t));
	if(!cs)
		return NULL;

	cs->name = alloc_device_name(dt_read_name(n), -1);
	cs->mask = CLOCKSOURCE_MASK(64);
	cs->read = cs_armv7_timer_read;
	cs->priv = NULL;
	clocksource_calc_mult_shift(&cs->mult, &cs->shift, (u64_t)rate, 1000000000ULL, 10);

	if(!(dev = register_clocksource(cs, drv)))
	{
		free_device_name(cs->name);
		free(cs->priv);
		free(cs);
		return NULL;
	}
	return dev;
}

static void cs_armv7_timer_remove(struct device_t * dev)
{
	struct clocksource_t * cs = (struct clocksource_t *)dev->priv;

	if(cs)
	{
		unregister_clocksource(cs);
		free_device_name(cs->name);
		free(cs->priv);
		free(cs);
	}
}

static void cs_armv7_timer_suspend(struct device_t * dev)
{
}

static void cs_armv7_timer_resume(struct device_t * dev)
{
}

static struct driver_t cs_armv7_timer = {
	.name		= "cs-armv7-timer",
	.probe		= cs_armv7_timer_probe,
	.remove		= cs_armv7_timer_remove,
	.suspend	= cs_armv7_timer_suspend,
	.resume		= cs_armv7_timer_resume,
};

static __init void cs_armv7_timer_driver_init(void)
{
	register_driver(&cs_armv7_timer);
}

static __exit void cs_armv7_timer_driver_exit(void)
{
	unregister_driver(&cs_armv7_timer);
}

driver_initcall(cs_armv7_timer_driver_init);
driver_exitcall(cs_armv7_timer_driver_exit);
