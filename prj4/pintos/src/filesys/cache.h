#ifndef CACHE_H
#define CACHE_H

#include <list.h>
#include "devices/block.h"
#include "devices/timer.h"
#include "threads/synch.h"


#define CACHE_SIZE 64                       // 64 cache sector
/*  
    TIMER_FREQ = 100
    1 tick =  1 / TIMER_FREQ (s) = 0.01 s
    Every 20s, write all dirty cache blocks to disk.
*/
#define WRITE_BACK_INTERVAL 2000 * TIMER_FREQ   

struct cache_block
{
  block_sector_t sector_no;         // Sector number

  // Used in second chance algorithm
  bool access;                      // If access
  bool dirty;                       // If written or not

  char data[BLOCK_SECTOR_SIZE];     // Cache data
  struct lock cache_lock;           // Lock for this cache block
  struct list_elem elem;            // Inserted in cache_list
};

void cache_init(void);
struct cache_block* in_cache(block_sector_t);
struct cache_block* cache_get(block_sector_t, bool);
struct cache_block* cache_new(void);
struct cache_block* cache_evict(void);
void periodical_write_back (void*);
void cache_write_to_disk(bool);
void cache_read_ahead(block_sector_t);
void func_read_ahead (void *);

#endif /* filesys/cache.h */
