/*
* CSE 422s Lab 1 multi- thread monitoring framework
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/sched.h>

static unsigned long log_sec=1;
static unsigned long log_nsec=0;

//timer interval
static struct hrtimer timer;
//timer itself
static ktime_t ktime;
//task struct pointers for each of 4 cores
static struct task_struct *tp0;
static struct task_struct *tp1; 
static struct task_struct *tp2; 
static struct task_struct *tp3; 

module_param(log_sec, ulong, 0000);
module_param(log_nsec, ulong, 0000);

static enum hrtimer_restart expiration(struct hrtimer *timer)
{
  wake_up_process(tp0);
  wake_up_process(tp1);
  wake_up_process(tp2);
  wake_up_process(tp3);
  hrtimer_forward_now(timer, ktime);
  return HRTIMER_RESTART;
}

static int print_statistics(void *parameter)
{
  int i = 1; 
  //printk as indicated in Step 4, generating function name
  printk(KERN_INFO "print_statistics");
  while(1)
  {
    //print the requested statistics as in Step 4
    printk(KERN_INFO "Iteration %d pid %d nvcsw= %lu nivcsw= %lu core= %d\n",
                     i,current->pid,current->nvcsw,current->nivcsw, smp_processor_id());
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    i++;
    
    if(kthread_should_stop()) 
    {
      return 0;
    }
  }
}

static int mtmod_init(void)
{
ktime = ktime_set(log_sec, log_nsec);
hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
timer.function = &expiration;
hrtimer_start(&timer,ktime,HRTIMER_MODE_REL );

tp0 = kthread_create(print_statistics, NULL,"thread 0");
kthread_bind(tp0,0);
tp1 = kthread_create(print_statistics, NULL,"thread 1");
kthread_bind(tp1,1);
tp2 = kthread_create(print_statistics, NULL,"thread 2");
kthread_bind(tp2,2);
tp3 = kthread_create(print_statistics, NULL,"thread 3");
kthread_bind(tp3,3);
wake_up_process(tp0);
wake_up_process(tp1);
wake_up_process(tp2);
wake_up_process(tp3);

return 0;
}

static void mtmod_exit(void)
{
int check_done;
check_done = hrtimer_cancel(&timer);
if(check_done)
{
  printk(KERN_ERR "Err - Timer is still running\n");
}
check_done = kthread_stop(tp0);
//error checking before returning from function
if(check_done != -EINTR)
{
  printk(KERN_INFO "Thread 1 stopped\n");
}
check_done = kthread_stop(tp1);
if(check_done != -EINTR)
{
  printk(KERN_INFO "Thread 2 stopped\n");
}
check_done = kthread_stop(tp2);
if(check_done != -EINTR)
{
  printk(KERN_INFO "Thread 3 stopped\n");
}
check_done = kthread_stop(tp3);
if(check_done != -EINTR)
{
  printk(KERN_INFO "Thread 4 stopped\n");
}
}

module_init(mtmod_init);
module_exit(mtmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANAMBAKAM");
MODULE_DESCRIPTION("CSE 422s Single Thread Monitoring Framework");
