#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
<<<<<<< Updated upstream
#include "threads/vaddr.h"

#include "lib/user/syscall.h"
#include "devices/shutdown.h"
=======
>>>>>>> Stashed changes
#include "userprog/process.h"
#include "userprog/pagedir.h"

#include "filesys/file.h"
<<<<<<< HEAD
<<<<<<< Updated upstream
=======
#include "devices/input.h"
#include "threads/synch.h"
>>>>>>> FETCH_HEAD

struct lock filesys_lock;

/* Typical return values from wait(). */
#define WAIT_SUCCESS 0          /* Successful wait. */
<<<<<<< HEAD
#define WAIT_FAILURE 1          /* Unsuccessful wait. */
=======
#include "threads/vaddr.h"
#include "userprog/pagedir.h"



>>>>>>> Stashed changes
=======
#define WAIT_FAILURE -1          /* Unsuccessful wait. */

>>>>>>> FETCH_HEAD

static void syscall_handler (struct intr_frame *);
void check_ptr(const void *);
void check_string(const void *);
void check_buffer(const void *, unsigned );


struct file* process_get_file(int fd);  //return file by file descriptor


void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  uint32_t *p = f->esp;
  check_ptr(p);
  switch(*p)
  {
    /* Halt the operating system. 
      IN : void
      OUT: void
    */
    case SYS_HALT:
      shutdown_power_off();
      break;
<<<<<<< Updated upstream
    
    /* Terminate this process. 
      IN : int status
      OUT: void
    */  
=======

    /* Terminate this process. */
>>>>>>> Stashed changes
    case SYS_EXIT:

      check_ptr(p+1);
      printf ("%s: exit(%d)\n", thread_current()->name, *(int *)p+1);
      thread_exit();
      break;
<<<<<<< Updated upstream
    
    /* Start another process. 
      IN : const char *file
      OUT: pid_t
    */  
=======

    /* Start another process. */
>>>>>>> Stashed changes
    case SYS_EXEC:

      check_string(p+1);
      f->eax = process_execute((char *)(p+1));
      break;
<<<<<<< Updated upstream
    
    /* Wait for a child process to die. 
      IN : pid_t
      OUT: int
    */  
=======

    /* Wait for a child process to die. */
>>>>>>> Stashed changes
    case SYS_WAIT:
      check_ptr(p+1);
      f->eax = process_wait(*(tid_t *)(p+1));
      break;
<<<<<<< Updated upstream
    
    /* Create a file. 
      IN :const char *file, unsigned initial_size
      OUT:bool
    */
    case SYS_CREATE:
      check_ptr(p+1);
      filesys_create()
      break;
    
    /* Delete a file. 
      IN: const char *file
      OUT: bool
    */  
=======

    /* Create a file. */
    case SYS_CREATE:
      break;

    /* Delete a file. */
>>>>>>> Stashed changes
    case SYS_REMOVE:

      break;
<<<<<<< Updated upstream
    
    /* Open a file. 
      IN: const char *file
      OUT: int
    */
    case SYS_OPEN:          

      break;       
    
    /* Obtain a file's size. 
      IN: int fd
      OUT: int
    */
    case SYS_FILESIZE:          

      break;     
    
    /* Read from a file. 
      IN: int fd, void *buffer, unsigned length
      OUT: int
    */
    case SYS_READ:            

      break;    
    
    /* Write to a file. 
      IN: int fd, const void *buffer, unsigned length
      OUT: int
    */  
    case SYS_WRITE:            
      // get_arg(f, arg, 3);
      break;     
    
    /* Change position in a file. 
      IN: int fd, unsigned position
      OUT: void
    */
=======

    /* Open a file. */
    case SYS_OPEN:
      // get_arg(f, arg, 1);
      break;

    /* Obtain a file's size. */
    case SYS_FILESIZE:
      // get_arg(f, arg, 1);
      break;

    /* Read from a file. */
    case SYS_READ:
      // get_arg(f, arg, 3);
      break;

    /* Write to a file. */
    case SYS_WRITE:
      // get_arg(f, arg, 3);
      break;

    /* Change position in a file. */
>>>>>>> Stashed changes
    case SYS_SEEK:

      break;
<<<<<<< Updated upstream
    
    /* Report current position in a file. 
      IN: int fd
      OUT: unsigned
    */
=======

    /* Report current position in a file. */
>>>>>>> Stashed changes
    case SYS_TELL:

      break;
<<<<<<< Updated upstream
    
    /* Close a file. 
      IN: int fd
      OUT: void
    */
=======

    /* Close a file. */
>>>>>>> Stashed changes
    case SYS_CLOSE:

      break;

    default:
      break;
  }
  printf ("system call!\n");
  thread_exit ();
}


