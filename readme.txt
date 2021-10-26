1. Author: Anand Nambakam
anambakam@wustl.edu

2. Module Design

MODULE BUILD INSTRUCTION: 
To build the project modules, I followed the instructions from Studio 5: 
1. Ensure that the Makefile is included in the directory of the modules to be compiled
2. issue command "module add raspberry" 
3. issue command "LINUX_SOURCE=path to Linux kernel source code"
3a. In my case, LINUX_SOURCE=/project/scratch01/compile/anambakam/linux_source/linux
4. issue command "make -C $LINUX_SOURCE ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$PWD modules"
5. Verify that the monitor_s4.ko (single-threaded) and mt_monitor_s5.ko (multi-threaded) have been created. 

In the module init function, I initialized and started the hrtimer using hrtimer_init() and hrtimer_start(). 
An expiration function was created that is invoked upon timer expiration. This function contains the
hrtimer_forward() function to reschedule the timer's next expiration time to one interval in the future. 
In the module exit function, I used hrtimer_cancel to cease execution of the timer. 

The log_sec and log_nsec variables were defined using the module_param macro. These parameters were initialized
with unsighed long data type and file permission 0000. If the module does not provide parameters, they 
are initialized as default value 1 second. 

3. Timer Design and Evaluation 
In the init function, I passed the unsigned long variables (log_sec,log_nsec) to ktime_set, which 
stores the resultant value within the static ktime interval variable declared. The hrtimer is initialized and 
started with hrtimer_init and hrtimer_start respectively. The hrtimer is spawned using CLOCK_MONOTONIC. 

Upon timer expiration, the expiration function is invoked. This function takes the static struct hrtimer as 
parameter and returns HRTIMER_RESTART, indicating to the kernel that the timer is restarting. 
hrtimer_forward(timer,timenow,ktime) was used to reschedule the timer's next expiration to one timer interval
in the future. 

After the timer expires, the expiration function is invoked, returning HRTIMER_RESTART, 
letting the kernel know that the timer is restarted. The time variable is stored in a local variable,
which is checked to see if the timer is still running prior to termination. hrtimer_cancel(&hrtimers) was 
the function used to terminate the timer. 

//interval = 0.1s
sudo insmod monitor_s3.ko log_sec=0 log_nsec=100000000
Mar 14 04:52:23 raspberrypi kernel: [ 1068.027852] Timer restarted
Mar 14 04:52:23 raspberrypi kernel: [ 1068.127852] Timer restarted
Mar 14 04:52:23 raspberrypi kernel: [ 1068.227854] Timer restarted
Mar 14 04:52:23 raspberrypi kernel: [ 1068.327848] Timer restarted
Mar 14 04:52:23 raspberrypi kernel: [ 1068.427849] Timer restarted
Mar 14 04:52:23 raspberrypi kernel: [ 1068.527848] Timer restarted

//interval = 1s (default parameters)
sudo insmod monitor_s3.ko
Mar 14 04:53:46 raspberrypi kernel: [ 1150.898055] Timer restarted
Mar 14 04:53:47 raspberrypi kernel: [ 1151.898055] Timer restarted
Mar 14 04:53:48 raspberrypi kernel: [ 1152.898056] Timer restarted
Mar 14 04:53:49 raspberrypi kernel: [ 1153.898056] Timer restarted
Mar 14 04:53:50 raspberrypi kernel: [ 1154.898054] Timer restarted
Mar 14 04:53:51 raspberrypi kernel: [ 1155.898055] Timer restarted

//interval = 1.1s
sudo insmod monitor_s3.ko log_sec=1 log_nsec=100000000
Mar 14 04:54:35 raspberrypi kernel: [ 1200.035568] Timer restarted
Mar 14 04:54:36 raspberrypi kernel: [ 1201.135568] Timer restarted
Mar 14 04:54:37 raspberrypi kernel: [ 1202.235569] Timer restarted
Mar 14 04:54:38 raspberrypi kernel: [ 1203.335568] Timer restarted
Mar 14 04:54:39 raspberrypi kernel: [ 1204.435569] Timer restarted
Mar 14 04:54:40 raspberrypi kernel: [ 1205.535567] Timer restarted

