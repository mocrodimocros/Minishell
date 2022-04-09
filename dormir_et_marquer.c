#include  <stdio.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <sys/wait.h>
#include  <string.h>
int main() {
	int s=10;
	int i =0;
	while (i<s) {
		sleep(1);
		printf("%d\n",i);
		i++;
	}
}
