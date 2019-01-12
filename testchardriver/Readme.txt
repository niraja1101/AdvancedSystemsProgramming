

Explanation for Deadlock Scenarios:

Testcase 1 (deadlock1.c):

A user space program is written to create a process and fork a child process. A new program is run in the child process using execl system call and it is ensured that both parent and child arte trying to access the common Driver concurrently. When the child process tries to access the lock, it cannot acquire it since the lock is with the parent. Meanwhile, the parent cannot release the lock until the child terminates. Thus the parent is waiting for the child and the child is waiting for the parent to make a change of state. This causes the deadlock.

Testcase 2 (deadlock2.c):

In this program too, a parent and child is created with a fresh program running in the child process, both pricesses are in mode 2. The variable count2 in the driver is thus 2. The child tries to change its mode to mode1 using ioctl but fails to do so since ioctl call waits for count2 to become 1 before changing the mode. The parent meanwhile waits for the child to terminate and thus count2 cannot become 1 until child has terminated. Thus child is waiting for parent to terminate and parent is waiting for child creating a deadlock.


Testcase 3 (deadlock3.c):

A multithreaded program is used to simulate a deadlock in this code. Two threads are created. The first thread runs in default mode 1 and acquires the lock for the driver. An infinite while loop is executed by the thread making it wait indefinitely while it still has the lock. Thus the second thread will get blocked while trying to open the device in Mode 1 since it cannot obtain the lock held by the first thread indefinitely. This causes a deadlock scenario.


Testcase 4 (deadlock4.c):

A multithreaded program is created consisiting of two threads. The first thread runs in Mode2 and the second runs in Mode1. Both threads try to change their mode. The second thread will change its mode to Mode 2 before the first thread can change its mode to Mode 1. To achieve this, a sleep statement must be inserted in the driver program at Line 172. Thus at this point, both threads will be in Mode 2. Thread 1 will now change its mode to Mode1 using ioctl but will get blocked since count2 is 2. Thus the second thread is waiting for termination of first thread and first thread is waiting for termination of the second (which will reduce the value of count2). This causes a deadlock.

Explanation for Race Condition Scenarios:

Critical region 1 (e2_open() and e2_release() funtion):

In the open() function of the driver, updating the values of the count1 and count2 shared variables is the possible critical region.
Improper synchronization without using sem1 and sem2 would have made the devices to read a wrong value of count1
and count2 which is an example of Race condition. But here, the critical region is perfectly enclosed within the sem1
and sem2 which prevents the potential Race Contions here. The same explanation goes for the e2_release() function as
well.

Critical region 2 (e2_read() and e2_write() function):

In these functions, accessing the devc->ramdisk variable is the important shared variable and a critical region. This
also has been properly synchronised using sem1 and sem2. If it hasn't been synchronised the devices would have read and
written wrong values to and from the driver.

Critical region 3 (e2_ioctl() function):

In this function, the count1 and count2 variables are the important critical shared variables. They are important in
changing the modes of the devices. They are properly synchronised using sem1 and sem2. Failure in synchronization would have put the devices in the wrong mode.
  
  
