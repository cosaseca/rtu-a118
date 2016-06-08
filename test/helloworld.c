#include <stdio.h>

int main(int argc, char *argv[]) {
  
 printf("arg num %d\n", argc);
 if(argc > 0) {
  printf("argv %s\n", argv[0]);
}
 if(argc > 1) {

  printf("argv 1 %s\n", argv[1]);
}

 if(argc > 2) {

  printf("argv 2 %s\n", argv[2]);
}
 return 0;

}