struct process_file {
  struct file *file;
  int fd;
  struct list_elem elem;
};
struct file* process_get_file(int fd)
{
	struct thread *t = thread_current();
	struct list_elem *e;
	for(e = list_begin(&t->file_list); e!=list_end(&t->file_list);e=list_next(e)) {
		 struct process_file *pf = list_entry (e, struct process_file, elem);
		 //if match fd, return the process file.
          if (fd == pf->fd)
	    {
	      return pf->file;
	    }
	}
	return NULL;
}
<<<<<<< HEAD
int write(int fd, const void *buffer, unsigned size)
=======
// ../lib/user/syscall.h
int write(int fd, const void *buffer, unsigned size) 
>>>>>>> FETCH_HEAD
{
	//Fd=1(STDOUT_FILENO) writes to the console.
	if(fd == STDOUT_FILENO)
	{
		putbuf(buffer,size);
		return size;
	}
	lock_acquire(&filesys_lock);
	// return file by file descriptor.
	struct file *f = process_get_file(fd);
	if(!f) {
		lock_release(&filesys_lock);
		return WAIT_FAILURE;  //-1
	}
	//return number of bytes actually written, maybe less than size if end of file is reached.
		int bytes = file_read(f, buffer, size);
		lock_release(&filesys_lock);
		return bytes;
}

int read(int fd, void *buffer,unsigned size)
{
	//read from keyboard. Fd 0 reads from keyboard using input_getc(): one each time.
	if(fd == STDIN_FILENO){
		uint8_t *into_buffer = (uint8_t *) buffer;
		unsigned i;
		for(i = 0;i<size;i++)
		{
			into_buffer[i] = input_getc();
		}
		return size;
	}

		//read from file into buffer
	lock_acquire(&filesys_lock);
	struct file *f = process_get_file(fd);
	//return -1 if file couldn't be read.
	if(!f) {
		lock_release(&filesys_lock);
		return WAIT_FAILURE;
	}
	int bytes = file_read(f,buffer,size);
	lock_release(&filesys_lock);
	return bytes;
}
<<<<<<< HEAD
<<<<<<< Updated upstream
*/
=======

>>>>>>> FETCH_HEAD

=======

=======
>>>>>>> Stashed changes
void
check_ptr(const void *ptr)
{
  if(ptr == NULL || !is_user_vaddr(ptr) ||
     pagedir_get_page(thread_current()->pagedir, ptr) == NULL)
  {
    printf ("%s: exit(%d)\n", thread_current()->name, EXIT_FAILURE);
    thread_exit();
  }
}

void
<<<<<<< Updated upstream
check_string(const void *ptr)
{
  check_ptr(ptr);
  int i = 1;
  while(*(char *)(ptr + i) != '/0')
  {
    check_ptr(ptr+i);
    i++;
  }

}

void
check_buffer(const void *ptr, unsigned size)
{
  unsigned i = 0;
  for(i = 0; i < size; i++)
  {
    check_ptr(ptr+i);
  }
}

struct child_process* get_child_process (int pid)
{
  struct thread *t = thread_current();
  struct list_elem *e; //current running thread's children

  for (e = list_begin (&t->child_list); e != list_end (&t->child_list);
       e = list_next (e))
        {
          struct child_process *cp = list_entry (e, struct child_process, elem);
          if (pid == cp->pid)
	    {
	      return cp;
	    }
        }
  return NULL;
}

void remove_child_process(struct child_process *cp)
{
	list_remove(&cp->elem);
	free(cp);
}

