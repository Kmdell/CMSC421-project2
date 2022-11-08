#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>

#define __NR_shut 549

long shutdown_syscall(void) {
    return syscall(__NR_shut);
}

int main() {
    long rc = 0;
    printf("Shutting down Mailbox System\n");
    if ((rc = shutdown_syscall()) != 0) {
        perror("Shutdown failed\n");
    }

    return 0;
}