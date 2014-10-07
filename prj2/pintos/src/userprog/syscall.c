#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  switch(*f->esp)
  {
    /* Halt the operating system. */
    case SYS_HALT:                  
      shutdown_power_off();
      break;
    
    /* Terminate this process. */
    case SYS_EXIT:  
      *f->esp
      break;  
    
    /* Start another process. */
    case SYS_EXEC:                  
      
      break;
    
    /* Wait for a child process to die. */
    case SYS_WAIT:                 

      break;
    
    /* Create a file. */
    case SYS_CREATE:                

      break;
    
    /* Delete a file. */
    case SYS_REMOVE:           

      break;     
    
    /* Open a file. */
    case SYS_OPEN:          

      break;       
    
    /* Obtain a file's size. */
    case SYS_FILESIZE:          

      break;     
    
    /* Read from a file. */
    case SYS_READ:            

      break;    
    
    /* Write to a file. */  
    case SYS_WRITE:            

      break;     
    
    /* Change position in a file. */
    case SYS_SEEK:

      break;
    
    /* Report current position in a file. */
    case SYS_TELL:                  

      break;
    
    /* Close a file. */
    case SYS_CLOSE:

      break;

    default:
      break;
  }
  printf ("system call!\n");
  thread_exit ();
}
