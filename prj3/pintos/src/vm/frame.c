#include "vm/frame.h"

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
   compare hash elements using frame_hash. */
void frame_table_init (void)
{
	lock_init(&frame_table_lock);
	hash_init (&frame_table, frame_hash, frame_hash, NULL);
}

void* frame_alloc (enum palloc_flags flags, struct sub_page_entry *spe)
{
	
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


/* Add a pointer of frame to sub_page_table*/
void frame_add_to_table (void *frame, struct sub_page_entry *spe)
{
	struct frame_entry *fe = malloc(sizeof(struct frame_entry));
	fe->frame = frame;
	fe->spe = spe;
	fe->thread = thread_current();
	lock_acquire(&frame_table_lock);
	hash_insert (&frame_table, fe->hash_elem);
	lock_release(&frame_table_lock);
}


void* frame_evict (enum palloc_flags flags)
{

}
