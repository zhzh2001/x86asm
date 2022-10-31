#include <iostream>
#include <cstdint>
#include <cstring>
#ifndef _WIN32
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#endif
using namespace std;
const int N = 1e8;
uint64_t rdtsc()
{
	uint32_t lo, hi;
	asm volatile("rdtsc"
				 : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}
uint64_t rdpmc(uint32_t counter)
{
	uint32_t lo, hi;
	asm volatile("rdpmc"
				 : "=a"(lo), "=d"(hi)
				 : "c"(counter));
	return ((uint64_t)hi << 32) | lo;
}
#ifndef _WIN32
int perf_fd = -1;

static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	return (int)syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

void init_cycle_counter()
{
	struct perf_event_attr attr;
	memset(&attr, 0, sizeof(attr));

	attr.type = PERF_TYPE_HARDWARE;
	attr.size = sizeof(attr);
	attr.config = PERF_COUNT_HW_CPU_CYCLES;
	attr.exclude_kernel = 1;

	perf_fd = perf_event_open(&attr, 0, -1, -1, 0);
	if (perf_fd == -1)
	{
		perror("perf_event_open");
		exit(1);
	}
}
#endif
int main()
{
	// system("wrmsr 0xc0010200 0x410076");
#ifndef _WIN32
	init_cycle_counter();
#endif
	asm volatile(
		"pxor %%xmm0, %%xmm0\n\t"
		"pxor %%xmm1, %%xmm1\n\t"
		"pxor %%xmm2, %%xmm2\n\t"
		"pxor %%xmm3, %%xmm3\n\t"
		"pxor %%xmm4, %%xmm4\n\t"
		"pxor %%xmm5, %%xmm5\n\t"
		"pxor %%xmm6, %%xmm6\n\t"
		"pxor %%xmm7, %%xmm7\n\t"
		"pxor %%xmm8, %%xmm8\n\t"
		"pxor %%xmm9, %%xmm9\n\t"
		"pxor %%xmm10, %%xmm10\n\t"
		"pxor %%xmm11, %%xmm11\n\t"
		"pxor %%xmm12, %%xmm12\n\t"
		:
		:
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12");
	auto start = rdtsc();
#ifndef _WIN32
	uint64_t start2;
	read(perf_fd, &start2, sizeof(start2));
	auto start3 = rdpmc(0);
#endif
	for (int i = 0; i < N; i++)
	{
		asm volatile(
			"sha256rnds2 %%xmm2, %%xmm1\n\t"
			"sha256rnds2 %%xmm2, %%xmm3\n\t"
			"sha256rnds2 %%xmm2, %%xmm4\n\t"
			"sha256rnds2 %%xmm2, %%xmm5\n\t"
			"sha256rnds2 %%xmm2, %%xmm6\n\t"
			"sha256rnds2 %%xmm2, %%xmm7\n\t"
			"sha256rnds2 %%xmm2, %%xmm8\n\t"
			"sha256rnds2 %%xmm2, %%xmm9\n\t"
			"sha256rnds2 %%xmm2, %%xmm10\n\t"
			"sha256rnds2 %%xmm2, %%xmm11\n\t"
			"sha256rnds2 %%xmm2, %%xmm12\n\t"
			:
			:
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12");
	}
	auto end = rdtsc();
#ifndef _WIN32
	uint64_t end2;
	read(perf_fd, &end2, sizeof(end2));
	auto end3 = rdpmc(0);
#endif
	cout << "rdtsc: " << 1.0 * (end - start) / N / 11 << endl;
#ifndef _WIN32
	cout << "rdpmc: " << 1.0 * (end2 - start2) / N / 11 << endl;
	cout << "perf_event: " << 1.0 * (end3 - start3) / N / 11 << endl;
#endif
	return 0;
}