//interval = 2.1s
sudo insmod monitor_s3.ko log_sec=2 log_nsec=100000000
Mar 14 04:55:08 raspberrypi kernel: [ 1232.605442] Timer restarted
Mar 14 04:55:10 raspberrypi kernel: [ 1234.705439] Timer restarted
Mar 14 04:55:12 raspberrypi kernel: [ 1236.805437] Timer restarted
Mar 14 04:55:14 raspberrypi kernel: [ 1238.905438] Timer restarted
Mar 14 04:55:16 raspberrypi kernel: [ 1241.005435] Timer restarted
Mar 14 04:55:18 raspberrypi kernel: [ 1243.105438] Timer restarted
Mar 14 04:55:20 raspberrypi kernel: [ 1245.205439] Timer restarted

//interval = 3.1s
sudo insmod monitor_s3.ko log_sec=3 log_nsec=100000000
Mar 14 04:55:48 raspberrypi kernel: [ 1273.032544] Timer restarted
Mar 14 04:55:51 raspberrypi kernel: [ 1276.132543] Timer restarted
Mar 14 04:55:54 raspberrypi kernel: [ 1279.232545] Timer restarted
Mar 14 04:55:57 raspberrypi kernel: [ 1282.332543] Timer restarted
Mar 14 04:56:00 raspberrypi kernel: [ 1285.432543] Timer restarted
Mar 14 04:56:03 raspberrypi kernel: [ 1288.532544] Timer restarted

Observations: 
Looking at 1s interval time, variance of 2+/- µs is observed between timer invocations. 
The variance for the 0.1s timer appears to be higher at 4+/- µs between timer invocations. 

4. Thread Design and Evaluation 
In the module init function, I used kthread_run() to spawn a respective thread function. The 
invoked static function uses a loop to print the iterative number of voluntary/involuntary context 
switches related to the current kernel thread. The state of the current process is changed 
from TASK_RUNNING to TASK_INTERRUPTIBLE and immediately call the schedule() function to 
relinquish thread control of the processor. kthread_should_stop() was used to see if the thread should 
stop (if the value of kthread_should_stop is true). 

The expiration() function was modified with wake_up_process() to regain control of sleeping kernel thread upon
invocation. The module init() function was changed to spawn a thread using kthread_run(). Respective
threads are terminated in the module exit() function with kthread_stop() passed the respective task pointer. 
This value is tested as error-checking to ensure a signal did not occur while the system call was in progress. 

//interval = 0.1s
sudo insmod monitor_s4.ko log_sec=0 log_nsec=100000000
Mar 14 21:48:42 raspberrypi kernel: [62047.483526] Iteration 37 pid 1823 nvcsw= 37 nivcsw= 0
Mar 14 21:48:43 raspberrypi kernel: [62047.583448] Timer restart.
Mar 14 21:48:43 raspberrypi kernel: [62047.583512] Iteration 38 pid 1823 nvcsw= 38 nivcsw= 0
Mar 14 21:48:43 raspberrypi kernel: [62047.683451] Timer restart.
Mar 14 21:48:43 raspberrypi kernel: [62047.683513] Iteration 39 pid 1823 nvcsw= 39 nivcsw= 0
Mar 14 21:48:43 raspberrypi kernel: [62047.783450] Timer restart.
Mar 14 21:48:43 raspberrypi kernel: [62047.783518] Iteration 40 pid 1823 nvcsw= 40 nivcsw= 0
Mar 14 21:48:43 raspberrypi kernel: [62047.883448] Timer restart.

