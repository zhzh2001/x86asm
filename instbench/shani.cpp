#include <iostream>
#include <immintrin.h>
#include <chrono>
using namespace std;
int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		cout<<"Usage: "<<argv[0]<<" n"<<endl;
		return 0;
	}
	int n=atoi(argv[1]);
	auto start=chrono::high_resolution_clock::now();
	register __m128i a,b,k,d;
	for(int i=0;i<n;i++)
		d=_mm_sha256rnds2_epu32(a,b,k);
	auto end=chrono::high_resolution_clock::now();
	cout<<"Time: "<<chrono::duration_cast<chrono::milliseconds>(end-start).count()<<" ms"<<endl;
	return 0;
}