#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

// use things defined in assignment5.c
#define DEVICE "/dev/a6"
#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IO(CDRV_IOC_MAGIC, 1)
#define E2_IOCMODE2 _IO(CDRV_IOC_MAGIC, 2)

void* open_thread(void* arg) {
	while(1) {
		printf("[Open Thread] Attempting to open %s...\n", DEVICE);
		int fd = open(DEVICE, O_RDWR);
		if (fd >= 0) {
			printf("[Open Thread] Now closing...\n");
			close(fd);
		}
		usleep(10000);
	}
	return NULL;
}

void* ioctl_thread(void* arg) {
	int fd = open(DEVICE, O_RDWR);
	if (fd < 0) { perror("ioctl open"); exit(1); }

	while(1) {
		printf("[IOCTL Thread] Switching to MODE1...\n");
		ioctl(fd, E2_IOCMODE1);
		printf("[IOCTL Thread] Switching to MODE2...\n");
		ioctl(fd, E2_IOCMODE2);
	}
	return NULL;
}

int main() {
	// create 2 threads -- one to trigger opens and one for ioctl mode switches
	pthread_t t1, t2;
	pthread_create(&t1, NULL, open_thread, NULL);
	pthread_create(&t2, NULL, ioctl_thread, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

