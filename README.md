# Assignment 5

Building, loading, unloading drivers

```
foo@bar:~$ make -C /usr/src/linux-headers-$(uname -r) M=$PWD modules
foo@bar:~$ insmod assignment5.ko
foo@bar:~$ rmmod assignment5
```

## Deadlock Scenarios

1. e2\_open + e2\_ioctl

*see deadlock1 for test program and driver*

sem2 permanently lost due to mode switch between open and release. A process opens in MODE1 and downs sem2, then the mode switches to MODE2. When that process closes, release takes the MODE2 branch and skips up(sem2). sem2 is permanently stuck at and  all future MODE1 opens deadlock;

Modification required in e2\_open. We add a sleep before the down on sem2 inside the MODE1 case.

**Relevant Lines:** 50

2. e2\_ioctl + e2\_release

*see deadlock2 for test program and driver*

A circular wait is created during mode switching as it conflicts with another program closing the device.

When switching to MODE1 in e2\_ioctl the program acquires sem1 and attempts to also down sem2 for exclusive access. In e2\_release, the driver needs sem1 before it can release sem2. If one process starts a mode switch and acquires sem1 and waits (SLEEP INSERTED), a second process calling e2\_release blocks on sem1. The ioctl process then will also block on sem2, and neither can proceed. DEADLOCK!

**Relevant Lines:** 185 

3. e2\_ioctl (MODE2)

*see deadlock3 for test program and driver* 

ioctl MODE2 branch waits on queue1 but count1 skips value 1. The ioctl waits for count1 == 1, but if an opener is permanently stuck on sem2, count1 is inflated and never decrements to 1. The wait\_event condition is never satisfied, which means infinite wait.

**Relevant Lines:** 65

4. e2\_open 

*see deadlock4 for test program*

When two processes open in MODE1, the first downs and consumes sem2, but the second blocks forever on sem2 since up on sem2 does not happen until release.

No additional sleep statements or modifications are required for this one.

**Relevant Lines:** 49

## Race Condition Critical Regions

1. e2\_read (MODE1) vs e2\_write (MODE1)

**e2_read (84-90)**  
Data accessed: devc-\>ramdisk (read), \*f\_pos  
Locks held: none  
  
**e2_write (113-119)**  
Data accessed: devc-\>ramdisk (write), \*f\_pos  
Locks held: none  

Race **exists** because both functions release sem1 before accessing the ramdisk
and f\_pos. MODE1 is intended to be single access, but if a second process inherits an fd and attempts to read or write concurrently, both processes would be successful without locks.  

2. e2\_read (MODE2) vs e2\_write (MODE2)

**e2_read (98-100)**  
Data accessed: devc-\>ramdisk (read), \*f\_pos  
Locks held: sem1 
  
**e2_write (127-129)**  
Data accessed: devc-\>ramdisk (write), \*f\_pos  
Locks held: sem1

Race does **not exist** during single read and write in MODE2 because sem1 is held throughout the copy in both functions. They are mutually exclusive. 

3. e2\_open vs e2\_ioctl(E2\_IOCMODE2)

**e2_open (45-51)**  
Data accessed: devc-\>mode, devc-\>count1  
Locks held: sem1 during  mode check and count1 increment -- released before down(sem2)

**e2_ioctl (150, 156-166)**  
Data accessed: devc-\>mode, devc-\>count1, devc-\>count2  
Locks held: sem1 during most of case -- released during wait

Race **exists** on devc-\>mode. After e2\_open() reads mode==MODE1, increments count1, and releases sem1, the ioctl can acquire sem1 and change devc-\>mode to MODE2 before e2\_open() reaches down(sem2). This means both functions will complete under different conditions, causing inconsistencies with fd.

4. e2\_release vs e2\_ioctl(E2\_IOCMODE1)

**e2_release (62, 69-74)**  
Data accessed: devc-\>mode, devc-\>count2  
Locks held: sem1

**e2_ioctl (169, 175-185)**  
Data accessed: devc-\>mode, devc-\>count2, devc-\>count1  
Locks held: sem1 -- released during wait -- reacquired before and held during down(sem2)  

Race **exists** because the mode checked at wrong time in release. e2\_release() checks devc-\>mode under sem1, but the mode may have changed between when the process opened the device and when it closes it.   
