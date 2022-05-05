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

#define BASE 0xe7

int main(void) {

  // Read payload
  printf("Reading payload... ");
  FILE *f = fopen("conjoined", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
  
  int sl = fsize;
  unsigned char *string = calloc(sl, 1);
  fread(string, fsize, 1, f);
  fclose(f);
  printf("done\n");

  printf("Chainging page permissions... ");
  // Change the permissions of the page that contains foo() to read, write, and execute
  // This assumes that foo() is fully contained by a single page
  void *foo_addr = (void*)foo;
  if(change_page_permissions_of_address(foo_addr, sl) == -1) {
      fprintf(stderr, "Error while changing page permissions of foo(): %s\n", strerror(errno));
      return 1;
  }
  printf("done\n");

  // Call the unmodified foo()
  int p = foo();

  unsigned char *add = foo_addr;
  printf("Copying payload... \n");

  int64_t _PLT_SLOTS[] = PLT_SLOTS;// {0x4018 + PAYLOAD_OFFSET, 0x4020 + PAYLOAD_OFFSET};
  void* _SLOT_VALUES[] = SLOT_VALUES;// {}
  int N_FNS = sizeof(_PLT_SLOTS) / 8;
 
  for (int i = 0; i < sl; i++)
    add[i] = string[i];
  printf("Done copying payload... \n");

  // decrypt payload
  int32_t key[] = {0x12d2c92e, 0x15ea3da8, 0x56233213, 0x26b00e15};
  void (*decrypt)(int v[], const int k[]) = foo_addr;
  for (int i = 0; i < sl / 8; i++) {
    (*decrypt)(&add[BASE + i * 8], key);
  }
  printf("Done\n");
  
  printf("Initializing PLT\n");
  for (int i = 0; i < N_FNS; i++) {
    printf("*%p = %p\n", &add[_PLT_SLOTS[i]], _SLOT_VALUES[i]);
    *(void **)(&add[_PLT_SLOTS[i]]) = _SLOT_VALUES[i];
  }
  printf("Done initializing PLT\n");


  int (*fn)(void *) = (foo_addr + HOOK_OFFSET + BASE);
  printf("%p %p\n", fn, foo); fflush(0);

  // Calling the modified foo()
  printf("Calling payload fn\n");
  int x = fn((void *) printf);

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
