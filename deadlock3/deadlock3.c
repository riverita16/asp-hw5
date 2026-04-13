#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/a6"
#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE2 _IO(CDRV_IOC_MAGIC, 2)

// T1 opens and immediately closes 
void *opener_and_closer(void *arg) {
    printf("[THREAD 1] opening device in MODE1\n");
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("open"); return NULL; }
    printf("[THREAD 1] opened\n");
    sleep(60);
    close(fd);
    return NULL;
}

// thread 2 opens and ioctl MODE2
void *ioctl_switcher(void *arg) {
    printf("[THREAD 2] opening device in MODE1\n");
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("open"); return NULL; }
    printf("[THREAD 2] opened successfully, closing immediately\n");
    close(fd);
    printf("[THREAD 2] closed (count1 now 0)\n");

    printf("[THREAD 2] re-opening to issue ioctl\n");
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("re-open"); return NULL; }

    printf("[THREAD 2] issuing ioctl to MODE2 -- deadlock expected\n");
    ioctl(fd, E2_IOCMODE2);
    printf("[THREAD 2] ioctl returned (no deadlock -- unexpected)\n");
    close(fd);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    // t2 opens first to consume sem2
    pthread_create(&t2, NULL, ioctl_switcher, NULL);
    sleep(1);
    // t1 blocks on sem2 forever, count1 stays at 2/
    pthread_create(&ta, NULL, opener_and_closer, NULL);

    pthread_join(tb, NULL);
    pthread_join(ta, NULL);
    return 0;
}
