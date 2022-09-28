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


// uint64 
// sys_sysinfo(void)
// {
//   struct sysinfo info;
//   // info.nproc = get_unused_proc();     //  获取系统中unused的process
//   // info.freemem = get_free_memory();   //  获取系统中剩余的内存
//   // info.freefd = get_unused_fd();      //  获取进程仍可以使用的fd数量

//   //  get addr pass from user
//   uint64 addr;
//   if(argaddr(0,&addr) < 0)
//   {
//     printf("argaddr error in sys_sysinfo!\n");
//     return -1;
//   }

//   //  pass data from kernel to user addr
//   // struct proc *p = myproc();
//   if(copyout(myproc()->pagetable,addr,(char*)&info,sizeof info) < 0)
//   {
//     printf("copyout error in sys_sysinfo\n");
//     return -1;
//   }
//   return 0;
// }
// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.



// uint64
// sys_sysinfo(void)
// {
//   // 从用户态读入一个指针，作为存放 sysinfo 结构的缓冲区
//   uint64 addr;
//   if(argaddr(0, &addr) < 0)
//     return -1;
  
//   struct sysinfo sinfo;
//   // sinfo.freemem = count_free_mem(); // kalloc.c
//   // sinfo.nproc = count_process(); // proc.c
  
//   // 使用 copyout，结合当前进程的页表，获得进程传进来的指针（逻辑地址）对应的物理地址
//   // 然后将 &sinfo 中的数据复制到该指针所指位置，供用户进程使用。
//   if(copyout(myproc()->pagetable, addr, (char *)&sinfo, sizeof(sinfo)) < 0)
//   {
//     printf("copyout error in sys_sysinfo\n");
//     return -1;
//   }
//   return 0;
// }

uint64
sys_sysinfo(void) {
  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return -1;

  // printf("addr = %p\n",addr);

  struct sysinfo info;
  
  info.freemem = get_free_memory();  // 获取系统空闲内存(在kernel/kalloc.c中实现)
  info.nproc = get_unused_proc();      // 获取系统当前的进程数量(在kernel/proc.c中实现)
  info.freefd = get_unused_fd();
  struct proc *p = myproc();
  
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
  {
    // printf("copyout error in sys_sysinfo\n");
    // exit(-1);
    return -1;
  
  }
  return 0;
}