
#include <malloc.h>

#include "memory.h"

static struct countenance_config mem_cfg = { 256 };



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
  size_t width;
  char buffer[];
};

static struct fw_manager **fw_buffer = NULL;
static size_t fw_managers = 0;

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

struct fw_manager *fw_new_mgr( size_t width ){

  struct fw_manager *ret = NULL;
  struct fw_manager **temp = NULL;

  /* TODO: This should end up being a bufferlist type
   * Most of the bootstrapping should probably have the option to be statically allocated
   * The only malloc which is truly unavoidable is claiming the memory pages, managers can
   * concievably be static, worst cases are pretty simple to calculate and testing can reveal
   * saner limits for things like the list of managers
   */

  temp = realloc( fw_buffer, sizeof(struct fw_manager*) * ( fw_managers + 1 ) );
  if( temp == NULL ){ return NULL; }
  fw_buffer = temp;

  ret = malloc( sizeof(struct fw_manager) + ( width * mem_cfg.fw_n_members ) );
  if( ret == NULL ){ return NULL; }

  ret->available = bs_fw_malloc();
  if( ret->available == NULL ){ return NULL; }

  ret->available->ptr = ret->buffer;
  ret->available->n = mem_cfg.fw_n_members;
  ret->available->next = NULL;

  fw_buffer[fw_managers] = ret;
  fw_managers += 1;

  return ret;
}

/*FW PUBLIC*/

void *fw_malloc( size_t size ){

  int i;
  struct fw_manager *buffer = NULL;
  fw_record *curr = NULL;
  void *ret = NULL;

  for( i = 0; i < fw_managers && curr == NULL; ++i ){
    buffer = fw_buffer[i];
    if( buffer->width == size ){
      curr = buffer->available;
    }
  }
  if( curr == NULL ){
    buffer = fw_new_mgr( size );
    if( buffer == NULL ){ return NULL; }

    curr = buffer->available;
  }
  /* curr is a record, curr->ptr is the first piece of open space */

  ret = curr->ptr;
  curr->n -= 1;
  if( curr->n != 0 ){
    curr->ptr = curr->ptr + 1;
  }else{
    buffer->available = curr->next;
    bs_fw_free( curr );
  }

  return ret;
}

void fw_free( void *ptr ){
  int i;
  struct fw_manager *manager = NULL;
  fw_record *curr;
  fw_record *temp;

  if( !ptr ){ return; } /* If ptr is NULL no action is performed */

  /* Find the manager corresponding to ptr */
  for( i = 0; i < fw_managers && !manager; ++i ){
    manager = fw_buffer[i];
    if( (char*)ptr <= manager->buffer ||
        manager->buffer + ( manager->width * mem_cfg.fw_n_members ) <= (char*)ptr ){
      manager = NULL;
    }
  }
  if( manager == NULL ){ /* This case is undefined by the standard definition of free */ }

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
