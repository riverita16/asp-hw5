# Assignment 5

Building, loading, unloading drivers

```
foo@bar:~$ make -C /usr/src/linux-headers-$(uname -r) M=$PWD modules
foo@bar:~$ insmod assignment5.ko
foo@bar:~$ rmmod assignment5
```

## Deadlock Scenarios

1. e2\_open + e2\_ioctl\

*See deadlock1 directory for modified driver and tester program*\

Deadlock occurs between e2\_open and e2\_ioctl when the driver is in MODE1 and/or switching to MODE1.\

One thread in e2\_open downs sem1, updates a counter, releases sem1, and attempts to down sem2. A second thread in e2\_ioctl downs sem1 to change to MODE1. While still holding sem1, it attempts to down sem2. If the first thread is paused (SLEEP INSERTED) after releasing sem1 but before downing sem2, the second thread can grab sem1. The second thread blocks on sem2, and when the first thread resumes it too blocks on sem2. DEADLOCK!\

**Relevant Lines in Modified Driver**
47-55, 172-190\

2. e2\_ioctl + e2\_release\

*see deadlock2*\

A circular wait is created during mode switching as it conflicts with another program closing the device.\

When switching to MODE1 in e2\_ioctl the program acquires sem1 and attempts to also down sem2 for exclusive access. In e2\_release, the driver needs sem1 before it can release sem2. If one process starts a mode switch and acquires sem1 and waits (SLEEP INSERTED), a second process calling e2\_release blocks on sem1. The ioctl process then will also block on sem2, and neither can proceed. DEADLOCK!\

**Relevant Lines**


## Race Condition Critical Regions

1.
