#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"

#include "lib/user/syscall.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"

#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "threads/synch.h"

#define FILE_ERROR -1;


struct lock filesys_lock;
static void syscall_handler (struct intr_frame *);


struct file *process_get_file(int fd);  //return file by file descriptor


void
syscall_init (void)
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  uint32_t *p = f->esp;
  check_ptr(p);
  switch (*(int *)p)
  {
  /* Halt the operating system.
    IN : void
    OUT: void
  */
  case SYS_HALT:
  {
    shutdown_power_off();
    break;
  }

  /* Terminate this process.
    IN : int status
    OUT: void
  */
  case SYS_EXIT:
  {
    check_ptr(p + 1);
    int status = *(int *)(p + 1);
    exit(status);
    break;
  }


  /* Start another process.
    IN : const char *file
    OUT: pid_t
  */
  case SYS_EXEC:
  {
    check_string(p + 1);
    f->eax = process_execute(*(char **)(p + 1));
    break;
  }

  /* Wait for a child process to die.
    IN : pid_t
    OUT: int
  */
  case SYS_WAIT:
  {
    check_ptr(p + 1);
    f->eax = process_wait(*(tid_t *)(p + 1));
    break;
  }

  /* Create a file.
    IN :const char *file, unsigned initial_size
    OUT:bool
  */
  case SYS_CREATE:
  {
    check_string(p + 1);
    check_ptr(p + 2);
    if(*(char**)(p+1)==NULL||*(int *)(p+1)>=PHYS_BASE||
      pagedir_get_page(thread_current()->pagedir, (const void *)*(p+1)) == NULL)
      exit(-1);
    lock_acquire(&filesys_lock);
    f->eax = filesys_create(*(const char **)(p + 1), *(off_t *)(p + 2));
    lock_release(&filesys_lock);
    break;
  }

  /* Delete a file.
    IN: const char *file
    OUT: bool
  */
  case SYS_REMOVE:
  {

    check_string(p + 1);
    if(*(char**)(p+1)==NULL||*(int *)(p+1)>=PHYS_BASE||
      pagedir_get_page(thread_current()->pagedir, (const void *)*(p+1)) == NULL)
      exit(-1);
    lock_acquire(&filesys_lock);
    f->eax = filesys_remove(*(char**)(p + 1));
    lock_release(&filesys_lock);
    break;
  }

  /* Open a file.
    IN: const char *file
    OUT: int
  */
  case SYS_OPEN:
  {
    check_string(p + 1);
    lock_acquire(&filesys_lock);
    struct file *file = filesys_open(*(char**)(p + 1));
    if (file == NULL)
    {
      f->eax = -1;
      break;
    }
    struct file_desc *desc = malloc(sizeof(struct file_desc));
    desc->file = file;
    desc->fd = thread_current()->fd;
    thread_current()->fd++;
    list_push_back(&thread_current()->file_list, &desc->elem);
    f->eax = desc->fd;
    lock_release(&filesys_lock);
    break;
  }

  /* Obtain a file's size.
    IN: int fd
    OUT: int
  */
  case SYS_FILESIZE:
  {
    check_ptr(p + 1);
    int fd = *(p + 1);
    lock_acquire(&filesys_lock);
    struct file *file = process_get_file(fd);
    if (file == NULL)
    {
      f->eax = FILE_ERROR;
    }
    else
    {
      f->eax = file_length(file);
    }
    lock_release(&filesys_lock);
    break;
  }

  /* Read from a file.
    IN: int fd, void *buffer, unsigned length
    OUT: int
  */
  case SYS_READ:
  {
    check_ptr(p + 1);
    check_ptr(p + 2);
    check_ptr(p + 3);
    check_buffer(p + 2, *(p + 3));

    int fd = *(int *)(p + 1);
    void *buffer = *(char**)(p + 2);
    unsigned length = *(unsigned *)(p + 3);

    //read from keyboard. Fd 0 reads from keyboard using input_getc(): one each time.
    if (fd == STDIN_FILENO)
    {
      uint8_t *into_buffer = (uint8_t *) buffer;
      unsigned i;
      for (i = 0; i < length; i++)
      {
        into_buffer[i] = input_getc();
      }
      f->eax = length;
    }
    else
    {
      //read from file into buffer
      lock_acquire(&filesys_lock);
      struct file *file = process_get_file(fd);
      //return -1 if file couldn't be read.
      if (file == NULL)
      {
        lock_release(&filesys_lock);
        f->eax = FILE_ERROR;  //-1
      }
      else
      {
        int bytes = file_read(file, buffer, length);
        lock_release(&filesys_lock);
        f->eax = bytes;
      }

    }
    break;
  }

  /* Write to a file.
    IN: int fd, const void *buffer, unsigned length
    OUT: int
  */
  case SYS_WRITE:
  {
    check_ptr(p + 1);
    check_ptr(p + 2);
    check_ptr(p + 3);
    check_buffer(p + 2, *(p + 3));

    int fd = *(int *)(p + 1);
    void *buffer = (void *)*(p + 2);
    unsigned length = *(off_t *)(p + 3);

    if (length <= 0)
    {
      f->eax = 0;
      break;
    }
    //Fd=1(STDOUT_FILENO) writes to the console.
    if (fd == STDOUT_FILENO)
    {
      putbuf(buffer, length);
      f->eax = length;
    }
    else
    {
      lock_acquire(&filesys_lock);
      // return file by file descriptor.
      struct file *file = process_get_file(fd);
      if (file == NULL)
      {
        lock_release(&filesys_lock);
        f->eax = FILE_ERROR;  //-1
      }
      else
      {
        //return number of bytes actually written,
        //maybe less than length if end of file is reached.
        int bytes = file_write(file, buffer, length);
        lock_release(&filesys_lock);
        f->eax = bytes;
      }
    }
    break;
  }

  /* Change position in a file.
    IN: int fd, unsigned position
    OUT: void
  */
  case SYS_SEEK:
  {
    check_ptr(p + 1);
    check_ptr(p + 2);
    int fd = *(int *)(p+1);
    unsigned position = *(unsigned *)(p+2);
    lock_acquire(&filesys_lock);
    struct file *file = process_get_file(fd);
    if(file != NULL)
    {
      file_seek(file, position);
    }
    lock_release(&filesys_lock);
    break;
  }

  /* Report current position in a file.
    IN: int fd
    OUT: unsigned
  */
  case SYS_TELL:
  {
    check_ptr(p + 1);
    int fd = *(int *)(p+1);

    lock_acquire(&filesys_lock);
    struct file *file = process_get_file(fd);
    if(file != NULL)
    {
      f->eax = file_tell(file);
    }
    else
      f->eax = FILE_ERROR;  //-1
    lock_release(&filesys_lock);
    break;
  }

  /* Close a file.
    IN: int fd
    OUT: void
  */
  case SYS_CLOSE:
  {
    check_ptr(p+1);
    int fd = *(int*)(p+1);
    lock_acquire(&filesys_lock);
    struct thread *t = thread_current();
    struct list_elem *next, *e = list_begin(&t->file_list);
    while (e != list_end (&t->file_list))
    {
      next = list_next(e);
      struct file_desc *fl = list_entry (e, struct file_desc, elem);
      if (fd == fl->fd)
      {
        file_close(fl->file);
        list_remove(&fl->elem);
        free(fl);
        break;
      }
      e = next;
    }
    lock_release(&filesys_lock);
    break;
  }

  default:
    break;
  }

}


