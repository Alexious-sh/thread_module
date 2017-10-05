#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/atomic.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleksii Shavykin");

// Change value to 0 to disable using locks
#define USE_LOCKS 1

static long global_counter;
struct task_struct *task1, *task2;

static void lock(int *lock)
{
	int ret;

	do {
		ret = xchg(lock, 1);
	} while (ret == 1);
}

static void unlock(int *lock)
{
	*lock = 0;
}

// kthread function
static int thread_fn(void *data)
{
	int i;
	static int l;

	if (USE_LOCKS)
		lock(&l);

	for (i = 0; i < 1000000; i++) {
		global_counter++;
		ndelay(1);
	}

	if (USE_LOCKS)
		unlock(&l);

	pr_info("Thread finished! Counter value: %ld\n", global_counter);

	while (!kthread_should_stop())
		schedule();

	do_exit(1);

	return 0;
}

// Module init
int init_module(void)
{
	task1 = kthread_run(thread_fn, NULL, "my_thread_%d", 1);
	task2 = kthread_run(thread_fn, NULL, "my_thread_%d", 2);

	return 0;
}

// Module exit
void cleanup_module(void)
{
	kthread_stop(task1);
	kthread_stop(task2);

	pr_info("Good bye!\n");
}

