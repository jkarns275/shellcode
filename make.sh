rm -rf build
mkdir build

# To encrypt the payload
gcc enc.c -o build/enc -DENCRYPT
gcc enc.c -o build/dec -DDECRYPT

# Payload code
gcc -c test_fn.c -o build/test_fn.o
gcc -shared -o build/test_fn build/test_fn.o
rm -rf build/test_w_defines.c
objdump -DF build/test_fn | grep -E "^0[0-9a-fA-F]+ <[a-zA-Z]" -A 1 | sed '/^--/d'
objdump -DF build/test_fn | grep -E "^0[0-9a-fA-F]+ <[a-zA-Z]" -A 1 | sed '/^--/d' | python3 mk_offset_map.py > build/test_w_defines.c
cat test.c >> build/test_w_defines.c

# Encrypt payload code
cat build/test_fn | build/enc > build/test_fn_enc

# Compile decrypt code
gcc -c tea.c -DNO_ENCRYPT -o build/tea.o
gcc -shared -o build/tea build/tea.o

gcc -g build/test_w_defines.c -fno-toplevel-reorder -o build/test
let dec_offset=$(nm --print-size --radix=d build/tea | grep decrypt | grep -Eo '^0[0-9a-f]+' | grep -Eo '[1-9a-f][0-9a-f]*')
let dec_size=$(nm --print-size --radix=d build/tea | grep decrypt | grep -Eo ' 0[0-9a-f]+' | grep -Eo '[1-9a-f][0-9a-f]*')

rm -rf build/conjoined
dd if=build/tea skip=$dec_offset count=$dec_size bs=1 of=build/conjoined
cat build/test_fn_enc >> build/conjoined

printf "encrypted data location: 0x%x\n" $dec_offset 
printf "encrypted data size: 0x%x\n" $dec_size
printf "decrypt procedure and encrypted payload: 0x%x\n" $(stat --format="%s" build/conjoined)
