
#include <malloc.h>
#include <unistd.h>

#include "memory.h"

#define SIZE_CLASSES 32
static long page_size = sysconfig( _GET_PAGESIZE );
/**
 * Memory records are of fixed width... hmmm...
 */

typedef struct fw_record{
  struct fw_record *next;
  void* ptr;
  size_t n;
}fw_record;

struct fw_manager{
  fw_record *available;
  struct fw_manager *next;
  void *page;
};

static struct fw_manager fw_bins[SIZE_CLASSES] = NULL;

static fw_record *bs_fw_malloc(){
  return malloc( sizeof( fw_record ) );
}

static void bs_fw_free( fw_record *ptr ){
  free( ptr );
}


/**
 * Variable width, resizable allocations
 */

void *rw_malloc( size_t size ){
  return malloc( size );
}

void *rw_realloc( void* ptr, size_t size ){
  return realloc( ptr, size );
}

void rw_free( void *ptr ){
  free( ptr );
}

/**
 * Fixed width, recyclable allocations
 */

size_t fit_to_size_class( size_t width ){

  /* sizeof only returns aligned widths
     META: We can decide on which allocator to use based on the width passed in*/
  size_t r = 0;
  size_t t = width;

  while( t >>= 1){
    ++r;
  }

  if( width && !( width & ( width - 1 ) ) )
    --r;

  return r;
}

fw_record *fw_claim( size_t width ){

  /* META: This is going to be POSIX compliant, so we won't be passing
     the fw_ prefix any requests over the page size, those are to be handled by a
     different interface which will mmap() rather than sbrk() to claim space
  */

  size_t n = ( page_size - sizeof( struct fw_manager ) ) / width;
  struct fw_manager *new = sbrk( page_size );
  if( new == -1 ){ /*ENOMEM*/ }

  new->page = &(new + 1);
  new->next = NULL;

  new->available = bs_fw_malloc();
  if( new->available == NULL ){ /*ENOMEM*/ }

  new->available->n = n;
  new->available->ptr = new->page;
  new->available->next = NULL;

  return new->available;
}

/*FW PUBLIC*/

void *fw_malloc( size_t size ){

  int i;
  struct fw_manager *buffer;
  fw_record *curr = NULL;
  void *ret = NULL;

  buffer = fw_bins[fit_to_size_class( size )];
  curr = ( buffer->available != NULL ) ? buffer->available : fw_claim( size );
  if( curr == NULL ){ /* ENOMEM */ }

  /* curr is a record, curr->ptr is the first piece of open space */

  ret = curr->ptr;
  curr->n -= 1;
  if( curr->n != 0 ){
    curr->ptr = curr->ptr + size;
  }else{
    buffer->available = curr->next;
    bs_fw_free( curr );
  }

  return ret;
}

void fw_free( void *ptr ){

  int i;
  struct fw_manager *manager = NULL;
  fw_record *temp;
  fw_record *curr;

  if( !ptr ){ return; } /* If ptr is NULL no action is performed */

  for( i = 0; i < SIZE_CLASSES && manager == NULL; ++i ){
    manager = fw_bins[i];
    while( manager ){
      if( ptr / page_size != manager->page / page_size ){
        manager = manager->next;
      }
    }
  }

  if( manager == NULL ){
    /*Getting here means that a pointer which was never returned by malloc has been
      passed to free.
      According to the POSIX standard this behavior is undefined
     *Failing hard seems pretty reasonable though
     */
    exit(1);
  }

  /* TODO: Everything below this line feels very special case heavy */
  curr = manager->available;
  if( curr == NULL ){
    temp = bs_fw_malloc();
    temp->n = 1;
    temp->ptr = ptr;
    temp->next = NULL;
    manager->available = temp;
    return;
  }

  /* Reordering these cases have potential performance effects */
  while( 1 ){
    if( curr->ptr == ptr + manager->width ){
      curr->ptr = ptr;
      curr->n += 1;
      break;
    }else if( curr->ptr + ( curr->n * manager->width ) == ptr - manager->width  ){
      curr->n += 1;
      if( curr->next && curr->next->ptr == ptr + manager->width ){
        curr->n += curr->next->n;
        temp = curr->next;
        curr->next = curr->next->next;
        bs_fw_free( temp );
      }
      break;
    }else if( curr->next == NULL || curr->next->ptr > ptr ){
      temp = bs_fw_malloc();
      temp->n = 1;
      temp->ptr = ptr;
      temp->next = curr->next;
      curr->next = temp;
      break;
    }else{
      curr = curr->next;
    }
  }
}

/**
 * Variable width, write once allocations
 */

void *wo_malloc( size_t size ){
  return malloc( size );
}

void wo_free( void *ptr ){
  free( ptr );
}
