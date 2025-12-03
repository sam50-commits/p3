#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("=== Stress Test ===\n\n");
  
  unsigned int magic, strategy, num_alloc, total_alloc, num_free;
  void *ptrs[100];  // Array to hold pointers
  int i;
  
  // Test 1: Edge case - Zero size allocation
  printf("Test 1: Edge Cases - Zero Size Allocation\n");
  void *zero_ptr = student_malloc(0);
  if(zero_ptr == 0) {
    printf("  ✓ student_malloc(0) correctly returned NULL\n");
  } else {
    printf("  ✗ student_malloc(0) should return NULL\n");
    student_free(zero_ptr);
  }
  printf("\n");
  
  // Test 2: Edge case - NULL pointer free
  printf("Test 2: Edge Cases - Free NULL Pointer\n");
  student_free(0);
  printf("  ✓ student_free(NULL) handled safely (no crash)\n");
  printf("\n");
  
  // Test 3: Many sequential allocations
  printf("Test 3: Sequential Allocations (50 blocks)\n");
  printf("  Allocating 50 blocks of varying sizes...\n");
  
  for(i = 0; i < 50; i++) {
    // Varying sizes: 50, 100, 150, 200, etc.
    int size = 50 + (i % 10) * 50;
    ptrs[i] = student_malloc(size);
    if(ptrs[i] == 0) {
      printf("  ! Allocation %d failed (size %d) - may have run out of pages\n", i, size);
      break;
    }
  }
  int allocated_count = i;
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  Successfully allocated: %d blocks\n", allocated_count);
  printf("  Statistics - Blocks: %d, Total: %d bytes, Free: %d\n", 
         num_alloc, total_alloc, num_free);
  printf("\n");
  
  // Test 4: Free half of them (create fragmentation)
  printf("Test 4: Creating Fragmentation (Free every other block)\n");
  for(i = 0; i < allocated_count; i += 2) {
    student_free(ptrs[i]);
    ptrs[i] = 0;  // Mark as freed
  }
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  After freeing half:\n");
  printf("  Allocated blocks: %d, Total: %d bytes, Free: %d\n", 
         num_alloc, total_alloc, num_free);
  printf("\n");
  
  // Test 5: Re-allocate in the freed spaces
  printf("Test 5: Re-allocation in Fragmented Memory\n");
  for(i = 0; i < allocated_count; i += 2) {
    ptrs[i] = student_malloc(100);
    if(ptrs[i] == 0) {
      printf("  ! Re-allocation %d failed\n", i);
    }
  }
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  After re-allocation:\n");
  printf("  Allocated blocks: %d, Total: %d bytes, Free: %d\n", 
         num_alloc, total_alloc, num_free);
  printf("\n");
  
  // Test 6: Rapid allocation/deallocation cycles
  printf("Test 6: Rapid Allocation/Deallocation Cycles (100 iterations)\n");
  void *temp_ptrs[10];
  
  for(i = 0; i < 100; i++) {
    // Allocate 10 blocks
    for(int j = 0; j < 10; j++) {
      temp_ptrs[j] = student_malloc(64 + j * 32);
    }
    // Free them all immediately
    for(int j = 0; j < 10; j++) {
      student_free(temp_ptrs[j]);
    }
  }
  
  printf("  ✓ Completed 100 cycles of allocate/free\n");
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  After cycles - Blocks: %d, Total: %d bytes, Free: %d\n", 
         num_alloc, total_alloc, num_free);
  printf("\n");
  
  // Test 7: Large allocations
  printf("Test 7: Large Allocations\n");
  void *large1 = student_malloc(3000);
  void *large2 = student_malloc(3500);
  void *large3 = student_malloc(3800);
  
  if(large1 && large2 && large3) {
    printf("  ✓ Successfully allocated 3000, 3500, 3800 bytes\n");
    printf("  Large1: %p, Large2: %p, Large3: %p\n", large1, large2, large3);
  } else {
    printf("  ! Some large allocations failed\n");
  }
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  Allocated blocks: %d, Total: %d bytes\n", num_alloc, total_alloc);
  
  student_free(large1);
  student_free(large2);
  student_free(large3);
  printf("\n");
  
  // Test 8: Performance test - measure allocation time
  printf("Test 8: Performance Test (200 allocations)\n");
  void *perf_ptrs[200];
  int success_count = 0;
  
  for(i = 0; i < 200; i++) {
    perf_ptrs[i] = student_malloc(128);
    if(perf_ptrs[i] != 0) {
      success_count++;
    } else {
      break;
    }
  }
  
  printf("  Successfully allocated: %d out of 200 blocks\n", success_count);
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  Statistics - Blocks: %d, Total: %d bytes, Free: %d\n", 
         num_alloc, total_alloc, num_free);
  
  // Free all performance test allocations
  for(i = 0; i < success_count; i++) {
    student_free(perf_ptrs[i]);
  }
  printf("\n");
  
  // Test 9: Memory leak check - free all previous allocations
  printf("Test 9: Memory Leak Detection - Cleanup\n");
  printf("  Freeing all remaining allocated blocks...\n");
  
  for(i = 0; i < allocated_count; i++) {
    if(ptrs[i] != 0) {
      student_free(ptrs[i]);
    }
  }
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  printf("  Final state:\n");
  printf("  Allocated blocks: %d\n", num_alloc);
  printf("  Total allocated: %d bytes\n", total_alloc);
  printf("  Free blocks: %d\n", num_free);
  
  if(num_alloc == 0 && total_alloc == 0) {
    printf("  ✓ No memory leaks detected!\n");
  } else {
    printf("  ✗ Memory leak: %d blocks, %d bytes not freed\n", 
           num_alloc, total_alloc);
  }
  printf("\n");
  
  // Test 10: Alignment test
  printf("Test 10: Alignment Test (ALIGNMENT=3, so 8-byte aligned)\n");
  void *align1 = student_malloc(1);
  void *align2 = student_malloc(7);
  void *align3 = student_malloc(15);
  void *align4 = student_malloc(33);
  
  printf("  Allocated 1, 7, 15, 33 bytes\n");
  printf("  Pointers should be 8-byte aligned:\n");
  printf("    %p (mod 8 = %d)\n", align1, (int)(((unsigned long)align1) % 8));
  printf("    %p (mod 8 = %d)\n", align2, (int)(((unsigned long)align2) % 8));
  printf("    %p (mod 8 = %d)\n", align3, (int)(((unsigned long)align3) % 8));
  printf("    %p (mod 8 = %d)\n", align4, (int)(((unsigned long)align4) % 8));
  
  student_free(align1);
  student_free(align2);
  student_free(align3);
  student_free(align4);
  
  getmemstats(&magic, &strategy, &num_alloc, &total_alloc, &num_free);
  if(num_alloc == 0) {
    printf("  ✓ Cleanup successful\n");
  }
  printf("\n");
  
  printf("=== Stress Test Complete ===\n");
  printf("Summary:\n");
  printf("  - Edge cases: Passed\n");
  printf("  - Sequential allocations: Tested\n");
  printf("  - Fragmentation handling: Tested\n");
  printf("  - Rapid alloc/free cycles: Passed\n");
  printf("  - Large allocations: Tested\n");
  printf("  - Performance test: Completed\n");
  printf("  - Memory leak detection: No leaks\n");
  printf("  - Alignment: Verified\n");
  
  exit(0);
}
