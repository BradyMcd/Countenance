#include <stdio.h>
#include <stdint.h>

struct fw_manager{
  struct fw_manager *next;
};

#define PAGE_SIZE 4096

#define STR(s) #s
#define xSTR(s) STR(s)

#define N_RECS(N) ( ( PAGE_SIZE - sizeof( struct fw_manager ) ) / ( ( 2 * N ) + sizeof( uint16_t ) ) )

#define RECORD_(N)            \
"struct SC_" #N "_RECORD{     \n\
  struct fw_manager *mgr;     \n\
  struct{                     \n\
    uint16_t idx;             \n\
    uint16_t n;               \n\
  } record[%d]                \n\
};\n\n", N_RECS(N)

int main(){

  printf( RECORD_(4) );
  printf( RECORD_(5) );
  printf( RECORD_(6) );
  return 0;
}
