#include <stdio.h>
#include <unistd.h>

int main()
{
	fork();

	fork();

	fork();
	while(1);
	return 0;
}