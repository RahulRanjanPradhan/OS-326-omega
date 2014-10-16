#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"

#define UNLOAD 0
#define LOAD_SUCCESS 1
#define LOAD_FAIL -1


struct child_process {
  int pid;
  bool wait;
  bool exit;
  int status;
  int load;
  struct semaphore sema;
  struct list_elem elem;
};

struct file_desc {
  int fd;
  struct file *file;
  struct list_elem elem;
};

void syscall_init (void);

void check_ptr(const void *);
void check_string(const void *);
void check_buffer(const void *, unsigned );
void check_content(const char **);
void exit(int);

struct child_process* add_child_process (int);
struct child_process* get_child_process(int);
void remove_child_process(struct child_process *);
void remove_child_processes (void);


#endif /* userprog/syscall.h */
