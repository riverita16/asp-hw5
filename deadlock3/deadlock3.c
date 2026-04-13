#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/a6"
#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IO(CDRV_IOC_MAGIC, 1)
#define E2_IOCMODE2 _IO(CDRV_IOC_MAGIC, 2)

void ensure_mode1(void) {
    int fd;
    printf("[SETUP] forcing MODE2 to reset sem2...\n");
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("setup open"); exit(1); }
    ioctl(fd, E2_IOCMODE2);
    close(fd);

    printf("[SETUP] switching back to MODE1...\n");
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) { perror("setup open2"); exit(1); }
    ioctl(fd, E2_IOCMODE1);
    close(fd);
    printf("[SETUP] device in MODE1\n\n");
}

void *thread1_func(void *arg) {
    printf("[THREAD 1] opening in MODE1\n");
    fflush(stdout);
    int fd = open(DEVICE, O_RDWR);
    printf("[THREAD 1] open returned fd=%d\n", fd);

    // now switch to MODE2 while t2 and t3 still sleeping in MODE1 open
    printf("[THREAD 1] switching to MODE2\n");
    ioctl(fd, E2_IOCMODE2);
    printf("[THREAD 1] sem1 and sem2 released in switch\n");

    // release
    close(fd);
    printf("[THREAD 1] done\n");

    return NULL;
}

void *thread2_func(void *arg) {
    sleep(4); // let t1 stay ahead
    printf("[THREAD 2] opening in MODE1 -- will hang on sem2 for a bit\n");
    fflush(stdout);
    int fd = open(DEVICE, O_RDWR);
    printf("[THREAD 2] open returned fd=%d\n", fd);

    // release in MODE2
    close(fd);
    printf("[THREAD 2] releasing in MODE2 -- sem2 held forever\n");
    printf("[THREAD 2] done --  T3 in deadlock\n");
    
    return NULL;
}

void *thread3_func(void *arg) {
    sleep(4); // let t1 stay ahead
    printf("[THREAD 3] opening in MODE1 -- will hang on sem2 for a bit\n");
    fflush(stdout);
    int fd = open(DEVICE, O_RDWR);
    printf("[THREAD 3] open returned fd=%d\n", fd);

    // release in MODE2
    close(fd);
    printf("[THREAD 3] releasing in MODE2 -- sem2 held forever\n");
    printf("[THREAD 3] done -- T2 in deadlock\n");

    return NULL;
}

int main(void) {
    pthread_t t1, t2, t3;

    ensure_mode1();

    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);
    pthread_create(&t3, NULL, thread3_func, NULL);

    pthread_join(t1, NULL);
    // t2 or t3 will never return
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
