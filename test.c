#include <stdio.h>
#include <time.h>

int main(){
	
	time_t timer = time(NULL);
	char *now = ctime(&timer);
	printf("%s",now);
	return 0;
}