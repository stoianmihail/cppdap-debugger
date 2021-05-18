#include <iostream>
#include <vector>

unsigned int fib(unsigned int n)
{
  if (n==0) {
    return 0;
  } else if (n==1) {
    return 1;
  } else {
    return fib(n-1) + fib(n);
  }
}

int main(int argc, char *argv[]) {
	std::cout << "entttttttttttter" << std::endl;
  std::vector<unsigned> v(1000000000);
  v[2 * v.size()] = 100;
  std::cerr << "val=" << v[v.size()] << std::endl;
#if 0
	unsigned int f0 = fib(0);
  unsigned int f1 = fib(1);
  unsigned int f2 = fib(2);
  printf("/*** fib ***/\n");
	std::cerr << f0 << " " << f1 << " " << f2 << std::endl;
#endif
  return 0;
}
