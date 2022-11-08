// Name: Kyle Dell
// Class: CMSC421 - Section 3
// Prof: Tompkins
// Email: kdell1@umbc.edu
// Date: 11/03/2022
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/mutex.h>

typedef struct queueNode {
    unsigned char *data;
    long length; 
    struct queueNode *next;
} queueNode;

typedef struct BSTNode {
    unsigned long ID;
    struct queueNode *begin, *end;
    long queueLength;
    struct BSTNode *left, *right;
} BSTNode;

static BSTNode *Root_Node = NULL;
struct mutex kmutex;

BSTNode *knew_node(unsigned long id) {
    BSTNode *new = (BSTNode *)kzalloc(sizeof(BSTNode), GFP_KERNEL);
    new->ID = id;
    new->begin = new->end = NULL;
    new->left = new->right = NULL;
    new->queueLength = 0;
    return new;
}

BSTNode *kdelete_node(BSTNode **node) {
    BSTNode *defNode = *node;
    /*need to find maximum value return it*/ 
    BSTNode *temp = defNode->left;
    while (temp->right != NULL) {
        temp = temp->right;
    }
    defNode->ID = temp->ID;
    defNode->begin = temp->begin;
    defNode->end = temp->end;
    defNode->queueLength = temp->queueLength;
    return defNode;
    /*free the current node*/ 
}

queueNode *knew_message(long len){
    queueNode *new = (queueNode *)kzalloc(sizeof(queueNode), GFP_KERNEL);
    new->length = len;
    new->next = NULL;
    return new;
}

long kcopy_send(unsigned char **dst, const unsigned char __user *src, int length) {
    unsigned char *tempStr = *dst;
    unsigned char temp;
    int ii = 0;
    /*check to make sure the pointer is readable*/
    if (!access_ok(src, length)) {
        return -EFAULT;
    }

    /*copies data from the user space pointer ocer to kernel space pointer*/
    for (ii = 0; ii < length; ii++) {
        if (get_user(temp, src + ii)) {
            return -EFAULT;
        }
        tempStr[ii] = temp;
    }
    temp = 0;
    return 0;
}

long kcopy_recv(unsigned char __user *dst, unsigned char *src, int length) {
    int ii = 0;
    /*check to make sure the pointer is readable*/
    if (!access_ok(dst, length)) {
        return -EFAULT;
    }

    /*copies data from the user space pointer ocer to kernel space pointer*/
    for (ii = 0; ii < length; ii++) {
        if (put_user(src[ii], dst + ii)) {
            return -EFAULT;
        }
    }
    return 0;
}

long kmailbox_shutdown(BSTNode **node) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    /*if null shutdown doesnt need to run*/
    if (*node == NULL) {
        return 0;
    }
    /*check children first*/
    kmailbox_shutdown(&defNode->left);
    kmailbox_shutdown(&defNode->right);
    temp = defNode->begin;
    /*while the begin is not null empty the mailbox*/
    while (temp != NULL) {
        defNode->begin = temp;
        temp = temp->next;
        kfree(defNode->begin->data);
        defNode->begin->data = NULL;
        kfree(defNode->begin);
        defNode->begin = NULL;
    }
    /*free the node and set to null for security*/
    kfree(*node);
    *node = NULL;
    return 0;
}

long kmailbox_create(BSTNode **node, unsigned long id) {
    /*if node is null then create new node here*/
    BSTNode *temp = *node;
    if (*node == NULL) {
        *node = knew_node(id);
        return 0;
    }
    /*recusrion untill you find a suitable place in tree*/
    if (temp->ID > id) {
        return kmailbox_create(&temp->left, id);
    } else if (temp->ID < id) {
        return kmailbox_create(&temp->right, id);
    }
    printk("Mailbox with ID %ld already exists\n", id);
    return -EEXIST;
}

