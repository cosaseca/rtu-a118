#include <stdio.h>
#include <sys/time.h>

int main() 

{
  struct timeval a0;
  gettimeofday(&a0, NULL);
  
  printf("%d, %d", a0.tv_sec, a0.tv_usec);
  return 0;
} 
