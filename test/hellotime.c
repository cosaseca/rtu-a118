#include <stdio.h>
#include <time.h>
int main(void)
{
time_t t;
struct tm *area;
t = time(NULL);
area = localtime(&t);
printf("Number of seconds since 1/1/1970 is: %ld\n", t);
printf("Local time is: %s", asctime(area));
t++;
area = localtime(&t);
printf("Add a second: %s", asctime(area));
t += 60;
area = localtime(&t);
printf("Add a minute: %s", asctime(area));
t += 3600;
area = localtime(&t);
printf("Add an hour: %s", asctime(area));
t += 86400L;
area = localtime(&t);
printf("Add a day: %s", asctime(area));
t += 2592000L;
area = localtime(&t);
printf("Add a month: %s", asctime(area));
t += 31536000L;
area = localtime(&t);
printf("Add a year: %s", asctime(area));
return 0;
}
