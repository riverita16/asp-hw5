#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <wait.h>

#define DEVICE "/dev/a6"
#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IO(CDRV_IOC_MAGIC, 1)

int main() {
    int fd_ioctl = open(DEVICE, O_RDWR);
    int fd_release = open(DEVICE, O_RDWR);

    if (fork() == 0) {
        // Switcher process
        printf("[IOCTL PROCESS] Switching to MODE1...\n");
        ioctl(fd_ioctl, E2_IOCMODE1);
        printf("[IOCTL PROCESS] Switched (no deadlock yet)...\n");
        exit(0);
    }

    // wait for switcher process then trigger release by closing fd
    sleep(1); 
    printf("[RELEASE PROCESS] Closing fd to trigger e2_release...\n");
    close(fd_release);
    printf("[RELEASE PROCESS] Close finished.\n");

    wait(NULL);
    return 0;
}

