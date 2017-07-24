
#include <stdio.h>
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif//PAGE_SIZE

typedef struct ovr{
  uint16_t a;
  uint16_t b;
}ovr;

size_t calc_wasteage( size_t width ){

  size_t n = ( PAGE_SIZE - sizeof( void* ) ) / ( width + ( sizeof( ovr ) / 2 ) );
  printf( "n is %d\n", n );
  return PAGE_SIZE - sizeof( void* ) - ( width * n ) - ( ( n / 2 ) * sizeof( ovr ) );
}


int main(){

  printf( "Wastage calculated as being %d bytes\n", calc_wasteage( 64 ) );

  return 0;
}
