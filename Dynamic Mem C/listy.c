/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ics.h"
#include "emalloc.h"
#include "listy.h"


node_t *new_node(event_t *val) {
    assert( val != NULL);

    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->val = val;
    temp->next = NULL;

    return temp;
}


node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}


node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}

int event_comparator(node_t *n1, node_t *n2) {
    return strcmp(n1->val->dtstart, n2->val->dtstart);
}

node_t* add_inorder(node_t *list, node_t *new) {
    if (list == NULL) {
        list = add_front(list, new);
    }

    else {
        node_t* prev = NULL;
        node_t* curr = list;
          while(curr != NULL){
            if (event_comparator(new, curr) > 0) {
                prev = curr;
                curr = curr->next;
            }
            else {
                break;
            }
        }
        new->next = curr;

        if (prev == NULL) {
            return new;
        }
        prev->next = new;
    }
    return list;
}



node_t *peek_front(node_t *list) {
    return list;
}


node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }

    return list->next;
}



void apply(node_t *list,
           void (*fn)(node_t *list, void *),
           void *arg)
{
    for ( ; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}
