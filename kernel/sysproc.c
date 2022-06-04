#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void)
{
  int ticks;
  uint64 func;
  argint(0, &ticks);
  argaddr(1, &func);
  struct proc *p = myproc();
  p->sig_duration = ticks;
  p->sig_call = func;
  p->sig_count = 0;

  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();

  p->trapframe->a0 = p->saved_a[0];
  p->trapframe->a1 = p->saved_a[1];
  p->trapframe->a2 = p->saved_a[2];
  p->trapframe->a3 = p->saved_a[3];
  p->trapframe->a4 = p->saved_a[4];
  p->trapframe->a5 = p->saved_a[5];
  p->trapframe->a6 = p->saved_a[6];
  p->trapframe->a7 = p->saved_a[7];

  p->trapframe->t0 = p->saved_t[0];
  p->trapframe->t1 = p->saved_t[1];
  p->trapframe->t2 = p->saved_t[2];
  p->trapframe->t3 = p->saved_t[3];
  p->trapframe->t4 = p->saved_t[4];
  p->trapframe->t5 = p->saved_t[5];
  p->trapframe->t6 = p->saved_t[6];

  p->trapframe->sp = p->saved_sp;

  p->trapframe->s0 = p->saved_s[0];
  p->trapframe->s1 = p->saved_s[1];
  p->trapframe->s2 = p->saved_s[2];
  p->trapframe->s3 = p->saved_s[3];
  p->trapframe->s4 = p->saved_s[4];
  p->trapframe->s5 = p->saved_s[5];
  p->trapframe->s6 = p->saved_s[6];
  p->trapframe->s7 = p->saved_s[7];
  p->trapframe->s8 = p->saved_s[8];
  p->trapframe->s9 = p->saved_s[9];
  p->trapframe->s10 = p->saved_s[10];
  p->trapframe->s11 = p->saved_s[11];

  p->trapframe->ra = p->saved_ra;

  p->trapframe->epc = p->saved_epc;

  // memmove(p->trapframe, &p->saved_trapframe, sizeof(p->trapframe));
  p->sig_in = 0;
  return 0;
}