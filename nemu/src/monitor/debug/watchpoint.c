#include "watchpoint.h"
#include "expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *newWP(void)
{
  if(!free_)
    return NULL;
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

void freeWP(int n)
{
  bool exist = false;
  if(head){  // Not empty
    if(head->next==NULL){  // Only one node
      if(head->NO == n){
        exist = true;
        head->next = free_;
        free_ = head;
        head = NULL;
      }
    }
    else{
      if(head->NO == n){ // delete the head node
        exist = true;
        WP *wp = head;
        head = head->next;
        wp->next = free_;
        free_ = wp;
      }
      else{
        WP *pre = head;
        WP *wp = head->next;
        while(wp){
          if(wp->NO == n){
            exist = true;
            pre->next = wp->next;
            wp->next = free_;
            free_ = wp;
            break;
          }
          pre = wp;
          wp = wp->next;
        }
      }
    }
  }
  if(exist)
    printf("WatchPoint %d has been deleted successfully.\n",n);
  else
    printf("WatchPoint %d doesn't exist, please run \"info w\" to display current WatchPoints.\n",n);
}

void DisplayWP(void)
{
  if(head==NULL){
    printf("No WatchPoints currently.\n");
    return ;
  }
  printf("  NO  expression              value\n");
  WP *wp = head;
  while(wp){
    printf("  %d    %s                   0x%x\n",wp->NO, wp->expression, wp->oldvalue);
    wp = wp->next;
  }
}

bool triggerWP(void)
{
  WP *wp = head;
  word_t v;
  bool trigger = false;
  bool suc;
  while(wp){
    v = expr(wp->expression, &suc);
    if(v != wp->oldvalue){
      wp->oldvalue = v;
      trigger = true;
      printf("WatchPoint %d has been triggered\n",wp->NO);
    }
    wp = wp->next;
  }
  return trigger;
}