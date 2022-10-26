#include <stdio.h>
#include <stdlib.h>

typedef struct queueNode {
    int data;
    long length; 
    struct queueNode *next;
} queueNode;

typedef struct BSTNode {
    unsigned long ID;
    struct queueNode *begin, *end;
    long queueLength;
    struct BSTNode *left, *right;
} BSTNode;

BSTNode *newBSTNode(unsigned long id) {
    BSTNode *new = (BSTNode *)malloc(sizeof(BSTNode));
    new->ID = id;
    new->begin = new->end = NULL;
    new->left = new->right = NULL;
    new->queueLength = 0;
    return new;
}

int deleteBSTNode(BSTNode *node) {
    // need to find maximum value return it
    BSTNode *temp = node->left;
    while (temp->right != NULL) {
        temp = temp->right;
    }
    node->ID = temp->ID;
    node->begin = temp->begin;
    node->end = temp->end;
    node->queueLength = temp->queueLength;
    return 0;
    // free the current node
}

int create_mailbox(BSTNode *node, unsigned long id) {
    // if node is null then creat enew node here
    if (node == NULL) {
        node = newBSTNode(id);
        return 0;
    }
    // recusrion untill you find a suitable place in tree
    if (node->ID > id) {
        return create_mailbox(node->left, id);
    } else if (node->ID < id) {
        return create_mailbox(node->right, id);
    }
    return -1;
}

int shutdown_mailbox(BSTNode *node) {
    // if null shutdown doesnt need to run
    if (node == NULL) {
        return 0;
    }
    // check children first
    if (shutdown_mailbox(node->right) == 0 && shutdown_mailbox(node->left) == 0) {
        queueNode *temp = node->begin->next;
        // while the begin is not null empty the mailbox
        while (node->begin != NULL) {
            free(node->begin);
            node->begin = temp;
            temp = temp->next;
        }
    }
    // free the node and set to null for security
    free(node);
    node = NULL;
    return 0;
}

int delete_mailbox(BSTNode *node, int id) {
    // if find a null node then return -1 because it aint there chief
    if (node == NULL) {
        return -1;
    }
    // go down each side of tree to find mailbox to delete
    if (node->ID > id) {
        delete_mailbox(node->left, id);
    } else if (node->ID < id) {
        delete_mailbox(node->right, id);
    } else {
        // run algorithm for deleting the mailbox
        BSTNode *temp = NULL;
        if (node->left == NULL) {
            temp = node;
            node = node->right;
            free(temp);
            temp = NULL;
        } else if (node->right == NULL) {
            temp = node;
            node = node->left;
            free(temp);
            temp = NULL;
        } else {
            queueNode *temp = node->begin->next;
            // while the begin is not null empty the mailbox
            while (node->begin != NULL) {
                free(node->begin);
                node->begin = temp;
                temp = temp->next;
            }
            if (deleteBSTNode(node) != 0) {
                return -1;
            }
            delete_mailbox(node->left, node->ID);
        }
    }
    return 0;
}

int main () {
    BSTNode* root = NULL;
    if (create_mailbox(root, 11) != 0) {
        printf("Error when create_mailbox root node");
    }
    if (create_mailbox(root, 5) != 0) {
        printf("Error when create_mailboxing one to left");
    }
    if (create_mailbox(root, 17) != 0) {
        printf("Error when create_mailboxing one to right");
    }
}