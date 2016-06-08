#include <stdio.h>

#define AT24_SIZE_FLAGS 8
#define AT24_SIZE_BYTELEN 5
#define AT24_DEVICE_MAGIC(_len, _flags) 		\
	((1 << AT24_SIZE_FLAGS | (_flags)) 		\
	    << AT24_SIZE_BYTELEN | ilog2(_len))

#define AT24_FLAG_ADDR16	0x80

int main() {

  printf("%d\n", AT24_DEVICE_MAGIC(262144 / 8, AT24_FLAG_ADDR16));
  return 0;
}
