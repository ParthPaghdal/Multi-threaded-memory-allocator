#include "myalloc.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t lock;

struct Myalloc {
  enum allocation_algorithm aalgorithm;
  int size;
  void *memory;
  struct nodeStruct *allocatedList;
  struct nodeStruct *freeList;
};

struct Myalloc myalloc;

// Find the best node in the free list based on size
struct nodeStruct *Best_Node(struct nodeStruct **headRef, int _size) {
  struct nodeStruct *temp = *headRef;
  struct nodeStruct *best = *headRef;
  __int64_t size = _size;

  while (temp != NULL) {
    if ((*(__int64_t *)(temp->blockptr - 8)) < (*(__int64_t *)(best->blockptr - 8)) &&
        (*(__int64_t *)(temp->blockptr - 8)) >= size) {
      best = temp;
    }
    temp = temp->next;
  }
  return best;
}


// Find the worst node in the free list based on size
struct nodeStruct *Worst_Node(struct nodeStruct **headRef, int _size) {
  struct nodeStruct *temp = *headRef;
  struct nodeStruct *worst = *headRef;
  __int64_t size = _size;

  while (temp != NULL) {
    if ((*(__int64_t *)(temp->blockptr - 8)) > (*(__int64_t *)(worst->blockptr - 8)) &&
        (*(__int64_t *)(temp->blockptr - 8)) >= size) {
      worst = temp;
    }
    temp = temp->next;
  }

  return worst;
}

// Initialize the allocator
void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
  assert(_size > 0);
  pthread_mutex_init(&lock, NULL);

  myalloc.aalgorithm = _aalgorithm;

  if (myalloc.aalgorithm != FIRST_FIT && myalloc.aalgorithm != BEST_FIT &&
      myalloc.aalgorithm != WORST_FIT) {
    printf("Invalid fit type");
    exit(1);
  }

  myalloc.size = _size;
  myalloc.memory = malloc((size_t)myalloc.size);
  myalloc.allocatedList = NULL;

  // Add some other initialization
  // Set all memory to be 0
  memset(myalloc.memory, 0, _size);

  __int64_t size = _size;
  (*(__int64_t *)(myalloc.memory)) = size;
  myalloc.memory += 8;
  myalloc.freeList = List_createNode(myalloc.memory);
}

// Destroy the allocator and free allocated memory
void destroy_allocator() {
  pthread_mutex_lock(&lock);

  // Free other dynamically allocated memory to avoid memory leaks
  free(myalloc.memory - 8);

  while (myalloc.freeList != NULL) {
    struct nodeStruct *temp = myalloc.freeList;
    myalloc.freeList = myalloc.freeList->next;
    free(temp);
  }

  while (myalloc.allocatedList != NULL) {
    struct nodeStruct *temp = myalloc.allocatedList;
    myalloc.allocatedList = myalloc.allocatedList->next;
    free(temp);
  }

  pthread_mutex_unlock(&lock);
  pthread_mutex_destroy(&lock);
}

