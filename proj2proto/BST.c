#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int copy_data(unsigned char **dst, const unsigned char *src, int length) {
    unsigned char *tempStr = *dst;
    int ii = 0;
    for (ii = 0; ii < length; ii++) {
        tempStr[ii] = src[ii];
    }
    return 0;
}

BSTNode *newBSTNode(unsigned long id) {
    BSTNode *new = (BSTNode *)malloc(sizeof(BSTNode));
    new->ID = id;
    new->begin = new->end = NULL;
    new->left = new->right = NULL;
    new->queueLength = 0;
    return new;
}

BSTNode *deleteBSTNode(BSTNode **node) {
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

queueNode *newQueueNode(long len){
    queueNode *new = (queueNode *)malloc(sizeof(queueNode));
    new->length = len;
    new->next = NULL;
    return new;
}

int create_mailbox(BSTNode **node, unsigned long id) {
    /*if node is null then create new node here*/
    BSTNode *temp = *node;
    if (*node == NULL) {
        *node = newBSTNode(id);
        return 0;
    }
    /*recusrion untill you find a suitable place in tree*/
    if (temp->ID > id) {
        return create_mailbox(&temp->left, id);
    } else if (temp->ID < id) {
        return create_mailbox(&temp->right, id);
    }
    return -1;
}

int shutdown_mailbox(BSTNode **node) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    /*if null shutdown doesnt need to run*/
    if (*node == NULL) {
        return 0;
    }
    /*check children first*/
    shutdown_mailbox(&defNode->left);
    shutdown_mailbox(&defNode->right);
    temp = defNode->begin;
    /*while the begin is not null empty the mailbox*/
    while (temp != NULL) {
        defNode->begin = temp;
        temp = temp->next;
        free(defNode->begin->data);
        defNode->begin->data = NULL;
        free(defNode->begin);
        defNode->begin = NULL;
    }
    /*free the node and set to null for security*/
    free(*node);
    *node = NULL;
    return 0;
}

int mailbox_destroy(BSTNode **node, unsigned long id) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    /*if find a null node then return -1 because it aint there chief*/
    if (*node == NULL) {
        return -1;
    }
    /*go down each side of tree to find mailbox to delete*/
    if (defNode->ID > id) {
        return mailbox_destroy(&defNode->left, id);
    } else if (defNode->ID < id) {
        return mailbox_destroy(&defNode->right, id);
    } else {
        /*run algorithm for deleting the mailbox*/
        temp = defNode->begin;
        /*while the begin is not null empty the mailbox*/
        while (temp != NULL) {
            defNode->begin = temp;
            temp = temp->next;
            free(defNode->begin->data);
            defNode->begin->data = NULL;
            free(defNode->begin);
            defNode->begin = NULL;
        }
        if (defNode->left == NULL) {
            /*If the left is NULL then set the current node to the right node*/
            *node = defNode->right;
            free(defNode);
            defNode = NULL;
        } else if (defNode->right == NULL) {
            /*If the right node is NULL then set to the left node*/
            *node = defNode->left;
            free(defNode);
            defNode = NULL;
        } else {
            /*If neither are NULL then gotta do it the hard way*/
            *node = deleteBSTNode(node);
            defNode = *node;
            mailbox_destroy(&defNode->left, defNode->ID);
        }
    }
    return 0;
}

int mailbox_count(BSTNode *node, unsigned long id) {
    /*if not found return -1 as error*/
    if (node == NULL) {
        return -1;
    }
    /*recursively find the mailbox*/
    if (node->ID < id) {
        return mailbox_count(node->right, id);
    } else if (node->ID > id) {
        return mailbox_count(node->left, id);
    } else {
        /*return queueLength*/
        return node->queueLength;
    }
    return -1;
}