long kmailbox_destroy(BSTNode **node, unsigned long id) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    /*if find a null node then return -ENOENT because it aint there chief*/
    if (*node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    /*go down each side of tree to find mailbBSTNode *defNode = *node;ox to delete*/
    if (defNode->ID > id) {
        return kmailbox_destroy(&defNode->left, id);
    } else if (defNode->ID < id) {
        return kmailbox_destroy(&defNode->right, id);
    } else {
        /*run algorithm for deleting the mailbox*/
        temp = defNode->begin;
        /*while the begin is not null empty the mailbox*/
        while (temp != NULL) {
            defNode->begin = temp;
            temp = temp->next;
            kfree(defNode->begin->data);
            defNode->begin->data = NULL;
            kfree(defNode->begin);
            defNode->begin = NULL;
        }
        if (defNode->left == NULL) {
            /*If the left is NULL then set the current node to the right node*/
            *node = defNode->right;
            kfree(defNode);
            defNode = NULL;
        } else if (defNode->right == NULL) {
            /*If the right node is NULL then set to the left node*/
            *node = defNode->left;
            kfree(defNode);
            defNode = NULL;
        } else {
            /*If neither are NULL then gotta do it the hard way*/
            *node = kdelete_node(node);
            defNode = *node;
            kmailbox_destroy(&defNode->left, defNode->ID);
        }
    }
    return 0;
}

