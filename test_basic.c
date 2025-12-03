#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("=== Basic Memory Allocator Test ===\n\n");
  
  // Variables to store statistics
  unsigned int magic, strategy, num_alloc, total_alloc, num_free;
  
  // Test 1: Initial statistics (should be initialized with free pages)
  printf("Test 1: Initial Statistics\n");
  if(getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free) == 0) {
    printf("  Magic Number: %d (Expected: 16)\n", magic);
    printf("  Allocation Strategy: %d (1=best-fit)\n", strategy);
    printf("  Allocated Blocks: %d\n", num_alloc);
    printf("  Total Memory Allocated: %d bytes\n", total_alloc);
    printf("  Free Blocks: %d\n", num_free);
    
    if(magic == 16) {
      printf("  ✓ Magic number verified!\n");
    } else {
      printf("  ✗ Magic number mismatch!\n");
      printf("\n");
    }
  } else {
    printf("  ✗ getmemstats() failed\n");
    exit(1);
  }
  printf("\n");
  
  // Test 2: Allocate blocks of different sizes
  printf("Test 2: Allocating blocks of different sizes\n");
  void *ptr1 = student_malloc(64);
  printf("  Allocated 64 bytes at %p\n", ptr1);
  
  void *ptr2 = student_malloc(128);
  printf("  Allocated 128 bytes at %p\n", ptr2);
  
  void *ptr3 = student_malloc(256);
  printf("  Allocated 256 bytes at %p\n", ptr3);
  
  void *ptr4 = student_malloc(512);
  printf("  Allocated 512 bytes at %p\n", ptr4);
  
  void *ptr5 = student_malloc(1024);
  printf("  Allocated 1024 bytes at %p\n", ptr5);
  
  // Check statistics after allocation
  if(getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free) == 0) {
    printf("\n  Statistics after allocation:\n");
    printf("    Allocated Blocks: %d (Expected: 5)\n", num_alloc);
    printf("    Total Memory Allocated: %d bytes (Expected: %d)\n", 
           total_alloc, 64+128+256+512+1024);
    printf("    Free Blocks: %d\n", num_free);
  }
  printf("\n");
  
  // Test 3: Free some blocks
  printf("Test 3: Freeing some blocks\n");
  printf("  Freeing ptr2 (128 bytes)\n");
  student_free(ptr2);
  
  printf("  Freeing ptr4 (512 bytes)\n");
  student_free(ptr4);
  
  // Check statistics after freeing
  if(getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free) == 0) {
    printf("\n  Statistics after freeing:\n");
    printf("    Allocated Blocks: %d (Expected: 3)\n", num_alloc);
    printf("    Total Memory Allocated: %d bytes (Expected: %d)\n", 
           total_alloc, 64+256+1024);
    printf("    Free Blocks: %d\n", num_free);
  }
  printf("\n");
  
  // Test 4: Allocate more blocks
  printf("Test 4: Allocating additional blocks\n");
  void *ptr6 = student_malloc(100);
  printf("  Allocated 100 bytes at %p\n", ptr6);
  
  void *ptr7 = student_malloc(200);
  printf("  Allocated 200 bytes at %p\n", ptr7);
  
  // Check statistics
  if(getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free) == 0) {
    printf("\n  Statistics after more allocations:\n");
    printf("    Allocated Blocks: %d (Expected: 5)\n", num_alloc);
    printf("    Total Memory Allocated: %d bytes\n", total_alloc);
    printf("    Free Blocks: %d\n", num_free);
  }
  printf("\n");
  
  // Test 5: Free all remaining blocks to prevent memory leak
  printf("Test 5: Freeing all remaining blocks\n");
  student_free(ptr1);
  student_free(ptr3);
  student_free(ptr5);
  student_free(ptr6);
  student_free(ptr7);
  printf("  All blocks freed\n");
  
  // Final statistics
  if(getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free) == 0) {
    printf("\n  Final Statistics:\n");
    printf("    Magic Number: %d\n", magic);
    printf("    Allocation Strategy: %d\n", strategy);
    printf("    Allocated Blocks: %d (Expected: 0)\n", num_alloc);
    printf("    Total Memory Allocated: %d bytes (Expected: 0)\n", total_alloc);
    printf("    Free Blocks: %d\n", num_free);
    
    if(num_alloc == 0 && total_alloc == 0) {
      printf("  ✓ No memory leaks!\n");
    } else {
      printf("  ✗ Memory leak detected!\n");
    }
  }
  printf("\n");
  
  printf("=== Test Complete ===\n");
  
  exit(0);
}