void
exit(int status)
{
  printf ("%s: exit(%d)\n", thread_current()->name, status);
  struct thread *t = thread_current ();
  if (thread_alive(t->parent))
  {
    t->cp->exit = true;
    t->cp->status = status;
  }
  thread_exit();
}

void
check_ptr(const void *ptr)
{
  if (ptr == NULL || !is_user_vaddr(ptr) ||
      pagedir_get_page(thread_current()->pagedir, ptr) == NULL)
  {
    exit(-1);
  }
}

void
check_string(const void *ptr)
{
  check_ptr(ptr);
  check_ptr(*(char **)ptr);
  int i = 1;
  while (*(char *)(ptr + i) != '\0')
  {
    check_ptr(ptr+i);
    i++;
  }
}

void
check_buffer(const void *ptr, unsigned size)
{
  unsigned i = 0;
  check_ptr(*(char **)ptr);
  for (i = 0; i < size; i++)
  {
    check_ptr(ptr + i);
    check_ptr(*(char **)ptr);
  }
}




struct child_process *add_child_process (int pid)
{
  struct child_process *cp = malloc(sizeof(struct child_process));
  cp->pid = pid;
  // cp->wait = false;
  cp->exit = false;
  list_push_back(&thread_current()->child_list,
                 &cp->elem);
  return cp;
}

struct child_process *get_child_process (int pid)
{
  struct thread *t = thread_current();
  struct list_elem *e; //current thread's children

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

void remove_child_processes (void)
{
  struct thread *t = thread_current();
  struct list_elem *next, *e = list_begin(&t->child_list);

  while (e != list_end (&t->child_list))
  {
    next = list_next(e);
    struct child_process *cp = list_entry (e, struct child_process,
                                           elem);
    list_remove(&cp->elem);
    free(cp);
    e = next;
  }
}

struct file *process_get_file(int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  for (e = list_begin(&t->file_list);
       e != list_end(&t->file_list);
       e = list_next(e))
  {
    struct file_desc *pf = list_entry (e, struct file_desc, elem);
    //if match fd, return the process file.
    if (fd == pf->fd)
    {
      return pf->file;
    }
  }
  return NULL;
}

