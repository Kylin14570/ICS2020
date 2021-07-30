#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expression[128];
  word_t oldvalue;

} WP;

WP *newWP(void);
void freeWP(int n);
void DisplayWP(void);
bool triggerWP(void);

#endif
