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
	sleep(1);
	printf("[THREAD 1] opening in MODE1 -- will block on sem2 forever\n");
	fflush(stdout);
	int fd = open(DEVICE, O_RDWR);
	printf("[THREAD 1] open returned fd=%d (no deadlock :()\n", fd);
	return NULL;
}

int main(void) {
	pthread_t t1;

	ensure_mode1();

	printf("[MAIN] opening in MODE1 -- acquires sem2\n");
	int fd = open(DEVICE, O_RDWR);
	if (fd < 0) { perror("main open"); return 1; }
	printf("[MAIN] open succeeded (fd=%d) - holding open forever\n\n", fd);

	pthread_create(&t1, NULL, thread1_func, NULL);

	printf("[MAIN] T1 will block on sem2 -- program hangs here\n");
	fflush(stdout);

	pthread_join(t1, NULL); // never returns
	return 0;
}
