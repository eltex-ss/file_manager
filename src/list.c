#include "list.h"

#include <stdlib.h>
#include <string.h>

/*  Create the linked list */
struct List* CreateList(void)
{
  struct List *head = malloc(sizeof(struct List));
  head->next = NULL;
  return head;
}

int ListSize(struct List *head)
{
  int size = 0;
  
  head = head->next;
  while (head) {
    ++size;
    head = head->next;
  }
  return size;
}

/*  Delete the linked list */
void RemoveList(struct List *list)
{
  ClearList(list);
  free(list);
  list = NULL;
}

/*  Add leaf by file_name */
void AddLeaf(const char *file_name, struct List *head)
{
  struct List *leaf = CreateList();
  strcpy(leaf->file_name, file_name);
  leaf->next = head->next;
  head->next = leaf;
}

/*  Remove leaf by name */
void RemoveLeafByName(int num, struct List *head)
{
  struct List *current_leaf = head->next;
  struct List *prev_leaf = head;
  int current_line = 0;

  for (current_line = 0; current_line != num; ++current_line) {
    prev_leaf = current_leaf;
    current_leaf = current_leaf->next;
  }
  prev_leaf->next = current_leaf->next;
  free(current_leaf);
  current_leaf = NULL;
}

/*  Clear the list */
void ClearList(struct List *head)
{
  struct List **currentLeaf = &head->next;
  
  struct List *leafToDelete = NULL;
  while (*currentLeaf) {
    leafToDelete = *currentLeaf;
    *currentLeaf = (*currentLeaf)->next;

    free(leafToDelete);
    leafToDelete = NULL;
  }
}
