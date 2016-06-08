#include <stdio.h>

int main() {
  unsigned short x;
  sscanf("0xFF", "%hx", &x);
  printf("%d\n", x);
}
