#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

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
sys_trace(void)
{
  //  user像kernel传递data的方式：argint
  //  kernel通过argint
    //  获取user态传入到内核的mask值
  int mask;
  int ret = argint(0,&mask);
  if(ret!=0){
    return -1;
  }

  //  获取调用sys_trace的process结构体
  //  xv6的 struct proc 类似于 linux 的 task_struct
  struct proc *cur_proc = myproc();
  if(cur_proc == 0){
    return -1;
  }

  //  设置mask 
    //  其打印效果会在上一层syscall显示出来
  cur_proc->trace_mask = mask;

  return 0;
}



uint64
sys_sysinfo(void) {
  struct sysinfo info;
  info.freemem = get_free_memory();  // 获取系统空闲内存(在kernel/kalloc.c中实现)
  info.nproc = get_unused_proc();      // 获取系统当前的进程数量(在kernel/proc.c中实现)
  info.freefd = get_unused_fd();

  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return -1;
  // printf("addr = %p\n",addr);

  struct proc *p = myproc();
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
  {
    // printf("copyout error in sys_sysinfo\n");
    // exit(-1);
    return -1;
  
  }
  return 0;
}