//interval = 1s (default parameters)
sudo insmod monitor_s4.ko
Mar 14 21:50:32 raspberrypi kernel: [62156.635675] Iteration 2 pid 1854 nvcsw= 2 nivcsw= 0
Mar 14 21:50:33 raspberrypi kernel: [62157.635583] Timer restart.
Mar 14 21:50:33 raspberrypi kernel: [62157.635628] Iteration 3 pid 1854 nvcsw= 3 nivcsw= 0
Mar 14 21:50:34 raspberrypi kernel: [62158.635585] Timer restart.
Mar 14 21:50:34 raspberrypi kernel: [62158.635662] Iteration 4 pid 1854 nvcsw= 4 nivcsw= 0
Mar 14 21:50:35 raspberrypi kernel: [62159.635587] Timer restart.
Mar 14 21:50:35 raspberrypi kernel: [62159.635673] Iteration 5 pid 1854 nvcsw= 5 nivcsw= 0
Mar 14 21:50:36 raspberrypi kernel: [62160.635588] Timer restart.


//interval = 1.1s
sudo insmod monitor_s4.ko log_sec=1 log_nsec=100000000
Mar 14 21:52:48 raspberrypi kernel: [62293.429210] Iteration 1 pid 1898 nvcsw= 1 nivcsw= 1
Mar 14 21:52:49 raspberrypi kernel: [62294.528856] Timer restart.
Mar 14 21:52:49 raspberrypi kernel: [62294.529555] Iteration 2 pid 1898 nvcsw= 2 nivcsw= 1
Mar 14 21:52:51 raspberrypi kernel: [62295.628855] Timer restart.
Mar 14 21:52:51 raspberrypi kernel: [62295.629523] Iteration 3 pid 1898 nvcsw= 3 nivcsw= 1
Mar 14 21:52:52 raspberrypi kernel: [62296.728855] Timer restart.
Mar 14 21:52:52 raspberrypi kernel: [62296.729555] Iteration 4 pid 1898 nvcsw= 4 nivcsw= 1
Mar 14 21:52:53 raspberrypi kernel: [62297.828858] Timer restart.

//interval = 2.1s
sudo insmod monitor_s4.ko log_sec=2 log_nsec=100000000
Mar 14 21:53:21 raspberrypi kernel: [62325.772954] Iteration 1 pid 1928 nvcsw= 1 nivcsw= 0
Mar 14 21:53:23 raspberrypi kernel: [62327.872617] Timer restart.
Mar 14 21:53:23 raspberrypi kernel: [62327.873179] Iteration 2 pid 1928 nvcsw= 2 nivcsw= 0
Mar 14 21:53:25 raspberrypi kernel: [62329.972612] Timer restart.
Mar 14 21:53:25 raspberrypi kernel: [62329.973261] Iteration 3 pid 1928 nvcsw= 3 nivcsw= 0
Mar 14 21:53:27 raspberrypi kernel: [62332.072614] Timer restart.
Mar 14 21:53:27 raspberrypi kernel: [62332.073365] Iteration 4 pid 1928 nvcsw= 4 nivcsw= 0
Mar 14 21:53:29 raspberrypi kernel: [62334.172612] Timer restart.

//interval = 3.1s
sudo insmod monitor_s4.ko log_sec=3 log_nsec=100000000
Mar 14 21:53:53 raspberrypi kernel: [62357.912772] Iteration 1 pid 1950 nvcsw= 1 nivcsw= 0
Mar 14 21:53:56 raspberrypi kernel: [62361.012471] Timer restart.
Mar 14 21:53:56 raspberrypi kernel: [62361.014095] Iteration 2 pid 1950 nvcsw= 2 nivcsw= 0
Mar 14 21:53:59 raspberrypi kernel: [62364.112472] Timer restart.
Mar 14 21:53:59 raspberrypi kernel: [62364.113322] Iteration 3 pid 1950 nvcsw= 3 nivcsw= 0
Mar 14 21:54:02 raspberrypi kernel: [62367.212466] Timer restart.
Mar 14 21:54:02 raspberrypi kernel: [62367.213299] Iteration 4 pid 1950 nvcsw= 4 nivcsw= 0
Mar 14 21:54:05 raspberrypi kernel: [62370.312465] Timer restart.

Observations: 