// Allocate memory of given size
void *allocate(int _size) {
  pthread_mutex_lock(&lock);

  void *cur = NULL;
  void *ptr = NULL;
  void *hptr = NULL;
  __int64_t remainingSize;

  struct nodeStruct *tempF = myalloc.freeList;

  if (myalloc.aalgorithm == FIRST_FIT) {
    // First Fit allocation algorithm
    while (tempF != NULL) {
      cur = tempF->blockptr;

      if (*((__int64_t *)(cur - 8)) >= (__int64_t)_size + 8) {
        // Allocate memory
        hptr = cur - 8;
        remainingSize = (*(__int64_t *)(hptr)) - _size - 8;
        __int64_t size = _size + 8;

        if (remainingSize > 8) {
          // Split the block into allocated and free blocks
          (*(__int64_t *)(hptr)) = size;
          struct nodeStruct *node = List_createNode(cur);
          List_insertTail(&myalloc.allocatedList, node);
          hptr += (_size + 8);
          (*(__int64_t *)(hptr)) = remainingSize;
          tempF->blockptr = hptr + 8;
        } else {
          // Use the entire block for allocation
          (*(__int64_t *)(hptr)) = size + remainingSize;
          struct nodeStruct *node = List_createNode(cur);
          List_insertTail(&myalloc.allocatedList, node);
          List_deleteNode(&myalloc.freeList, tempF);
        }

        pthread_mutex_unlock(&lock);
        return cur;
      }

      tempF = tempF->next;
    }
  } else if (myalloc.aalgorithm == BEST_FIT) {
    // Best Fit allocation algorithm
    tempF = Best_Node(&myalloc.freeList, _size);

    if (tempF == NULL) {
      pthread_mutex_unlock(&lock);
      return ptr;
    }

    cur = tempF->blockptr;
    hptr = cur - 8;
    remainingSize = (*(__int64_t *)(hptr)) - _size - 8;
    __int64_t size = _size + 8;

    if (remainingSize > 8) {
      (*(__int64_t *)(hptr)) = size;
      struct nodeStruct *node = List_createNode(cur);
      List_insertTail(&myalloc.allocatedList, node);
      hptr += (_size + 8);
      (*(__int64_t *)(hptr)) = remainingSize;
      tempF->blockptr = hptr + 8;
    } else {
      (*(__int64_t *)(hptr)) = size + remainingSize;
      struct nodeStruct *node = List_createNode(cur);
      List_insertTail(&myalloc.allocatedList, node);
      List_deleteNode(&myalloc.freeList, tempF);
    }

    pthread_mutex_unlock(&lock);
    return cur;
    } else if (myalloc.aalgorithm == WORST_FIT) {
    // Worst Fit allocation algorithm
    tempF = Worst_Node(&myalloc.freeList, _size);

    if (tempF == NULL) {
      pthread_mutex_unlock(&lock);
      return ptr;
    }

    cur = tempF->blockptr;
    hptr = cur - 8;
    remainingSize = (*(__int64_t *)(hptr)) - _size - 8;
    __int64_t size = _size + 8;

    if (remainingSize > 8) {
      (*(__int64_t *)(hptr)) = size;
      struct nodeStruct *node = List_createNode(cur);
      List_insertTail(&myalloc.allocatedList, node);
      hptr += (_size + 8);
      (*(__int64_t *)(hptr)) = remainingSize;
      tempF->blockptr = hptr + 8;
    } else {
      (*(__int64_t *)(hptr)) = size + remainingSize;
      struct nodeStruct *node = List_createNode(cur);
      List_insertTail(&myalloc.allocatedList, node);

      List_deleteNode(&myalloc.freeList, tempF);
    }

    pthread_mutex_unlock(&lock);
    return cur;
  }
  pthread_mutex_unlock(&lock);
  return ptr;
}

void deallocate(void *_ptr) {
  pthread_mutex_lock(&lock);

  if (_ptr == NULL) {
    pthread_mutex_unlock(&lock);
    return;
  }

  struct nodeStruct *tempD = List_findNode(myalloc.allocatedList, _ptr);
  List_deleteNode(&myalloc.allocatedList, tempD);

  struct nodeStruct *tempN = List_createNode(_ptr);
  List_insertTail(&myalloc.freeList, tempN);

  memset(_ptr, 0, *(__int64_t *)(_ptr - 8) - 8);

  struct nodeStruct *temp_a = myalloc.freeList;
  bool contiguous = false;

  while (temp_a != NULL && temp_a->next != NULL) {
    __int64_t a_size = *(__int64_t *)(temp_a->blockptr - 8);
    contiguous = false;

    if (temp_a->blockptr + a_size == temp_a->next->blockptr) {
      contiguous = true;
    }

    if (contiguous) {
      // Merge contiguous free blocks
      a_size += *(__int64_t *)(temp_a->next->blockptr - 8);
      *(__int64_t *)(temp_a->blockptr - 8) = a_size;
      memset(temp_a->next->blockptr - 8, 0, 8);
      List_deleteNode(&myalloc.freeList, temp_a->next);
    }

    temp_a = temp_a->next;
  }

  pthread_mutex_unlock(&lock);
}

int compact_allocation(void **_before, void **_after) {
  pthread_mutex_lock(&lock);
  int compacted_size = 0;

  // compact allocated memory
  // update _before, _after and compacted_size

  pthread_mutex_unlock(&lock);
  return compacted_size;
}

int available_memory() {
  int available_memory_size = 0;

  // Calculate available memory size
  struct nodeStruct *tempA = myalloc.freeList;
  void *cur;
  void *hptr;

  while (tempA != NULL) {
    cur = tempA->blockptr;
    hptr = cur - 8;
    available_memory_size += *(__int64_t *)(hptr)-8;
    tempA = tempA->next;
  }

  return available_memory_size;
}

