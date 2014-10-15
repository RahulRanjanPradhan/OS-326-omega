#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
<<<<<<< Updated upstream
#include "userprog/process.h"
#include "filesys/file.h"

struct lock filesys_lock;
=======
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
>>>>>>> Stashed changes

static void syscall_handler (struct intr_frame *);
struct file* process_get_file(int fd);  //return file by file descriptor

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  printf ("system call!\n");
  thread_exit ();
}

<<<<<<< Updated upstream
int wait(pid_t pid)
{
	return process_wait(pid);
}

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
int write(int fd, const void *buffer, unsigned size) 
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
	if(!file) {
		lock_release(&filesys_lock);
		return ERROR;  //-1
	}
	//return number of bytes actually written, maybe less than size if end of file is reached.
		int bytes = file_read(f, buffer, size);
		lock_release(*filesys_lock);
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
	lock_acqure(&filesys_lock);
	struct file *f = process_get_file(fd);
	//return -1 if file couldn't be read.
	if(!f) {
		lock_release(&filesys_lock);
		return ERROR;
	}
	int bytes = file_read(f,buffer,size);
	lock_release(&filesys_lock);
	return bytes;
}
		
=======
void
check_ptr(const void *ptr)
{
  if(ptr == NULL || !is_user_vaddr(ptr) ||
     pagedir_get_page(thread_current()->pagedir, ptr) == NULL)
  {
    thread_exit();
  }
}

void
check_string(void *string, unsigned len)
{
  void *string_end = string + len - 1;
  check_ptr(string);
  check_ptr(string_end);
}

void
check_buffer (void *buffer, unsigned size)
{
  void *buffer_end = buffer + size -1;
  check_ptr(buffer);
  check_ptr(buffer_end);
}
>>>>>>> Stashed changes
