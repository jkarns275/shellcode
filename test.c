#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
/*
 *
 * https://stackoverflow.com/questions/21681578/creating-a-c-function-with-a-given-size-in-the-text-segment
 */
int foo(void);
int change_page_permissions_of_address(void *addr, size_t len);
int main(void) {
    FILE *f = fopen("test_fn", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
    
    unsigned char *string = malloc(fsize);
    fread(string, fsize, 1, f);
    fclose(f);


    void *foo_addr = (void*)foo;

    // Change the permissions of the page that contains foo() to read, write, and execute
    // This assumes that foo() is fully contained by a single page
    if(change_page_permissions_of_address(foo_addr, fsize) == -1) {
        fprintf(stderr, "Error while changing page permissions of foo(): %s\n", strerror(errno));
        return 1;
    }

    // Call the unmodified foo()
    puts("Calling foo...");
    int p = foo();
    printf("before: %d\n", p);

    unsigned char *add = foo_addr;

    for (int i = 0; i < fsize; i++) {
      printf("%d\n", i);
      add[i] = string[i];
    }
    printf("Done\n");


    int (*fn)(void) = (foo_addr + 0x11139);
    printf("%p %p\n", fn, foo); fflush(0);


    // Call the modified foo()
    puts("Calling fn...");
    int x = 0;//fn();
    printf("after: %d\n", x);

    // void *pp = (void *)change_page_permissions_of_address - (void *) foo;
    // printf("%x \n");

    return 0;
}

int foo(void) {
    int i=10;
    
    printf("i: %d\n", i);
    return i;
}
asm(".org .+0xFFFFF");
int change_page_permissions_of_address(void *start, size_t len) {
  int page_size = getpagesize();
  void *addr = start - ((unsigned long)start % page_size);
  
  while(addr < start + len) {
    // Move the pointer to the page boundary
    if(mprotect(addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return -1;
    }
    addr += page_size;
  }
  
  return 0;
}
