📖 readme.txt  
    ├── 💡 Implementation explanation  
    **About my demo video** quick side note, 
    i feel like i did not have enough time to fully explain all of my code but i tried my best by speeding up the video , i mainly explained kalloc.c in the video 
    and did not go too much into depth about all the other files i modified in this assignment , like syscall.h , defs.h, etc... however
    in my code i added comments so everything that i did not have the full time to explain i added comments in my corresponds
    
        In my implementation, each free list entry corresponds to one full 4KB page.
I did not implement block splitting, because the assignment allows a simplified allocator.

This means:

Every block in the free list is the same size (a full page).

When student_malloc(size) is called, I simply choose a free page.

Even if the user request is small (e.g., 50 bytes), the entire 4KB page is marked as allocated.

If there isnt a free page available , I call kalloc() to get another new 4KB page.

Because all blocks have identical size, the best-fit strategy does not actually need to find the “smallest” free block—there is no smaller or larger page. Every free block is equally suitable.

So in practice:

Best-fit behaves the same as first-fit in my implementation

The allocator simply returns the first available free page
    ├── 🔍 Design decisions  

        • System Call Architecture:
          I modified the student_get_stats() function signature to return void instead of int.
          This design allows me to retrieve multiple statistics (magic number, strategy, 
          allocated blocks, total allocated memory, and free blocks) by passing pointers as 
          parameters. This approach is cleaner and more straightforward than encoding multiple 
          values into a single return value in my opinion.
          
          The getmemstats() system call, calls student_get_stats() in 
          the kernel, retrieves all five statistics, and copies them back to user space using 
          copyout(). This separation of concerns keeps the kernel implementation simple while 
          providing a comprehensive interface to user programs.
        
        • Pointer Arithmetic for User Data:
          At lines 217-218 in kalloc.c, I return (void*)((char*)best + sizeof(struct block_header))
          instead of just returning best. At first i thought to return best but the casting is crucial because the block header (16 bytes) 
          contains internal data (size, magic number, allocated flag, and next pointer) 
          that the user should never access or modify.
          
          By casting best to (char*) before adding sizeof(struct block_header), I ensure the 
          pointer arithmetic works correctly in bytes. This returns a pointer to the user data 
          area (starting 16 bytes after the page start), protecting the header from accidental 
          corruption while giving the user access to the remaining 4080 bytes of usable memory.
          
          When student_free() is called, the reverse operation (ptr - sizeof(struct block_header)) 
          retrieves the header to verify the magic number and update allocation status.
        
        • Memory Alignment:
          I use 8-byte alignment (ALIGNMENT = 3, meaning 2³ = 8 bytes) for compatibility
          with 64-bit architectures like RISC-V, where pointers and long integers require 
          8-byte alignment for optimal performance.
        
        • Pre-allocation Strategy:
          I pre-allocate 20 pages (FREE_LIST_SIZE = 20) during initialization to reduce the 
          overhead of calling kalloc() for every early allocation. This provides 81,920 bytes 
          of immediately available memory.
        
        • Thread Safety:
          All allocator operations use spinlocks (student_mem.lock) to prevent race conditions 
          when multiple CPUs access the allocator simultaneously. This ensures data consistency 
          in a multi-core environment.
    
    ├── 🧪 Testing approach  
        
        I created three comprehensive test programs to validate the allocator:
        
        • test_basic.c [70 Points]:
          This test validates core functionality:
          - Test 1: Verifies initial statistics (magic number = 16, strategy = 1, 20 free blocks)
          - Test 2: Allocates 5 blocks of different sizes (64, 128, 256, 512, 1024 bytes)
                    and verifies num_allocated = 5, total_allocated = 1984 bytes
          - Test 3: Frees 2 blocks (128 and 512 bytes) and confirms statistics update correctly
                    (num_allocated = 3, total_allocated = 1344 bytes, num_free increases by 2)
          - Test 4: Allocates 2 more blocks (100 and 200 bytes) to test reusing freed blocks
                    and verifies the free list management works properly
          - Test 5: Frees all remaining blocks and confirms no memory leaks
                    (num_allocated = 0, total_allocated = 0, num_free returns to 20)
          
          Each test calls getmemstats() to retrieve and verify the allocator's internal state,
          ensuring the magic number (16) matches my student number and all statistics are accurate.
        
        • test_strategy.c [20 Points]:
          This test validates the best-fit allocation strategy:
          - Creates fragmentation by allocating 5 blocks of 1000 bytes each, then freeing 
            alternating blocks (blocks 2 and 4) to create a pattern: 
            [Allocated][Free][Allocated][Free][Allocated] basically like this 
          - Tests best-fit behavior by allocating blocks of various sizes (500, 800 bytes)
            and verifying the allocator selects appropriate free blocks
          - Tests different allocation patterns:
            * Sequential small allocations (100 bytes each)
            * Mixed size allocations (50, 500, 150, 1500 bytes)
          - Verifies that the strategy value returned by getmemstats() is 1 (best-fit)
          - Note: Since all free blocks are identical 4KB pages, best-fit effectively 
            behaves as first-fit, but the test confirms the strategy implementation works
          - Ensures all blocks are freed at the end to prevent memory leaks
        
        • test_stress.c [10 Points]:
          This test performs rigorous stress testing:
          - Edge Cases:
            * Tests student_malloc(0) returns NULL
            * Tests student_free(NULL) doesn't crash
          - Sequential Allocations:
            * Allocates 50 blocks of varying sizes (50, 100, 150, ... cycling)
            * Verifies the allocator can handle many allocations
          - Fragmentation Stress:
            * Frees every other block to create maximum fragmentation
            * Re-allocates in freed spaces to test free list management
          - Rapid Allocation/Deallocation:
            * Performs 100 cycles of allocating 10 blocks and immediately freeing them
            * Tests allocator stability under heavy churn
          - Large Allocations:
            * Tests allocating 3000, 3500, and 3800 byte blocks
            * Ensures large requests within page limits work correctly
          - Performance Test:
            * Attempts 200 allocations to test scalability
            * Reports how many succeed (may run out of pages)
          - Memory Leak Detection:
            * Frees all allocated blocks and verifies num_allocated = 0, total_allocated = 0
          - Alignment Verification:
            * Allocates blocks of 1, 7, 15, and 33 bytes
            * Verifies all returned pointers are 8-byte aligned (address % 8 == 0)
          
          All tests use getmemstats() extensively to monitor allocator state and catch bugs.
    
    └── 📚 Usage instructions
        
        To compile and run the tests:
        
        1. Navigate to the xv6 directory in bash:
           $ cd /cs/home/sam50/xv6-studentno-mem
        
        2. Clean and build xv6:
           $ make clean
           $ make qemu
        
        3. Once xv6 boots, you'll see the shell prompt ($). Run any test:
           
           $ test_basic       # Run basic functionality tests (70 points)
           $ test_strategy    # Run allocation strategy tests (20 points)
           $ test_stress      # Run stress/edge case tests (10 points)
        
        4. To exit qemu:
           Press Ctrl-A, then press X
        
        Expected output:
        - test_basic: Should show 5 test sections with statistics verification,
                      confirming magic number = 16, no memory leaks
        - test_strategy: Should demonstrate fragmentation handling and best-fit behavior
        - test_stress: Should complete all 10 test sections including edge cases,
                       rapid cycles, and alignment verification
        
        All tests should print "✓" for successful checks.