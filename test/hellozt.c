#include <stdio.h>
#include <time.h>
int main(void)
{
time_t td;
putenv("TZ=PST8PDT");
tzset();
time(&td);
printf("Current time = %s\n", asctime(localtime(&td)));
return 0;
}