int kmailbox_count(BSTNode *node, unsigned long id) {
    /*if not found return -ENOENT as error*/
    if (node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    /*recursively find the mailbox*/
    if (node->ID < id) {
        return kmailbox_count(node->right, id);
    } else if (node->ID > id) {
        return kmailbox_count(node->left, id);
    } else {
        /*return queueLength*/
        return node->queueLength;
    }
}

long kmailbox_send(BSTNode **node, unsigned long id, const unsigned char __user *msg, long len) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    unsigned char *tempStr = NULL;
    long rc = 0;
    /*if not found return -1*/
    if (*node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    /*recursively look for mailbox*/
    if (defNode->ID < id) {
        return kmailbox_send(&defNode->right, id, msg, len);
    } else if(defNode->ID > id) {
        return kmailbox_send(&defNode->left, id, msg, len);
    } else {
        /*create a new node with the data length of len and allocate a string for the node*/
        temp = knew_message(len);
        tempStr = (unsigned char *)kzalloc(len, GFP_KERNEL);
        if ((rc = kcopy_send(&tempStr, msg, len)) != 0) {
            return rc;
        }
        if (defNode->begin == NULL) {
            /* add the node to the beginning and end if no begin node exists*/
            defNode->begin = temp;
            defNode->end = temp;
            defNode->end->data = tempStr;
            defNode->queueLength = 1;
        } else {
            /*add the node to the end if the begin node exists and increment length*/
            defNode->end->next = temp;
            defNode->end = temp;
            defNode->end->data = tempStr;
            defNode->queueLength++;
        }
        return 0;
    }
}

long kmailbox_recv(BSTNode **node, unsigned long id, unsigned char *msg, long len) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    long rc = 0;
    /*return bad code if not found*/
    if (*node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    /*recursively go down each side of the tree*/
    if (defNode->ID < id) {
        return kmailbox_recv(&defNode->right, id, msg, len);
    } else if(defNode->ID > id) {
        return kmailbox_recv(&defNode->left, id, msg, len);
    } else {
        /*check that a message exists*/
        long length = 0;
        if (defNode->begin != NULL) {
            if (defNode->begin == defNode->end) {
                /*get the lowest length*/
                if ((length = defNode->begin->length) > len) {
                    length = len;
                }
                /*copy the string over to the message*/
                if ((rc = kcopy_recv(msg, defNode->begin->data, length)) != 0) {
                    return rc;
                }
                /*delete the string data*/
                kfree(defNode->begin->data);
                defNode->begin->data = NULL;
                /*delete the node that holds the message*/
                kfree(defNode->begin);
                defNode->begin = NULL;
                defNode->end = NULL;
                defNode->queueLength = 0;
            } else {
                temp = defNode->begin;
                /*get the lowest length*/
                if ((length = defNode->begin->length) > len) {
                    length = len;
                } 
                /*copy string*/
                kcopy_recv(msg, defNode->begin->data, length);
                /*free the string in queue node*/
                kfree(defNode->begin->data);
                defNode->begin->data = NULL;
                defNode->begin = defNode->begin->next;
                defNode->queueLength--;
                /*free the queuenode*/
                kfree(temp);
                temp = NULL;
            }
            return length;
        }
        printk("There are no messages in the mailbox\n");
        return -ESRCH;
    }
}

long kmessage_delete(BSTNode **node, unsigned long id) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    if (*node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    if (defNode->ID < id) {
        return kmessage_delete(&defNode->right, id);
    } else if (defNode->ID > id) {
        return kmessage_delete(&defNode->left, id);
    } else {
        if (defNode->begin != NULL) {
            if (defNode->begin == defNode->end) {
                /*free the first nodes data in queue*/
                kfree(defNode->begin->data);
                defNode->begin->data = NULL;
                /*free the the first node and */
                kfree(defNode->begin);
                defNode->begin = NULL;
                defNode->end = NULL;
                /*set queue length to zero*/
                defNode->queueLength = 0;
            } else {
                temp = defNode->begin;
                /*free the data in the queue node*/
                kfree(defNode->begin->data);
                defNode->begin->data = NULL;
                /*set the begin of queue to the next node then free the first node*/
                defNode->begin = defNode->begin->next;
                kfree(temp);
                temp = NULL;
                /*decrement the queue length*/
                defNode->queueLength--;
            }
            return 0;
        }
        printk("There are no messages in the mailbox with ID: %ld\n", id);
        return -ESRCH;
    }
}

long kmailbox_length(BSTNode *node, unsigned long id) {
    /*return error code */
    if (node == NULL) {
        printk("Could not find mailbox with ID: %ld\n", id);
        return -ENOENT;
    }
    /*if not here then traverse tree to find node with id*/
    if (node->ID < id) {
        return kmailbox_length(node->right, id);
    } else if (node->ID > id) {
        return kmailbox_length(node->left, id);
    } else {
        /*if the begin is not null then */
        if (node->begin != NULL) {
            return node->begin->length;
        }
        printk("There are no messages in the mailbox with ID: %ld\n", id);
        return -ESRCH;
    }
}

SYSCALL_DEFINE0(mailbox_init) {
    mutex_init(&kmutex);
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (Root_Node != NULL) {
        printk("A Mailbox is Already Initialized\n");
        mutex_unlock(&kmutex);
        return 0-ENOMEM;
    }
    /*Initialize the mailbox with a dumby variable*/
    Root_Node = kzalloc(sizeof(BSTNode), GFP_KERNEL);
    Root_Node->ID = 0;
    Root_Node->begin = Root_Node->end = NULL;
    Root_Node->queueLength = 0;
    Root_Node->left = Root_Node->right = NULL;
    printk("Initialized the Mailbox System\n");
    mutex_unlock(&kmutex);
    return 0;
}

SYSCALL_DEFINE0(mailbox_shutdown) {
    long rc = 0;
    printk("Mailbox Shutdown has been called\n");
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (Root_Node == NULL) {
        printk("Root Node is NULL\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    printk("Shutting Down Mailbox System\n");
    rc = kmailbox_shutdown(&Root_Node);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE1(mailbox_create, unsigned long, id) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    /*If the root node is the default node then we */
    if (Root_Node->ID == 0) {
        Root_Node->ID = id;
        mutex_unlock(&kmutex);
        return 0;
    }
    rc = kmailbox_create(&Root_Node, id);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE1(mailbox_destroy, unsigned long, id) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmailbox_destroy(&Root_Node, id);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE1(mailbox_count, unsigned long, id) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmailbox_count(Root_Node, id);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE3(mailbox_send, unsigned long, id, const unsigned char __user *, msg, long, len) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmailbox_send(&Root_Node, id, msg, len);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE3(mailbox_recv, unsigned long, id, unsigned char __user *, msg, long, len) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmailbox_recv(&Root_Node, id, msg, len);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE1(message_delete, unsigned long, id) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmessage_delete(&Root_Node, id);
    mutex_unlock(&kmutex);
    return rc;
}

SYSCALL_DEFINE1(mailbox_length, unsigned long, id) {
    long rc = 0;
    if (mutex_trylock(&kmutex) != 1) {
        printk("Could not get the mutex lock\n");
        return -EPERM;
    }
    if (id == 0) {
        printk("Mailbox System Refuses ID of 0\n");
        mutex_unlock(&kmutex);
        return -EINVAL;
    }
    /*If root is null then we return that the mailbox system is not initialized*/
    if (Root_Node == NULL) {
        printk("Mailbox System must be Initialized\n");
        mutex_unlock(&kmutex);
        return -ENODEV;
    }
    rc = kmailbox_length(Root_Node, id);
    mutex_unlock(&kmutex);
    return rc;
}