The interval variation for 0.1s scheduling was 6.4µs between iterations and 1.5µs between timer reschedules. 
The interval variation for 1.0s scheduling was 21.8µs between iterations and 2.2µs between timer reschedules. 
The interval variation for 2.1s scheduling was 93.2µs between iterations and 2.36µs between timer reschedules. 

	1. As shown by the calculated variations above, there is more variation between iterations than timer reschedules,
	across interval rescheduling times chosen. 

	2. The nvcsw count increases when the module kernel thread are preempted voluntarily, and this is indicated 
	in every trial. Because my module makes an explicit call to schedule() upon every invocation of static function 
	print_statistics(). Therefore, nvcsw increases by one every iteration across all trials. The nivcsw count indicate
	event of the system scheduler suspending an active thread in favor of a different thread. Based on what my kernel
	thread function does (print statistics), involuntary contact switches indicate that multiple iterations 
	are occupying the same core and executing at the same time. For 2.1s scheduling trial, as nvcsw increased by 1 
	every iteration, nivcsw was 1 across iterations. 

5. Multi-threading Design and Evaluation 

I changed the single-thread task_struct pointer to 4 task_struct pointers, for each respective core of the RPi.
Within the module init function, I called kthread_create() four times with each respective task pointer, instead
of using kthread_run() in the single-thread variant. After each call to kthread_create(), I used kthread_bind()
to bind each respective spawned thread to a different core. Also, I woke up each thread to begin initial execution
using wake_up_process()

The expiration() callback function was also modified to wake_up_process() each respective process upon invocation. 
For each iteration, the module parameter indicates the time per interval. Because now 4 threads are spawned, 
with each running on a respective core, the static thread function is invoked 4 times concurrently upon 
expiration() function invocation. The module exit function was similarly changed to stop each of the four respectively spawned threads, using 
four kthread_stop() functions, each for a respective task_struct pointer. 

//interval = 0.1s
sudo insmod mt_monitor.ko log_sec=0 log_nsec=100000000
Mar 14 23:42:26 raspberrypi kernel: [68871.402560] Iteration 20 pid 2457 nvcsw= 20 nivcsw= 0 core= 1
Mar 14 23:42:26 raspberrypi kernel: [68871.402567] Iteration 20 pid 2456 nvcsw= 20 nivcsw= 0 core= 0
Mar 14 23:42:26 raspberrypi kernel: [68871.402574] Iteration 20 pid 2458 nvcsw= 20 nivcsw= 1 core= 2
Mar 14 23:42:26 raspberrypi kernel: [68871.402582] Iteration 20 pid 2459 nvcsw= 20 nivcsw= 0 core= 3
Mar 14 23:42:26 raspberrypi kernel: [68871.502559] Iteration 21 pid 2456 nvcsw= 21 nivcsw= 0 core= 0
Mar 14 23:42:26 raspberrypi kernel: [68871.502566] Iteration 21 pid 2457 nvcsw= 21 nivcsw= 0 core= 1
Mar 14 23:42:26 raspberrypi kernel: [68871.502572] Iteration 21 pid 2458 nvcsw= 21 nivcsw= 1 core= 2
Mar 14 23:42:26 raspberrypi kernel: [68871.502580] Iteration 21 pid 2459 nvcsw= 21 nivcsw= 0 core= 3
Mar 14 23:42:27 raspberrypi kernel: [68871.602566] Iteration 22 pid 2456 nvcsw= 22 nivcsw= 0 core= 0
Mar 14 23:42:27 raspberrypi kernel: [68871.602574] Iteration 22 pid 2458 nvcsw= 22 nivcsw= 1 core= 2
Mar 14 23:42:27 raspberrypi kernel: [68871.602582] Iteration 22 pid 2459 nvcsw= 22 nivcsw= 0 core= 3
Mar 14 23:42:27 raspberrypi kernel: [68871.602590] Iteration 22 pid 2457 nvcsw= 22 nivcsw= 0 core= 1

