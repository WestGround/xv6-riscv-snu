// Sleeping locks

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->prio = PRIO_INIT;
  lk->firstblocked = 1;
}

void
acquiresleep(struct sleeplock *lk)
{
  struct proc *p = myproc();
  int prio = PRIO_INIT;

  acquire(&lk->lk);
  while (lk->locked) {
    if((prio = kthread_get_prio_with_pid(lk->pid)) < 0)
      panic("acquire sleeplock");
    if(prio > p->prio) {
      kthread_set_prio_with_pid(lk->pid, p->prio);
    }
    if(lk->firstblocked) {
      lk->firstblocked = 0;
      lk->prio = prio;
    }
    sleep(lk, &lk->lk);
  }

  lk->locked = 1;
  lk->pid = p->pid;
  lk->prio = p->prio; //save original priority
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  struct proc *p = myproc();

  acquire(&lk->lk);
  p->prio = lk->prio; //restore priority
  lk->firstblocked = 1;
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



