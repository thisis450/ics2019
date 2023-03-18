#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char exp[128];
  uint32_t old_val;
  int hit_num;


} WP;
WP* new_wp(char *args);
void free_wp(int N);
bool check_wp();
#endif