int mailbox_send(BSTNode **node, unsigned long id, const unsigned char *msg, long len) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    unsigned char *tempStr = NULL;
    /*if not found return -1*/
    if (*node == NULL) {
        return -1;
    }
    /*recursively look for mailbox*/
    if (defNode->ID < id) {
        return mailbox_send(&defNode->right, id, msg, len);
    } else if(defNode->ID > id) {
        return mailbox_send(&defNode->left, id, msg, len);
    } else {
        /*create a new node with the data length of len and allocate a string for the node*/
        temp = newQueueNode(len);
        tempStr = (unsigned char *)malloc(len);
        printf("Before copy_data\n");
        copy_data(&tempStr, msg, len);
        printf("After Copy_data\n");
        if (defNode->begin == NULL) {
            /* add the node to the beginning and end if no begin node exists*/
            defNode->begin = temp;
            defNode->end = temp;
            defNode->end->data = tempStr;
            defNode->queueLength = 1;
            return 0;
        } else {
            /*add the node to the end if the begin node exists and increment length*/
            defNode->end->next = temp;
            defNode->end = temp;
            defNode->end->data = tempStr;
            defNode->queueLength++;
            return 0;
        }
    }
    return -1;
}

int mailbox_recv(BSTNode **node, unsigned long id, unsigned char **msg, long len) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    /*return bad code if not found*/
    if (*node == NULL) {
        return -1;
    }
    /*recursively go down each side of the tree*/
    if (defNode->ID < id) {
        return mailbox_recv(&defNode->right, id, msg, len);
    } else if(defNode->ID > id) {
        return mailbox_recv(&defNode->left, id, msg, len);
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
                copy_data(msg, defNode->begin->data, length);
                /*delete the string data*/
                free(defNode->begin->data);
                defNode->begin->data = NULL;
                /*delete the node that holds the message*/
                free(defNode->begin);
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
                copy_data(msg, defNode->begin->data, length);
                /*free the string in queue node*/
                free(defNode->begin->data);
                defNode->begin->data = NULL;
                defNode->begin = defNode->begin->next;
                defNode->queueLength--;
                /*free the queuenode*/
                free(temp);
                temp = NULL;
            }
        }
        return length;
    }
    return -1;
}

int message_delete(BSTNode **node, unsigned long id) {
    BSTNode *defNode = *node;
    queueNode *temp = NULL;
    if (*node == NULL) {
        return -1;
    }
    if (defNode->ID < id) {
        return message_delete(&defNode->right, id);
    } else if (defNode->ID > id) {
        return message_delete(&defNode->left, id);
    } else {
        if (defNode->begin != NULL) {
            if (defNode->begin == defNode->end) {
                /*free the first nodes data in queue*/
                free(defNode->begin->data);
                defNode->begin->data = NULL;
                /*free the the first node and */
                free(defNode->begin);
                defNode->begin = NULL;
                defNode->end = NULL;
                /*set queue length to zero*/
                defNode->queueLength = 0;
            } else {
                temp = defNode->begin;
                /*free the data in the queue node*/
                free(defNode->begin->data);
                defNode->begin->data = NULL;
                /*set the begin of queue to the next node then free the first node*/
                defNode->begin = defNode->begin->next;
                free(temp);
                temp = NULL;
                /*decrement the queue length*/
                defNode->queueLength--;
            }
            return 0;
        }
    }
    return -1;
}        

int mailbox_length(BSTNode **node, unsigned long id) {
    BSTNode *defNode = *node;
    /*return error code */
    if (*node == NULL) {
        return -1;
    }
    /*if not here then traverse tree to find node with id*/
    if (defNode->ID < id) {
        return mailbox_length(&defNode->right, id);
    } else if (defNode->ID > id) {
        return mailbox_length(&defNode->left, id);
    } else {
        /*if the begin is not null then */
        if (defNode->begin != NULL) {
            return defNode->begin->length;
        }
    }
    return -1;
}

