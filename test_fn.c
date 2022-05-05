#include <stdio.h>

int dd() {
  return 4;
}

int cc() {
  return dd();
}

int bb() {
  return cc();
}

int aa() {
  int b = 4 + 3;
  printf("Hello\n");
  return bb();
}

int base_hook(void *pr) {
  aa();
  return 1;
}
