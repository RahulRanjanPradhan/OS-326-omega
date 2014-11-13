#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#define CLOSE_ALL -1
#define USER_VADDR_BOTTOM ((void *) 0x08048000)

/* Serializes file system operations. */
struct lock fs_lock;

void syscall_init (void);
void syscall_exit (void);

#endif /* userprog/syscall.h */