void get_statistics(struct Stats *_stat) {
  pthread_mutex_lock(&lock);

  // Populate struct Stats with the statistics

  int num_allocated_chunks = 0;
  struct nodeStruct *cur = myalloc.allocatedList;

  while (cur != NULL) {
    num_allocated_chunks++;
    cur = cur->next;
  }

  _stat->allocated_chunks = num_allocated_chunks;

  int num_free_chunks = 0;
  cur = myalloc.freeList;

  while (cur != NULL) {
    num_free_chunks++;
    cur = cur->next;
  }

  _stat->free_size = available_memory();

  _stat->allocated_size =
      myalloc.size - available_memory() - (num_allocated_chunks * 8) - 8;

  _stat->free_chunks = num_free_chunks;

  int smallest_free_chunk = myalloc.size;
  cur = myalloc.freeList;

  if (cur != NULL) {
    smallest_free_chunk = *(__int64_t *)(myalloc.freeList->blockptr - 8);

    while (cur != NULL) {
      if (*((__int64_t *)(cur->blockptr - 8)) < smallest_free_chunk) {
        smallest_free_chunk = *((__int64_t *)(cur->blockptr - 8));
      }
      cur = cur->next;
    }
  }

  smallest_free_chunk -= 8;

  _stat->smallest_free_chunk_size = smallest_free_chunk;

  int largest_free_chunk = 0;
  cur = myalloc.freeList;

  if (cur != NULL) {
    largest_free_chunk = *(__int64_t *)(myalloc.freeList->blockptr);

    while (cur != NULL) {
      if (*((__int64_t *)(cur->blockptr - 8)) > largest_free_chunk) {
        largest_free_chunk = *((__int64_t *)(cur->blockptr - 8));
      }
      cur = cur->next;
    }
  }
  largest_free_chunk -= 8;

  _stat->largest_free_chunk_size = largest_free_chunk;
  
  pthread_mutex_unlock(&lock);
}

/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node.
 */
struct nodeStruct *List_createNode(void *blockptr) {
  struct nodeStruct *temp = NULL;
  temp = malloc(sizeof(struct nodeStruct));
  temp->blockptr = blockptr;
  temp->next = NULL;
  return temp;
}

void List_insertHead(struct nodeStruct **headRef, struct nodeStruct *node) {
  node->next = *headRef;
  *headRef = node;
};

/*
 * Insert node after the tail of the list.
 */
void List_insertTail(struct nodeStruct **headRef, struct nodeStruct *node) {
  if (*headRef == NULL) {
    *headRef = node;
  } else {
    struct nodeStruct *lastNode = *headRef;
    while (lastNode->next != NULL) {
      lastNode = lastNode->next;
    }
    lastNode->next = node;
  }
};

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes(struct nodeStruct *head) {
  if (head == NULL) {
    return 0;
  } else {
    int count = 1;
    struct nodeStruct *lastNode = head;
    while (lastNode->next != NULL) {
      lastNode = lastNode->next;
      count++;
    }
    return count;
  }
};

/*
 * Return the first node holding the value item, return NULL if none found
 */
struct nodeStruct *List_findNode(struct nodeStruct *head, void *blockptr) {
  struct nodeStruct *searchNode = head;
  while (searchNode != NULL) {
    if (searchNode->blockptr == blockptr) {
      return searchNode;
    }
    searchNode = searchNode->next;
  }
  return NULL;
};

void List_sort(struct nodeStruct **head) {
  struct nodeStruct *temp = *head;
  struct nodeStruct *traversal = NULL;
  void *a;
  if (head == NULL)
    return;
  for (; temp != NULL; temp = temp->next) {
    traversal = temp->next;
    for (; traversal != NULL; traversal = traversal->next) {
      if (temp->blockptr > traversal->blockptr) {
        a = traversal->blockptr;
        traversal->blockptr = temp->blockptr;
        temp->blockptr = a;
      }
    }
  }
}

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set to a valid node
 * in the list. For example, the client code may have found it by calling
 * List_findNode(). If the list contains only one node, the head of the list
 * should be set to NULL.
 */
void List_deleteNode(struct nodeStruct **headRef, struct nodeStruct *node) {
  struct nodeStruct *tmp;
  if (*headRef == node) {
    tmp = *headRef;
    *headRef = (*headRef)->next;
    free(tmp);
  } else {
    struct nodeStruct *currentNode = *headRef;
    while (currentNode->next != NULL) {
      if (currentNode->next == node) {
        tmp = currentNode->next;
        currentNode->next = currentNode->next->next;
        free(tmp);
        break;
      } else {
        currentNode = currentNode->next;
      }
    }
  }
};
