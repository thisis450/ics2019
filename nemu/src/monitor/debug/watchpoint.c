#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static int W_id;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].last_val=0;
    wp_pool[i].now_val=0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char *args)//get a free watchpoint from the list 'free_'
{
	if (free_==NULL)
	{
		assert(0);
		return NULL;
	}
	if (head==NULL)
	{
		head=free_;
		free_=free_->next;
		head->next=NULL;
		head->NO=++W_id;
		return head;
	}
	WP *now=head;
	while (now->next!=NULL) now=now->next;
	now->next=free_;
	
	free_=free_->next;
	now->next->next=NULL;
	now->next->NO=++W_id;
	now->next->hit_num=0;
	return now->next;
}

