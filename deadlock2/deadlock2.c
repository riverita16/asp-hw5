#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <wait.h>

#define DEVICE "/dev/a6"
#define E2_IOCMODE1 _IO('Z', 1)
#define E2_IOCMODE2 _IO('Z', 2)

int main() {

	int fd_holder = open(DEVICE, O_RDWR);
	ioctl(fd_holder, E2_IOCMODE1);

	int fd_setup = open(DEVICE, O_RDWR);
	ioctl(fd_setup, E2_IOCMODE2); // ensure we are switching INTO MODE1
	printf("[MODE2 ENSURED]\n");
	close(fd_setup);
	
	if (fork() == 0) {
		// Switcher process
		int fd_ioctl = open(DEVICE, O_RDWR);
		printf("[IOCTL PROCESS] Switching to MODE1...\n");
		ioctl(fd_ioctl, E2_IOCMODE1);
		printf("[IOCTL PROCESS] Switched (no deadlock yet)...\n");
		exit(0);
	}

	// wait for switcher process then trigger release by closing fd
	sleep(2); 

	printf("[RELEASE PROCESS] Closing fd to trigger e2_release...\n");
	int fd_release = open(DEVICE, O_RDWR);
	close(fd_release);
	printf("[RELEASE PROCESS] Close finished.\n");

	wait(NULL);
	return 0;
}

