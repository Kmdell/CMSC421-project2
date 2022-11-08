#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>

#define __NR_init 548
#define __NR_shut 549
#define __NR_create 550
#define __NR_destroy 551
#define __NR_count 552
#define __NR_send 553
#define __NR_recv 554
#define __NR_delete 555
#define __NR_length 556

long init_syscall(void) {
    return syscall(__NR_init);
}

long shutdown_syscall(void) {
    return syscall(__NR_shut);
}

long create_syscall(unsigned long id) {
    return syscall(__NR_create, id);
}

long destroy_syscall(unsigned long id) {
    return syscall(__NR_destroy, id);
}

long count_syscall(unsigned long id) {
    return syscall(__NR_count, id);
}

long send_syscall(unsigned long id, unsigned char *msg, long len) {
    return syscall(__NR_send, id, msg, len);
}

long recv_syscall(unsigned long id, unsigned char *msg, long len) {
    return syscall(__NR_recv, id, msg, len);
}

long delete_syscall(unsigned long id) {
    return syscall(__NR_delete, id);
}

long length_syscall(unsigned long id) {
    return syscall(__NR_length, id);
}

int main() {
    unsigned char *src = (unsigned char *)"\\ testmonkey";
    long srcLen = 12;
    unsigned char dst[50];
    long rc = 0;
    int id = 1;
    int ii = 0;
    int jj = 0;

    for (ii = 0; ii < 15; ii++) {
        printf("Creating mailbox with ID: %d\n", ii);
        if ((rc = create_syscall(ii)) != 0) {
            perror("Initialization failed\n");
        }
    }

    for (ii = 0; ii < 15; ii++) {
        for (jj = 0; jj < 10; jj++) {
            printf("Sending to mailbox with ID: %d\n", ii);
            if ((rc = send_syscall(ii, src, srcLen)) != 0) {
                perror("Sending failed\n");
            }
        }
    }

    return 0;
}