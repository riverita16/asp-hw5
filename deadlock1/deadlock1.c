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

void* thread1(void* arg) {
	printf("[THREAD 1] opening device in MODE1...\n");
	int fd = open(DEVICE, O_RDWR);
	if (fd < 0) perror("open");
	printf("[THREAD 1] device opened\n");

	// switch to MODE2
	printf("[THREAD 1] switching to MODE2\n");
	ioctl(fd, E2_IOCMODE2);

	// back to MODE1
	printf("[THREAD 1] switching to MODE1 will block\n");
	ioctl(fd, E2_IOCMODE1);

	close(fd);
	printf("[THREAD 1] done -- should not print\n");
	return NULL;
}

void* thread2(void* arg) {
	sleep(1); // ensure open runs first
	int fd = open(DEVICE, O_RDWR);
	printf("[THREAD 2] opened device\n");
	printf("[THREAD 2] release will block\n");	
	close(fd);
	printf("[THREAD 2] done -- should not print\n");
	return NULL;
}

int main() {
	pthread_t t1, t2;

	// start in MODE1
	pthread_create(&t1, NULL, thread1, NULL);
	pthread_create(&t2, NULL, thread2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}
