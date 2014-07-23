#include <stdio.h>
#include "fby_core_unix.h"

using namespace FbyHelpers;

using namespace std;
#if 0

class Junk
{
public:
	int v;
	Junk(int v) : v(v) {}
};

void PrintArray(CocoaArray<int> &array)
{
	int c = array->Count;
	printf("Array size %d\n", c);

	for (int i = 0; i < array->Count; ++i)
	{
		printf("%d: %d\n", i, array[i]);
	}
}


int main(int argc, char **argv)
{
	CocoaArray<int> array;
	CocoaArray<int> array2;

	array2->Add(41);
	array2->Add(71);

	array->Add(166);
	array->Add(4);
	array->Add(16);
	array->Add(78);
	array->Add(7);

	PrintArray(array);
	printf("Removing 3rd element\n");
	array->RemoveAt(3);
	PrintArray(array);

	printf("Removing element 7\n");
	array->Remove(7);
	PrintArray(array);

	printf("Added other range\n");
	array->AddRange(array2);
	PrintArray(array);

	return 0;
}
#endif
