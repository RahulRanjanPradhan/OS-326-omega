#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "filesys/file.h"

struct lock filesys_lock;

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

int wait(pid_t pid)
{
	return process_wait(pid);
}

struct file* process_get_file(int fd)
{
	return NULL;
}
int write(int fd, const void *buffer, unsigned size) 
{
	//Fd=1(STDOUT_FILENO) writes to the console. 
	if(fd == STDOUT_FILENO)
	{
		putbuff(buffer,size);
		return size;
	}
	lock_acquire(&filesys_lock);
	// return file by file descriptor.
	struct file *f = process_get_file(fd);
	if(!file) {
		lock_release(&filesys_lock);
		return ERROR; //-1
	}
	//return number of bytes actually written, maybe less than size if end of file is reached.
		int size = file_read(f, buffer, size);
		lock_release(*filesys_lock);
		return size;
}

		
