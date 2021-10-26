/*
* CSE 422s Lab 1 single thread monitoring framework
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

//task struct pointer
static struct task_struct *tp; 

module_param(log_sec, ulong, 0000);
module_param(log_nsec, ulong, 0000);

static enum hrtimer_restart expiration(struct hrtimer *timer)
{
  //print function to indicate that the timer has been rescheduled, for interval statistic calculation
  printk(KERN_INFO "Timer restart.\n");
  wake_up_process(tp);
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
    //print statistics for thread-process Step 4 
    printk(KERN_INFO "Iteration %d pid %d nvcsw= %lu nivcsw= %lu\n",
                     i,current->pid,current->nvcsw,current->nivcsw);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    i++;
    
    if(kthread_should_stop()) 
    {
      return 0;
    }
  }
}
/*
* hello_init – the init function, called when the module is loaded.
* Returns zero if successfully loaded, nonzero otherwise.
*/
static int hello_init(void)
{
ktime = ktime_set(log_sec, log_nsec);
hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
timer.function = &expiration;
hrtimer_start(&timer,ktime,HRTIMER_MODE_REL );

tp = kthread_run(print_statistics, NULL,"thread");

return 0;
}

/*
* hello_exit – the exit function, called when the module is removed.
*/
static void hello_exit(void)
{
int fin;
fin = hrtimer_cancel(&timer);
if(fin)
{
  printk(KERN_ERR "Err - Timer is still running");
}
fin = kthread_stop(tp);
//error checking
if(fin != -EINTR)
{
  printk(KERN_INFO "Thread stopped\n");
}
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANAMBAKAM");
MODULE_DESCRIPTION("CSE 422s Single Thread Monitoring Framework");