//interval = 1s (default parameters)
sudo insmod mt_monitor.ko
Mar 14 23:41:32 raspberrypi kernel: [68817.479158] Iteration 6 pid 2422 nvcsw= 6 nivcsw= 0 core= 1
Mar 14 23:41:32 raspberrypi kernel: [68817.479166] Iteration 6 pid 2421 nvcsw= 6 nivcsw= 0 core= 0
Mar 14 23:41:32 raspberrypi kernel: [68817.479173] Iteration 6 pid 2424 nvcsw= 6 nivcsw= 0 core= 3
Mar 14 23:41:32 raspberrypi kernel: [68817.479181] Iteration 6 pid 2423 nvcsw= 6 nivcsw= 1 core= 2
Mar 14 23:41:33 raspberrypi kernel: [68818.479158] Iteration 7 pid 2421 nvcsw= 7 nivcsw= 0 core= 0
Mar 14 23:41:33 raspberrypi kernel: [68818.479166] Iteration 7 pid 2422 nvcsw= 7 nivcsw= 0 core= 1
Mar 14 23:41:33 raspberrypi kernel: [68818.479174] Iteration 7 pid 2423 nvcsw= 7 nivcsw= 1 core= 2
Mar 14 23:41:33 raspberrypi kernel: [68818.479181] Iteration 7 pid 2424 nvcsw= 7 nivcsw= 0 core= 3
Mar 14 23:41:34 raspberrypi kernel: [68819.479157] Iteration 8 pid 2421 nvcsw= 8 nivcsw= 0 core= 0
Mar 14 23:41:34 raspberrypi kernel: [68819.479164] Iteration 8 pid 2422 nvcsw= 8 nivcsw= 0 core= 1
Mar 14 23:41:34 raspberrypi kernel: [68819.479172] Iteration 8 pid 2423 nvcsw= 8 nivcsw= 1 core= 2
Mar 14 23:41:34 raspberrypi kernel: [68819.479179] Iteration 8 pid 2424 nvcsw= 8 nivcsw= 0 core= 3

//interval = 1.1s
sudo insmod mt_monitor.ko log_sec=1 log_nsec=100000000
Mar 14 23:44:13 raspberrypi kernel: [68978.275511] Iteration 4 pid 2535 nvcsw= 4 nivcsw= 0 core= 0
Mar 14 23:44:13 raspberrypi kernel: [68978.275519] Iteration 4 pid 2536 nvcsw= 4 nivcsw= 0 core= 1
Mar 14 23:44:13 raspberrypi kernel: [68978.275527] Iteration 4 pid 2537 nvcsw= 4 nivcsw= 1 core= 2
Mar 14 23:44:13 raspberrypi kernel: [68978.275534] Iteration 4 pid 2538 nvcsw= 4 nivcsw= 0 core= 3
Mar 14 23:44:14 raspberrypi kernel: [68979.375530] Iteration 5 pid 2536 nvcsw= 5 nivcsw= 0 core= 1
Mar 14 23:44:14 raspberrypi kernel: [68979.375537] Iteration 5 pid 2537 nvcsw= 5 nivcsw= 1 core= 2
Mar 14 23:44:14 raspberrypi kernel: [68979.375545] Iteration 5 pid 2538 nvcsw= 5 nivcsw= 0 core= 3
Mar 14 23:44:14 raspberrypi kernel: [68979.375552] Iteration 5 pid 2535 nvcsw= 5 nivcsw= 0 core= 0
Mar 14 23:44:15 raspberrypi kernel: [68980.475520] Iteration 6 pid 2536 nvcsw= 6 nivcsw= 0 core= 1
Mar 14 23:44:15 raspberrypi kernel: [68980.475528] Iteration 6 pid 2537 nvcsw= 6 nivcsw= 1 core= 2
Mar 14 23:44:15 raspberrypi kernel: [68980.475536] Iteration 6 pid 2538 nvcsw= 6 nivcsw= 0 core= 3
Mar 14 23:44:15 raspberrypi kernel: [68980.475544] Iteration 6 pid 2535 nvcsw= 6 nivcsw= 0 core= 0


