gcc -g test_fn.c -o test_fn -fpic
gcc -g test.c -fno-toplevel-reorder -Wl,-N,-Bdyamic -o test
