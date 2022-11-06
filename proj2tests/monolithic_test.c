#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/kernel.h>
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
    long rc = 0;
    unsigned char chr = 47;
    long chrLen = 1;
    unsigned char *dest = (unsigned char *)"Hello";
    long destLen = 5;
    unsigned char cool[50];
    long coolLen = 50;

    printf("Initializing first time\n");
    if ((rc = init_syscall()) != 0) {
        perror("First Initialization failed\n");
    }

    printf("Initializing second time\n");
    if ((rc = init_syscall()) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 10\n");
    if ((rc = create_syscall(10)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating a duplicate mailbox with ID: 10\n");
    if ((rc = create_syscall(10)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 5\n");
    if ((rc = create_syscall(5)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 7\n");
    if ((rc = create_syscall(7)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 0\n");
    if ((rc = create_syscall(0)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 1\n");
    if ((rc = create_syscall(1)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 4\n");
    if ((rc = create_syscall(4)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 6\n");
    if ((rc = create_syscall(6)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Creating mailbox with ID: 9\n");
    if ((rc = create_syscall(9)) != 0) {
        perror("Second Initialization failed\n");
    }

    printf("Trying to delete mailbox with ID: 10\n");
    if ((rc = destroy_syscall(10)) != 0) {
        perror("Error while shuttind down mailbox with ID: 10\n");
    }


    if ((rc = send_syscall(1, &chr, chrLen)) != 0) {
        perror("Error when sending the message\n");
    }

    if ((rc = send_syscall(1, dest, destLen)) != 0) {
        perror("Error when sending the message\n");
    }
    if ((rc = length_syscall(1)) < 1) {
        perror("Error when receiving the length of mailbox 1\n");
    }
    printf("Length of mailbox 1: %ld\n", rc);

    if ((rc = count_syscall(1)) < 1) {
        perror("Error when receiving the count of mailbox 1\n");
    }
    printf("Count of mailbox 1: %ld\n", rc);

    printf("should fail to receive from 5\n");
    if ((rc = recv_syscall(5, &chr, chrLen)) < 1) {
        perror("Error when receiving the message\n");
    }

    if ((rc = send_syscall(1, dest, destLen)) != 0) {
        perror("Error when sending the message\n");
    }

    if ((rc = send_syscall(1, &chr, chrLen)) != 0) {
        perror("Error when sending the message\n");
    }

    if ((rc = recv_syscall(1, dest, destLen)) < 1) {
        perror("Error when receiving the message\n");
    }
    printf("dest = %s, with length %ld\n", dest, rc);

    if ((rc = recv_syscall(1, &chr, chrLen)) < 1) {
        perror("Error when receiving the message\n");
    }
    printf("chr = %c, with length of %ld\n", chr, rc);

    if ((rc = recv_syscall(1, cool, coolLen)) < 1) {
        perror("Error when receiving the message\n");
    }
    printf("cool = %s, with length of %ld\n", cool, rc);
    
    if ((rc = recv_syscall(1, cool, coolLen)) < 1) {
        perror("Error when receiving the message\n");
    }
    printf("cool = %s, with length of %ld\n", cool, rc);

    if ((rc = length_syscall(1)) < 1) {
        perror("Error when receiving the length of mailbox 1\n");
    }
    printf("Length of mailbox 1: %ld\n", rc);

    if ((rc = count_syscall(1)) < 1) {
        perror("Error when receiving the count of mailbox 1\n");
    }
    printf("Count of mailbox 1: %ld\n", rc);

    if ((rc = send_syscall(5, cool, coolLen)) != 0) {
        perror("Error when sending the message\n");
    }

    if ((rc = delete_syscall(5)) != 0) {
        perror("Error when deleting the message\n");
    }

     printf("should fail to receive from 5\n");
    if ((rc = recv_syscall(5, &chr, chrLen)) < 1) {
        perror("Error when receiving the message\n");
    }

    printf("Shutting down Mailbox\n");
    if ((rc = shutdown_syscall()) != 0) {
        perror("Error shutting down mailbox system\n");
    }

    return 0;
}