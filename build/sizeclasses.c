
#include <unistd.h>
#include <stdio.h>

/*This program prints the number of size classes which can fit in a single page + 1
 */

int main(){

  long page_size = sysconf( _SC_PAGE_SIZE );
  int r = 0;

  if( page_size ){
    do{ ++r; }while( page_size >>= 1 );
  }
  printf( "%d", r );

  return 0;
}
