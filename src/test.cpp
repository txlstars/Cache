#include <iostream>
#include "cache.h"

using namespace std;
using namespace cache;

namespace test
{
	int a = 1;
}


int main()
{
	Cache<int, int> cache;
	cout << test::a << endl;

	return 0;
}
