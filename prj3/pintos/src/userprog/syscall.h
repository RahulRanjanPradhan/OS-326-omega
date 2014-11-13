#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#define CLOSE_ALL -1
/* Serializes file system operations. */
static struct lock fs_lock;

void syscall_init (void);
void syscall_exit (void);

#endif /* userprog/syscall.h */
