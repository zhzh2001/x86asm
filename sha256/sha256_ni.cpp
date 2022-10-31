#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
using namespace std;
const int BLOCK_SIZE = 64;
const int DIGEST_SIZE = 32;
const uint32_t K256[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
#define ROTR(x, n) ((x >> n) | (x << (32 - n)))
extern "C" void sha256_ni_transform(uint32_t *digest, const void *data, uint32_t numBlocks);
struct SHA256
{
	uint32_t state[8];
	uint8_t buffer[BLOCK_SIZE];
	uint64_t totalBytes;

	SHA256()
	{
		totalBytes = 0;
		state[0] = 0x6a09e667;
		state[1] = 0xbb67ae85;
		state[2] = 0x3c6ef372;
		state[3] = 0xa54ff53a;
		state[4] = 0x510e527f;
		state[5] = 0x9b05688c;
		state[6] = 0x1f83d9ab;
		state[7] = 0x5be0cd19;
	}

	void processBlock(uint8_t *block)
	{
#ifdef _WIN32
		// convert to AMD64 ABI under Windows
		uint64_t rdi, rsi, rdx, rax;
		// backup registers
		asm volatile(
			"mov %%rdi, %0\n\t"
			"mov %%rsi, %1\n\t"
			"mov %%rdx, %2\n\t"
			"mov %%rax, %3\n\t"
			: "=m"(rdi), "=m"(rsi), "=m"(rdx), "=m"(rax)
			:
			: "memory");
		asm volatile(
			"mov %0, %%rdi\n\t"
			"mov %1, %%rsi\n\t"
			"mov $1, %%rdx\n\t"
			"call sha256_ni_transform\n\t"
			:
			: "r"(state), "r"(block)
			: "rdi", "rsi", "rdx");
		// restore registers
		asm volatile(
			"mov %0, %%rdi\n\t"
			"mov %1, %%rsi\n\t"
			"mov %2, %%rdx\n\t"
			"mov %3, %%rax\n\t"
			:
			: "m"(rdi), "m"(rsi), "m"(rdx), "m"(rax)
			: "memory");
#else
		sha256_ni_transform(state, block, 1);
#endif
	}

	void sha256(uint8_t *data, uint64_t len)
	{
		uint64_t i = 0;
		while (len - i >= BLOCK_SIZE)
		{
			processBlock(data + i);
			i += BLOCK_SIZE;
		}
		uint8_t block[BLOCK_SIZE] = {0};
		uint64_t rem = len - i;
		for (uint64_t j = 0; j < rem; j++)
		{
			block[j] = data[i + j];
		}
		block[rem] = 0x80;
		uint64_t bitLen = len * 8;
		if (rem < 56)
		{
			for (int i = 0; i < 8; i++)
			{
				block[56 + i] = (bitLen >> ((7 - i) * 8)) & 0xFF;
			}
			processBlock(block);
		}
		else
		{
			processBlock(block);
			for (int i = 0; i < 56; i++)
			{
				block[i] = 0;
			}
			for (int i = 0; i < 8; i++)
			{
				block[56 + i] = (bitLen >> ((7 - i) * 8)) & 0xFF;
			}
			processBlock(block);
		}
	}
};

void readfile(char *filename)
{
	ifstream fin(filename, ios::binary);
	SHA256 sha256;
	fin.seekg(0, ios::end);
	uint64_t len = fin.tellg();
	fin.seekg(0, ios::beg);
	uint8_t *data = new uint8_t[len];
	fin.read((char *)data, len);
	sha256.sha256(data, len);
	cout << hex;
	cout.width(8);
	cout.fill('0');
	for (int i = 0; i < 8; i++)
		cout << sha256.state[i];
	cout << endl;
}

void benchmark(int n = 1e6)
{
	SHA256 sha256;
	uint8_t data[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++)
		data[i] = rand() % 256;
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < n; i++)
		sha256.processBlock(data);
	cout << hex;
	cout.width(8);
	cout.fill('0');
	for (int i = 0; i < 8; i++)
		cout << sha256.state[i];
	cout << endl;
	auto end = chrono::high_resolution_clock::now();
	cout << dec;
	cout.width(0);
	// output in seconds
	cout << chrono::duration_cast<chrono::duration<double>>(end - start).count() << " seconds" << endl;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " n" << endl;
		benchmark();
	}
	else
	{
		int n = atoi(argv[1]);
		benchmark(n);
	}
	return 0;
}