//interval = 2.1s
sudo insmod mt_monitor.ko log_sec=2 log_nsec=100000000
Mar 14 23:45:14 raspberrypi kernel: [69038.993648] Iteration 3 pid 2582 nvcsw= 3 nivcsw= 0 core= 1
Mar 14 23:45:14 raspberrypi kernel: [69038.993656] Iteration 3 pid 2583 nvcsw= 3 nivcsw= 1 core= 2
Mar 14 23:45:14 raspberrypi kernel: [69038.993663] Iteration 3 pid 2584 nvcsw= 3 nivcsw= 0 core= 3
Mar 14 23:45:14 raspberrypi kernel: [69038.993671] Iteration 3 pid 2581 nvcsw= 3 nivcsw= 0 core= 0
Mar 14 23:45:16 raspberrypi kernel: [69041.093651] Iteration 4 pid 2582 nvcsw= 4 nivcsw= 0 core= 1
Mar 14 23:45:16 raspberrypi kernel: [69041.093659] Iteration 4 pid 2581 nvcsw= 4 nivcsw= 0 core= 0
Mar 14 23:45:16 raspberrypi kernel: [69041.093666] Iteration 4 pid 2584 nvcsw= 4 nivcsw= 0 core= 3
Mar 14 23:45:16 raspberrypi kernel: [69041.093675] Iteration 4 pid 2583 nvcsw= 4 nivcsw= 1 core= 2
Mar 14 23:45:18 raspberrypi kernel: [69043.193650] Iteration 5 pid 2582 nvcsw= 5 nivcsw= 0 core= 1
Mar 14 23:45:18 raspberrypi kernel: [69043.193658] Iteration 5 pid 2581 nvcsw= 5 nivcsw= 0 core= 0
Mar 14 23:45:18 raspberrypi kernel: [69043.193665] Iteration 5 pid 2583 nvcsw= 5 nivcsw= 1 core= 2
Mar 14 23:45:18 raspberrypi kernel: [69043.193672] Iteration 5 pid 2584 nvcsw= 5 nivcsw= 0 core= 3


