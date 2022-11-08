#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>

#define __NR_init 548

const unsigned long INVALID = -1;

long init_syscall(void) {
    return syscall(__NR_init);
}

int main() {
    long rc = 0;
    printf("Initializing first time\n");
    if ((rc = init_syscall()) != 0) {
        perror("First Initialization failed\n");
    }

    return 0;
}