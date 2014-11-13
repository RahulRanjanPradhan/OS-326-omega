#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "threads/thread.h"

static struct hash frame_table;
static struct lock frame_table_lock;

/* Returns a hash value for frame f. */
static unsigned frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  const struct frame_entry *f = hash_entry (f_, struct frame_entry, hash_elem);
  return hash_bytes (&f->frame, sizeof f->frame);
}

/* Returns true if frame a precedes frame b. */
static bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
                        void *aux UNUSED)
{
  const struct frame_entry *a = hash_entry (a_, struct frame_entry, hash_elem);
  const struct frame_entry *b = hash_entry (b_, struct frame_entry, hash_elem);

  return a->frame < b->frame;
}

/* Initialize frame table to compute hash values using frame_hash and
   compare hash elements using frame_less. */
void frame_table_init (void)
{
  lock_init(&frame_table_lock);
  hash_init (&frame_table, frame_hash, frame_less, NULL);
}

/* Allocate a frame for a page, if frame table is full, evict one.
 * Otherwise, add page to the frame table*/
void *frame_alloc (enum palloc_flags flags, struct spt_entry *spte)
{
  if ( (flags & PAL_USER) == 0 )
  {
    return NULL;
  }
  void *frame = palloc_get_page(flags);
  if (frame)
  {
    frame_add_to_ft(frame, spte);
  }
  else
  {
    while (!frame)
    {
      frame = frame_evict(flags);
      lock_release(&frame_table_lock);
    }
    frame_add_to_ft(frame, spte);
  }
  return frame;
}

/* Finds, removes, and returns an element, which frame is equal to frame
 * in frame table. Returns a null pointer if no equal element existed in the table.
 * Finally free the frame_entry.
 */
void frame_free (void *frame)
{
  struct frame_entry f;
  struct hash_elem *f_elem;
  lock_acquire(&frame_table_lock);

  f.frame = frame;
  f_elem = hash_find (&frame_table, &f.hash_elem);

  // Need process empty list ?
  if (f_elem != NULL)
  {
    hash_delete(&frame_table, f_elem);
    struct frame_entry *f_entry = hash_entry(f_elem, struct frame_entry, hash_elem);
    free(f_entry);
    palloc_free_page(frame);
  }
  lock_release(&frame_table_lock);
}


/* Add a pointer which points to sub_page_table to frame_table*/
void frame_add_to_ft (void *frame, struct spt_entry *spte)
{
  struct frame_entry *fe = malloc(sizeof(struct frame_entry));
  fe->frame = frame;
  fe->spte = spte;
  fe->thread = thread_current();
  lock_acquire(&frame_table_lock);
  hash_insert (&frame_table, &fe->hash_elem);
  lock_release(&frame_table_lock);
}

/* Use second chance algorithm to do the page replacement,
 * If swap is full, panic the kernel.
 */
void *frame_evict (enum palloc_flags flags)
{
  lock_acquire(&frame_table_lock);
  struct hash_iterator i;
  hash_first (&i, &frame_table);
  while (hash_next (&i))
  {
    struct frame_entry *fte = hash_entry (hash_cur (&i), struct frame_entry, hash_elem);
    if (!fte->spte->locked)
    {
      struct thread *t = fte->thread;
      if (pagedir_is_accessed(t->pagedir, fte->spte->vaddr))
      {
        pagedir_set_accessed(t->pagedir, fte->spte->vaddr, false);
      }
      else
      {
        if (pagedir_is_dirty(t->pagedir, fte->spte->vaddr) ||
            fte->spte->type == SWAP)
        {
          if (fte->spte->type == MMAP)
          {
            lock_acquire(&fs_lock);
            file_write_at(fte->spte->file, fte->frame,
                          fte->spte->read_bytes,
                          fte->spte->offset);
            lock_release(&fs_lock);
          }
          else
          {
            fte->spte->type = SWAP;
            fte->spte->swap_index = swap_out(fte->frame);
          }
        }
        fte->spte->in_memory = false;
        hash_delete(&frame_table, &fte->hash_elem);
        pagedir_clear_page(t->pagedir, fte->spte->vaddr);
        palloc_free_page(fte->frame);
        free(fte);
        return palloc_get_page(flags);
      }
    }
  }
}
