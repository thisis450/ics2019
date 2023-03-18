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
    wp_pool[i].old_val=0;
    wp_pool[i].hit_num=0;
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
    Log("没有足够的监视点\n");
		assert(0);
		return NULL;
	}
  bool success=true;
  uint32_t val=expr(args,&success);
  if(success=false)
  {
    Log("监视点设置时表达式计算错误\n");
    val=0;
    return NULL;
  }
	if (head==NULL)
	{
		head=free_;
		free_=free_->next;
		head->next=NULL;
		head->NO=++W_id;
    head->hit_num=0;
    strcpy(head->exp,args);
    head->old_val=val;
		return head;
	}
	WP *now=head;
	while (now->next!=NULL) now=now->next;
	now->next=free_;

	
	free_=free_->next;
	now->next->next=NULL;
	now->next->NO=++W_id;
	now->next->hit_num=0;
  strcpy(now->next->exp,args);
  now->next->old_val=val;
	return now->next;
}
void free_wp(int N)//make wp free and return to the list 'free_'
{
	WP *now=head,*wp;	
	if (now!=NULL&&now->NO==N)
	{
		head=head->next;
		now->next=free_;
		free_=now;
		return;
	}
	while (now!=NULL&&now->next->NO!=N) now=now->next;
	if (now==NULL)return;
  if(now->NO!=N)
  {
    printf("没有编号为%d的监视点\n",N);
    return;
  }
	wp=now->next;
	now->next=wp->next;
	wp->next=free_;
	free_=wp;
}
bool check_wp()
{
	bool _suc,change=0;
  uint32_t now_val;
	for (WP *now=head;now!=NULL;now=now->next)
	{
       now_val=expr(now->exp,&_suc);
       if (now->old_val!=now_val) 
       {
        now->hit_num++;
        change=1;
        printf("监视点%d，表达式%s的值由%u变为了%u\n",now->NO,now->exp,now->old_val,now_val);
        now->old_val=now_val;
       }
       else
       {
  //printf("监视点%d，表达式%s的值为%u，未改变\n",now->NO,now->exp,now->old_val);
       }
       
	}
	return change;
}
void print_wp()
{
  for (WP*now=head;now!=NULL;now=now->next)
  {
    printf("监视点%d, 表达式为%s,  值为%u, hitnum为%d\n",now->NO,now->exp,now->old_val,now->hit_num);
  }
};