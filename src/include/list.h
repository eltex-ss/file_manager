#pragma once

#define FILE_NAME_LENGHT 79

struct List {
  char file_name[FILE_NAME_LENGHT];
  struct List *next;
};

/*  Create the linked list */
struct List* CreateList(void);

/*  Returns list's size*/
int ListSize(struct List *head);

/*  Delete the linked list */
void RemoveList(struct List *list);

/*  Add leaf by line */
void AddLeaf(const char *file_name, struct List *head);

/*  Remove leaf by name */
void RemoveLeafByNumber(int num, struct List *head);

/*  Clear the list */
void ClearList(struct List *head);
