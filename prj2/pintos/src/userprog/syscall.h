#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"

struct child_process {
  int pid;
  int load;
  bool wait;
  bool exit;
  int status;
  struct lock wait_lock;
  struct list_elem elem;
};

struct process_file {
  struct file *file;
  int fd;
  struct list_elem elem;
};





void syscall_init (void);
struct child_process* get_child_process(int);
void remove_child_process(struct child_process *);


#endif /* userprog/syscall.h */