//interval = 3.1s
sudo insmod mt_monitor.ko log_sec=3 log_nsec=100000000
Mar 15 00:24:55 raspberrypi kernel: [71420.460952] Iteration 4 pid 2738 nvcsw= 4 nivcsw= 0 core= 0
Mar 15 00:24:55 raspberrypi kernel: [71420.460960] Iteration 4 pid 2739 nvcsw= 4 nivcsw= 0 core= 1
Mar 15 00:24:55 raspberrypi kernel: [71420.460967] Iteration 4 pid 2740 nvcsw= 4 nivcsw= 1 core= 2
Mar 15 00:24:55 raspberrypi kernel: [71420.460975] Iteration 4 pid 2741 nvcsw= 4 nivcsw= 0 core= 3
Mar 15 00:24:58 raspberrypi kernel: [71423.560949] Iteration 5 pid 2738 nvcsw= 5 nivcsw= 0 core= 0
Mar 15 00:24:58 raspberrypi kernel: [71423.560957] Iteration 5 pid 2739 nvcsw= 5 nivcsw= 0 core= 1
Mar 15 00:24:58 raspberrypi kernel: [71423.560964] Iteration 5 pid 2740 nvcsw= 5 nivcsw= 1 core= 2
Mar 15 00:24:58 raspberrypi kernel: [71423.560972] Iteration 5 pid 2741 nvcsw= 5 nivcsw= 0 core= 3
Mar 15 00:25:02 raspberrypi kernel: [71426.660953] Iteration 6 pid 2738 nvcsw= 6 nivcsw= 0 core= 0
Mar 15 00:25:02 raspberrypi kernel: [71426.660960] Iteration 6 pid 2739 nvcsw= 6 nivcsw= 0 core= 1
Mar 15 00:25:02 raspberrypi kernel: [71426.660968] Iteration 6 pid 2741 nvcsw= 6 nivcsw= 0 core= 3
Mar 15 00:25:02 raspberrypi kernel: [71426.660976] Iteration 6 pid 2740 nvcsw= 6 nivcsw= 1 core= 2
Mar 15 00:25:05 raspberrypi kernel: [71429.760950] Iteration 7 pid 2738 nvcsw= 7 nivcsw= 0 core= 0
Mar 15 00:25:05 raspberrypi kernel: [71429.760957] Iteration 7 pid 2739 nvcsw= 7 nivcsw= 0 core= 1
Mar 15 00:25:05 raspberrypi kernel: [71429.760964] Iteration 7 pid 2741 nvcsw= 7 nivcsw= 0 core= 3
Mar 15 00:25:05 raspberrypi kernel: [71429.760971] Iteration 7 pid 2740 nvcsw= 7 nivcsw= 1 core= 2

	For the 0.1s trial, the variation between interval is indicated below per-thread:
	thread 0 variation = 4.4µs; thread 1 variation = 15.87µs; thread 2 variation = 1.13µs; thread 3 variation = 1µs
	The average variation per-thread for the 0.1s mutli-thread trial is 5.6µs.
	The average variation for the single-thread 0.1s trial was 6.4µs. 

	For the 1.0s trial: 
	thread 0 variation = 4.93µs; thread 1 variation =  4.16µs; thread 2 variation = 8.54µs; thread 3 variation = 4.16µs
	The average variation per-thread for the 1.0s multi-thread trial is 5.4µs.
	The average variation for the single-thread 1.0s trial was 21.8µs.

	For the 2.1s trial: 
	thread 0 variation = 4.58µs; thread 1 variation = 11.53µs; thread 2 variation = 9.5µs; thread 3 variation = 1.13µs
	The average variation per-thread for the 2.1s multi-thread trial is 6.7µs.
	The average variation for the single-thread 2.1s trial was 93.2µs.

	1. As indicated by the statistics above, the variation of each loop's iteration varies less on average in the 
	multi-threaded monitor compared to the single-threaded monitor. The module parameter specifies the timeout value
	for every iteration. Because the multi-threaded variant runs four threads on four cores concurrently, the increased
	number of threads may reduce the value of the variance. I expected there to be greater variance across four 
	concurrent threads. As the interval time increased, there appeared to be marginal change in the average
	per-thread variation, as compared to the single-threaded trials. 

	2. As in the single-threaded monitor, the nvcsw count increases by one per iteration, due to the explicit call to 
	schedule(). It is noted that only core 2 receives involuntary preemption, across all time interval trials. 
	Process threads are respective to the processor running them. I observed that there were more involuntary process
	switches in the multi-threaded module than the single-threaded module. In this multi-threaded variant, an 
	nivcsw=1 was observed across all trials for respective core 2. nivcsw only increased for one trial (1.1s) in the 
	single-threaded example. 

6. The screenshot is included as "trace.png". Looking at the trace in kernelshark, I can verify that 
the program is being scheduled at consistent intervals. 

7. System Performance 
	1. Zooming in on several thread wakeups, I found threads ran to completion in most cases, across 
	different cores and different locations in the trace. Because four threads are running on four 
	cores, some preemptions are noticed, indicated by overlapping colors in the kernelshark window. 

	2. Given that the threads are run in kernel mode, they are preempted by threads with higher associated
	priority value. An interrupt with a higher interrupt request level could cause these threads to be 
	preempted. 

	Thread wakeup total execution time estimation: 
		Group 1: 68µs
		Group 2: 72µs
		Group 3: 78µs

	Average total execution time (3 trials) = 72.7µs. 
	These results were calculated by the delta value in kernelshark. The included total_exec.png screenshot 
	details one trial calculation. 

	Thread jitter calculation: 
	Difference between thread first and last thread startup times:
		Trial 1: 21µs
		Trial 2: 10µs
		Trial 3: 16µs

		Minimum = 10µs
		Maximum = 21µs
		Mean = 15.7µs

	Calculation over five wakeups of thread/0:
		Trial 1: 205.754937
		Trial 2: 206.754898
		Trial 3: 207.754885
		Trial 4: 208.754868
		Trial 5: 209.754833

		Minimum = 1µs
		Maximum = 53µs
		Mean = 38.3µs

8. Development Effort 
I spent approximately 12 person-hours on this project. 