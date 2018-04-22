#include "syscall.h"

int array[128];

int main()
{
	int i;
	for (i = 0; i < 1; i++) 
		array[i] = 42;

	//i = 5;
	Exit(888);
}

