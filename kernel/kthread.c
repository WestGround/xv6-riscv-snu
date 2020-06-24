//--------------------------------------------------------------------
//
//  4190.307: Operating Systems (Spring 2020)
//
//  PA#6: Kernel Threads
//
//  June 2, 2020
//
//  Jin-Soo Kim
//  Systems Software and Architecture Laboratory
//  Department of Computer Science and Engineering
//  Seoul National University
//
//--------------------------------------------------------------------

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "defs.h"

#ifdef SNU
void kthreadret(void) {
  struct proc *p = myproc();
  release(&p->lock);
  p->fn((void*)p->arg);
  return;
}

int 
kthread_create(const char *name, int prio, void (*fn)(void *), void *arg)
{
  struct proc *p;
  int tid;

  if((p = allockthread(fn, arg)) == 0)
    return -1;

  tid = p->pid;
  safestrcpy(p->name, name, sizeof(name));
  p->prio = prio;
  p->context.ra = (uint64)kthreadret;
  p->fn = fn;
  p->arg = arg;
  release(&p->lock);
  if(prio < myproc()->prio)
    yield();

  return tid;
}

void 
kthread_exit(void)
{
  struct proc *p = myproc();

  acquire(&p->lock);
  p->tf = 0;
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->prio = -1;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
  sched();

  panic("kthread_exit");
}

void
kthread_yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

void
kthread_set_prio(int newprio)
{
  struct proc *p = myproc();
  int oldprio;

  acquire(&p->lock);
  oldprio = p->prio;
  p->prio = newprio;
  if(oldprio < newprio) {
    p->state = RUNNABLE;
    sched();
  }
  release(&p->lock);
  return;
}

int
kthread_get_prio(void)
{
  return myproc()->prio;
}
#endif

