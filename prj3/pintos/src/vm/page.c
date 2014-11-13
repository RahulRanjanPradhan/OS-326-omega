#include <string.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"

static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct spt_entry *spte = hash_entry(e, struct spt_entry, elem);
  return hash_int((int) spte->vaddr);
}

static bool page_less_func (const struct hash_elem *a,
                            const struct hash_elem *b,
                            void *aux UNUSED)
{
  struct spt_entry *sa = hash_entry(a, struct spt_entry, elem);
  struct spt_entry *sb = hash_entry(b, struct spt_entry, elem);
  if (sa->vaddr < sb->vaddr)
  {
    return true;
  }
  return false;
}

static void page_destroy_action (struct hash_elem *e, void *aux UNUSED)
{
  struct spt_entry *spte = hash_entry(e, struct spt_entry,
                                      elem);
  if (spte->in_memory)
  {
    frame_free(pagedir_get_page(thread_current()->pagedir, spte->vaddr));
    pagedir_clear_page(thread_current()->pagedir, spte->vaddr);
  }
  free(spte);
}

void page_table_init (struct hash *spt)
{
  hash_init (spt, page_hash_func, page_less_func, NULL);
}

void page_table_destroy (struct hash *spt)
{
  hash_destroy (spt, page_destroy_action);
}



struct spt_entry *get_spte (void *vaddr)
{
  struct spt_entry spte;
  spte.vaddr = pg_round_down(vaddr);

  struct hash_elem *e = hash_find(&thread_current()->spt, &spte.elem);
  if (!e)
  {
    return NULL;
  }
  return hash_entry (e, struct spt_entry, elem);
}

bool load_page (struct spt_entry *spte)
{
  bool success = false;
  spte->locked = true;
  if (spte->in_memory)
  {
    return success;
  }
  switch (spte->type)
  {
  case FILE:
  case MMAP:
    success = load_file(spte);
    break;
  case SWAP:
    success = load_swap(spte);
  default:
    break;
  }
  return success;
}

bool load_swap (struct spt_entry *spte)
{
  uint8_t *frame = frame_alloc (PAL_USER, spte);
  if (!frame)
  {
    return false;
  }
  if (!install_page(spte->vaddr, frame, spte->writable))
  {
    frame_free(frame);
    return false;
  }
  swap_in(spte->swap_index, spte->vaddr);
  spte->in_memory = true;
  return true;
}

bool load_file (struct spt_entry *spte)
{
  enum palloc_flags flags = PAL_USER;
  if (spte->read_bytes == 0)
  {
    flags |= PAL_ZERO;
  }
  uint8_t *frame = frame_alloc(flags, spte);
  if (!frame)
  {
    return false;
  }
  if (spte->read_bytes > 0)
  {
    // lock_acquire(&fs_lock);
    if ((int) spte->read_bytes != file_read_at(spte->file, frame,
        spte->read_bytes,
        spte->offset))
    {
      // lock_release(&fs_lock);
      frame_free(frame);
      return false;
    }
    // lock_release(&fs_lock);
    memset(frame + spte->read_bytes, 0, spte->zero_bytes);
  }

  if (!install_page(spte->vaddr, frame, spte->writable))
  {
    frame_free(frame);
    return false;
  }

  spte->in_memory = true;
  return true;
}

bool add_file_to_pt (struct file *file, int32_t ofs, uint8_t *upage,
                     uint32_t read_bytes, uint32_t zero_bytes,
                     bool writable)
{
  struct spt_entry *spte = malloc(sizeof(struct spt_entry));
  if (!spte)
  {
    return false;
  }
  spte->file = file;
  spte->offset = ofs;
  spte->vaddr = upage;
  spte->read_bytes = read_bytes;
  spte->zero_bytes = zero_bytes;
  spte->writable = writable;
  spte->in_memory = false;
  spte->type = FILE;
  spte->locked = false;

  if (hash_insert(&thread_current()->spt, &spte->elem))
  {
    spte->type = HASH_FAIL;
    return false;
  }
  return true;
}

bool add_mmap_to_pt(struct file *file, int32_t ofs, uint8_t *upage,
                    uint32_t read_bytes, uint32_t zero_bytes)
{
  struct spt_entry *spte = malloc(sizeof(struct spt_entry));
  if (!spte)
  {
    return false;
  }
  spte->file = file;
  spte->offset = ofs;
  spte->vaddr = upage;
  spte->read_bytes = read_bytes;
  spte->zero_bytes = zero_bytes;
  spte->in_memory = false;
  spte->type = MMAP;
  spte->writable = true;
  spte->locked = false;

  struct mmap_file *mf = malloc(sizeof(struct mmap_file));
  if (mf)
  {
    mf->spte = spte;
    mf->mapid = thread_current()->mapid;
    list_push_back(&thread_current()->mmaps, &mf->elem);
  }
  else
  {
    free(spte);
    return false;
  }

  // Return old equal elem in spt. So old should be null.
  if (hash_insert(&thread_current()->spt, &spte->elem))
  {
    spte->type = HASH_FAIL;
    return false;
  }
  return true;
}

bool grow_stack (void *vaddr)
{
  if ( (size_t) (PHYS_BASE - pg_round_down(vaddr)) > MAX_STACK_SIZE)
  {
    return false;
  }
  struct spt_entry *spte = malloc(sizeof(struct spt_entry));
  if (!spte)
  {
    return false;
  }
  spte->vaddr = pg_round_down(vaddr);
  spte->in_memory = true;
  spte->writable = true;
  spte->type = SWAP;
  spte->locked = true;

  uint8_t *frame = frame_alloc (PAL_USER, spte);
  if (!frame)
  {
    free(spte);
    return false;
  }

  if (!install_page(spte->vaddr, frame, spte->writable))
  {
    free(spte);
    frame_free(frame);
    return false;
  }

  if (intr_context())
  {
    spte->locked = false;
  }

  return (hash_insert(&thread_current()->spt, &spte->elem) == NULL);
}