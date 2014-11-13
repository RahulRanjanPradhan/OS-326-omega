#include "vm/swap.h"

void swap_init (void)
{
  swap_block = block_get_role (BLOCK_SWAP);
  if (!swap_block)
  {
    return;
  }
  swap_bitmap = bitmap_create( block_size(swap_block) / SECTORS_PER_PAGE );
  if (!swap_bitmap)
  {
    return;
  }
  bitmap_set_all(swap_bitmap, 0);
  lock_init(&swap_lock);
}


size_t swap_out (void *frame)
{
  if (!swap_block || !swap_bitmap)
  {
    PANIC("No swap partition!");
  }
  lock_acquire(&swap_lock);
  // starting from 0 find 1 of 0(free) in swap map
  size_t free_index = bitmap_scan_and_flip(swap_bitmap, 0, 1, 0);

  if (free_index == BITMAP_ERROR)
  {
    PANIC("No free space in swap partition!");
  }

  size_t i;
  for (i = 0; i < SECTORS_PER_PAGE; i++)
  {
    block_write(swap_block, free_index * SECTORS_PER_PAGE + i,
                (uint8_t *) frame + i * BLOCK_SECTOR_SIZE);
  }
  lock_release(&swap_lock);
  return free_index;
}

void swap_in (size_t used_index, void *frame)
{
  if (!swap_block || !swap_bitmap)
  {
    PANIC("No swap partition!");
  }
  lock_acquire(&swap_lock);
  // bitmap[used_index] should be 1
  if (bitmap_test(swap_bitmap, used_index) == 0)
  {
    PANIC ("Wrong swap index!");
  }
  bitmap_flip(swap_bitmap, used_index);

  size_t i;
  for (i = 0; i < SECTORS_PER_PAGE; i++)
  {
    block_read(swap_block, used_index * SECTORS_PER_PAGE + i,
               (uint8_t *) frame + i * BLOCK_SECTOR_SIZE);
  }
  lock_release(&swap_lock);
}
