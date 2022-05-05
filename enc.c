#include "tea.c"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#ifdef ENCRYPT
#define f encrypt
#define g decrypt
#else
#define f decrypt
#define g encrypt
#endif

int main() {
  char ch;
  int len = 0;
  char *m = malloc(0xFFFFFF);

  int32_t key[] = {0x12d2c92e, 0x15ea3da8, 0x56233213, 0x26b00e15}; 

  while (read(0, &ch, 1) > 0) {
    m[len] = ch;
    len += 1;
  }

  if (len % 8 != 0) len += len % 8;
  for (int i = 0; i < len / 8; i++) {
    int32_t v[2];
    v[0] = *(int*) &m[i * 8];
    v[1] = *(int*) &m[i * 8 + 4];
    f(&m[i * 8], key);

    int32_t u[2];
    u[0] = *(int*) &m[i * 8];
    u[1] = *(int*) &m[i * 8 + 4];
    g(u, key);
    if (v[0] != u[0] || v[1] != u[1]) {
      fprintf(stderr, "doesnt match %x vs %x\n", v[0], u[0]);
    }
  }

  write(1, m, len);

  free(m);
}
