all: sha256.cpp sha256_ni_asm.o
	g++ -o sha256 sha256.cpp sha256_ni_asm.o -march=native -O3
	objdump -d sha256 > sha256.dump

sha256_ni_asm.o: sha256_ni_asm.S
	gcc -c sha256_ni_asm.S -o sha256_ni_asm.o
