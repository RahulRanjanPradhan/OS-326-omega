#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"

struct child_process {
  int pid;
  bool wait;
  bool exit;
  int status;
  struct list_elem elem;
};

struct file_desc {
  int fd;
  struct file *file;
  struct list_elem elem;
};





void syscall_init (void);

struct child_process* add_child_process (int);
struct child_process* get_child_process(int);
void remove_child_process(struct child_process *);
void remove_child_processes (void);


#endif /* userprog/syscall.h */