int printInorder(BSTNode *node) {
    queueNode *temp = NULL;
    if (node == NULL) {
        return 0;
    }
    temp = node->begin;
    printInorder(node->left);
    printf("ID: %ld\n", node->ID);
    printf("Number of mail %ld\n", node->queueLength);
    while (temp != NULL) {
        printf("Length of mail: %ld\n", temp->length);
        temp = temp->next;
    }
    printInorder(node->right);
    return 0;
}

int main () {
    BSTNode* root = NULL;
    /*#ifdef DEBUG*/
    unsigned char *srcStr = (unsigned char *)"Message";
    int strLen = 8;
    unsigned char *dstStr0 = (unsigned char *)malloc(8 * sizeof(unsigned char));
    /*#endif*/
    if (create_mailbox(&root, 11) != 0) {
        printf("Error when create_mailbox root node\n");
    }
    if (root == NULL) {
        printf("Root is NULL\n");
    }
    if (create_mailbox(&root, 5) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 5) != 0) {
        printf("Passed a failure to create a new mailbox\n");
    }
    if (create_mailbox(&root, 17) != 0) {
        printf("Error when create_mailboxing one to right\n");
    }
    if (create_mailbox(&root, 5) == 0) {
        printf("Managed to add a 5 when shouldn't have\n");
    }
    if (create_mailbox(&root, 12) != 0) {
        printf("Error when create_mailboxing one to right\n");
    }
    if (create_mailbox(&root, 3) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 1) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 9) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 8) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 19) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    if (create_mailbox(&root, 15) != 0) {
        printf("Error when create_mailboxing one to left\n");
    }
    printf("Everything is going somewhat fine\n");

    printf("Run first in order print: \n");
    printInorder(root);
    if(mailbox_destroy(&root, 11) != 0) {
        printf("Something went wrong when deleting the mailbox\n");
    }
    if(mailbox_destroy(&root, 11) != 0) {
        printf("Passed failure on trying to destroy a mailbox\n");
    }
    printf("Run second in order print: \n\n");
    printInorder(root);
    if (mailbox_send(&root, 1, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    if (mailbox_send(&root, 200, srcStr, strLen) != 0) {
        printf("Passed failing to send message\n");
    }
    if (mailbox_send(&root, 1, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    if (mailbox_send(&root, 15, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    if (mailbox_send(&root, 15, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    if (mailbox_send(&root, 19, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    if (mailbox_send(&root, 19, srcStr, strLen) != 0) {
        printf("Error sending message to mailbox\n");
    }
    printf("Printing inorder after adding mail\n\n");
    printInorder(root);
    if (mailbox_destroy(&root, 19) != 0) {
        printf("Error when destroying mailbox\n");
    }
    
    if (mailbox_recv(&root, 15, &dstStr0, strLen) != strLen) {
        printf("Error when receiving message to mailbox\n");
    }
    if (mailbox_recv(&root, 15, &dstStr0, strLen) != strLen) {
        printf("Error when receiving message to mailbox\n");
    }
    if (mailbox_recv(&root, 15, &dstStr0, strLen) != strLen) {
        printf("Passed failing to receive message\n");
    }

    printf("Printing the length of the queue for an empty node and for anode that has stuff %d, %d\n", mailbox_length(&root, 1), mailbox_length(&root, 5));

    printf("Printing inorder after receiving mail\n\n");
    printInorder(root);
    
    if (message_delete(&root, 1) != 0) {
        printf("Something went wrong when we deleted the most recent message\n");
    }
    if (message_delete(&root, 1) != 0) {
        printf("Something went wrong when we deleted the most recent message\n");
    }
    if (message_delete(&root, 1) != 0) {
        printf("passed failing to delete a message\n");
    }

    printf("Printing inorder after deleting messages\n\n");
    printInorder(root);

    if (shutdown_mailbox(&root) != 0) {
        printf("Error while shutting down all mailboxes\n");
    }
    printf("Printing after shutdown: \n\n");
    printInorder(root);
    
    free(dstStr0);
    dstStr0 = NULL;

    return 0;
}   