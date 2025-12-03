// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// Custom allocator definitions
#define DEFAULT_BLOCK_SIZE 768
#define ALLOCATION_STRATEGY 1  // 1 = best-fit
#define ALIGNMENT 3 // 8-byte alignment, i.e., 2^3 = 8, power-of-2 alignments for memory
#define FREE_LIST_SIZE 20
#define MAGIC_NUMBER 16

struct block_header {
  uint size;        // User's requested size
  uint magic;       // Magic number (16)
  uint allocated;   // 1 if allocated, 0 if free
  struct block_header* next; // Next block in free list
};

struct { // Student memory allocator state
  struct spinlock lock;
  struct block_header* freelist;
  uint num_allocated;
  uint total_allocated;
  uint num_free;
  int initialized;
} student_mem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Round up size to alignment boundary
static uint
round_up(uint size)
{
  uint align = (1 << ALIGNMENT); // 8-byte alignment, left bit shift 
  return (size + align - 1) & ~(align - 1); //round up to nearest multiple of align
}

// Initialize custom student allocator
void
student_init()
{
  if(student_mem.initialized)
    return; // Already initialized
  
  initlock(&student_mem.lock, "student_mem"); // Initialize lock
  student_mem.freelist = 0;
  student_mem.num_allocated = 0;
  student_mem.total_allocated = 0;
  student_mem.num_free = 0;
  
  // Pre-allocate FREE_LIST_SIZE pages
  for(int i = 0; i < FREE_LIST_SIZE; i++) { //20 pages
    void* page = kalloc();
    if(page == 0) //kalloc failed
      break;
    
    // Initialize block header
    struct block_header* block = (struct block_header*)page;
    block->size = 0;
    block->magic = MAGIC_NUMBER;
    block->allocated = 0;
    block->next = student_mem.freelist;
    student_mem.freelist = block;
    student_mem.num_free++;
  }
  
  student_mem.initialized = 1; // Mark as initialized
}

// Allocate memory using custom allocator
void*
student_malloc(uint size)
{
   // 1. Initialize if first call
  if(!student_mem.initialized)
    student_init();
  
  //if allocating 0 bytes then return 0
  if(size == 0)
    return 0;
  
  uint aligned_size = round_up(size);
  
  //getting the lock, to avoid race conditions
  acquire(&student_mem.lock);
  
  // Find best-fit block
  struct block_header* best = 0; // 0 initialy because no block selected yet
  struct block_header* curr = student_mem.freelist; //starting at the head of the free list
  
  // Best-fit strategy: find smallest suitable block
  //since all blocks are of size PGSIZE, I just need to find a free block
  if(ALLOCATION_STRATEGY == 1) { // 1 = best-fit
    while(curr) { //traverse the free list
      if(!curr->allocated) { //if block is free
        uint available = PGSIZE - sizeof(struct block_header); //available space in block
        //   block is big enough , which is always true since all blocks are same size
        if(aligned_size <= available ) {
          best = curr; //update best
          break; //found suitable block, done
        }
      }
      curr = curr->next;
    }
  }
  
  // If no suitable block found, allocate new page
  if(best == 0) {
    void* page = kalloc(); //get new page
    if(page == 0) { //kalloc failed
      release(&student_mem.lock);
      return 0;
    }
    
    best = (struct block_header*)page; //set best to new page
    best->magic = MAGIC_NUMBER;
    best->allocated = 0; //mark as free
    best->next = student_mem.freelist; // i decided to add new block at head of free list, 
    student_mem.freelist = best; ////becuase it is O(1) at the head, update head to new block
  }
  
  // Now mark block as allocated
  best->size = size;
  best->allocated = 1;
  student_mem.num_allocated++;
  student_mem.total_allocated += size; // track total mem allocated size
  
  // Update free count
  student_mem.num_free = 0;  // Reset counter to 0, will recount below, how many free blocks there are
  curr = student_mem.freelist; //starting at head of free list
  while(curr) {
    if(!curr->allocated)
      student_mem.num_free++; 
    curr = curr->next;
  }
  
  release(&student_mem.lock); // Unlock (done modifying)

  //at first i though to return 
  //return (void*)best;  
  // but this does not work because it returns pointer to header, not to user data
  
  // Return pointer after header
  return (void*)((char*)best + sizeof(struct block_header));
}

// Free memory allocated by student_malloc
void
student_free(void* ptr)
{
  if(ptr == 0)
    return; //nothing to free
  
  if(!student_mem.initialized)
    return; // Allocator not initialized, nothing to free
  
  // Get block header
  struct block_header* block = (struct block_header*)((char*)ptr - sizeof(struct block_header));
  
  acquire(&student_mem.lock); // Lock for thread safety
  
  // Verify magic number
  if(block->magic != MAGIC_NUMBER) {
    release(&student_mem.lock);
    panic("student_free: bad magic number");
  }
  
  if(!block->allocated) { // prevent Double freeing becuase that would be pointless
    release(&student_mem.lock);
    panic("student_free: double free");
  }
  
  // Mark as free
  student_mem.total_allocated -= block->size; // Update total allocated size
  student_mem.num_allocated--; // Decrement allocated count
  block->allocated = 0;
  block->size = 0;
  student_mem.num_free++; // Increment free count
  
  release(&student_mem.lock);
}


// Get detailed memory statistics for system call
// decided to modify the signature as i see fit for student stats
void
student_get_stats(uint* magic, uint* strategy, uint* num_alloc, uint* total_alloc, uint* num_free)
{
  if(!student_mem.initialized) // Initialize if not done yet
    student_init();
  
  acquire(&student_mem.lock);
  
  *magic = MAGIC_NUMBER;
  *strategy = ALLOCATION_STRATEGY;
  *num_alloc = student_mem.num_allocated; //number of currently allocated blocks
  *total_alloc = student_mem.total_allocated; //total size of all allocated blocks
  *num_free = student_mem.num_free;
  
  release(&student_mem.lock);
}
