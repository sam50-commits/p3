#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
  return kkill(pid);
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
sys_getmemstats(void)
{
  uint64 magic_addr, strategy_addr, num_alloc_addr, total_alloc_addr, num_free_addr;
  
  // Get arguments (pointers to user space)
  argaddr(0, &magic_addr);
  argaddr(1, &strategy_addr);
  argaddr(2, &num_alloc_addr);
  argaddr(3, &total_alloc_addr);
  argaddr(4, &num_free_addr);
  
  uint magic, strategy, num_alloc, total_alloc, num_free;
  
  // Get stats from kernel allocator
  student_get_stats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  
  // Copy values to user space
  struct proc *p = myproc();
  if(copyout(p->pagetable, magic_addr, (char*)&magic, sizeof(uint)) < 0) 
    return -1; // if copying magic number fails, return -1
  if(copyout(p->pagetable, strategy_addr, (char*)&strategy, sizeof(uint)) < 0)
    return -1;
  if(copyout(p->pagetable, num_alloc_addr, (char*)&num_alloc, sizeof(uint)) < 0)
    return -1;
  if(copyout(p->pagetable, total_alloc_addr, (char*)&total_alloc, sizeof(uint)) < 0)
    return -1;
  if(copyout(p->pagetable, num_free_addr, (char*)&num_free, sizeof(uint)) < 0)
    return -1;
  
  return 0;
}

uint64
sys_student_malloc(void)
{
  uint size;
  
  // Get size argument
  argint(0, (int*)&size);
  
  // Call kernel allocator
  void* ptr = student_malloc(size);
  
  // Return pointer as uint64 (0 if allocation failed)
  return (uint64)ptr;
}

uint64
sys_student_free(void)
{
  uint64 ptr_addr;
  
  // Get pointer argument
  argaddr(0, &ptr_addr);
  
  // Call kernel free function
  student_free((void*)ptr_addr);
  
  return 0;
}
