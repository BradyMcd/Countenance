
#include <stdio.h>
#include <unistd.h>

int main(){

  printf( "%d", sysconf( _SC_PAGE_SIZE ) );

  return 0;
}
