#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "threads/thread.h"


struct list cache_list;
struct lock evict_lock;

// If this sector is in cache.
struct cache_block *
in_cache (block_sector_t sector)
{
  struct cache_block *cb;
  struct list_elem *e;
  for (e = list_begin(&cache_list); e != list_end(&cache_list);
       e = list_next(e))
  {
    cb = list_entry(e, struct cache_block, elem);
    if ( cb->sector_no == sector)
      return cb;
  }
  return NULL;
}

// Init cache
void cache_init()
{
  list_init(&cache_list);
  lock_init(&evict_lock);
  thread_create("periodical_write_back", PRI_DEFAULT, periodical_write_back, NULL);
}

// Try to get the sector in cache.
// If it's in cache, get this cache, otherwise create a new cache.
struct cache_block *
cache_get(block_sector_t sector, bool dirty)
{
  struct cache_block *cb = in_cache(sector);
  if (cb)
  {
    cb->access = true;
    cb->dirty |= dirty;
  }
  else
  {
    cb = cache_new();
    cb->sector_no = sector;
    cb->access = true;
    cb->dirty = dirty;
    block_read(fs_device, cb->sector_no, &cb->data);
  }
  //cache_read_ahead(sector);  //failed to read ahead
  return cb;
}


// Create a new cache
struct cache_block *
cache_new()
{
  struct cache_block *cb;
  if (list_size(&cache_list) < CACHE_SIZE)
  {
    cb = malloc(sizeof(struct cache_block));
    if(!cb)
      PANIC ("No enough memory for cache!");
    list_push_back(&cache_list, &cb->elem);
  }
  else
  {
    cb = cache_evict();
  }
  return cb;
}

// When the cache is full, evict a cache_block using second chance algorithm.
struct cache_block *
cache_evict()
{
  lock_acquire(&evict_lock);
  struct cache_block *cb;
  bool loop = true;
  while (loop)
  {
    struct list_elem *e;
    for (e = list_begin(&cache_list); e != list_end(&cache_list);
         e = list_next(e))
    {
      cb = list_entry(e, struct cache_block, elem);

      if (cb->access)
      {
        cb->access = false;
      }
      else
      {
        if (cb->dirty)
        {
          block_write(fs_device, cb->sector_no, &cb->data);
        }
        lock_release(&evict_lock);
        return cb;
      }
    }
  }
}

// Every 2s, write back all the cache.
void 
periodical_write_back (void *aux UNUSED)
{
  while (true)
  {
    timer_sleep(WRITE_BACK_INTERVAL);
    cache_write_to_disk(false);
  }
}

// Write back all the cache.
void 
cache_write_to_disk(bool done)
{
  struct list_elem *next, *e;
  struct cache_block *cb;
  for (e = list_begin(&cache_list); e != list_end(&cache_list);)
  {
    next = list_next(e);
    cb = list_entry(e, struct cache_block, elem);

    if (cb->dirty)
    {
      block_write(fs_device, cb->sector_no, &cb->data);
      cb->dirty = false;
    }
    if(done)
    {
      list_remove(&cb->elem);
      free(cb);
    }
    e = next;
  }
}

// Read ahead.
void
cache_read_ahead(block_sector_t sector)
{
  block_sector_t *aux = malloc(sizeof(block_sector_t));
  if (!aux)
    PANIC("No enough memory for aux of read_ahead!");
  
  *aux = sector + 1;
  thread_create("cache_read_ahead", PRI_DEFAULT, func_read_ahead,
                aux);
}

// Read ahead function in background.
void 
func_read_ahead (void *aux)
{
  block_sector_t sector = * (block_sector_t *) aux;
  cache_get(sector, false);
  free(aux);
}

