#include "userprog/syscall.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Copies a byte from user address USRC to kernel address DST.
   USRC must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool
get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
       : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}

/* Creates a copy of user string US in kernel memory
   and returns it as a page that must be freed with
   palloc_free_page().
   Truncates the string at PGSIZE bytes in size.
   Call thread_exit() if any of the user accesses are invalid. */
static char *
copy_in_string (const char *us) 
{
  char *ks;
  size_t length;
 
  ks = palloc_get_page (0);
  if (ks == NULL) 
    thread_exit ();
 
  for (length = 0; length < PGSIZE; length++)
    {
      if (us >= (char *) PHYS_BASE || !get_user (ks + length, us++)) 
        {
          palloc_free_page (ks);
          thread_exit (); 
        }
       
      if (ks[length] == '\0')
        return ks;
    }
  ks[PGSIZE - 1] = '\0';
  return ks;
}


/* Returns true if UADDR is a valid, mapped user address,
   false otherwise. */
static bool
verify_user (const void *uaddr) 
{
  return (uaddr < PHYS_BASE
          && pagedir_get_page (thread_current ()->pagedir, uaddr) != NULL);
}


static void
copy_in (void *dst_, const void *usrc_, size_t size) 
{
  uint8_t *dst = dst_;
  const uint8_t *usrc = usrc_;
 
  for (; size > 0; size--, dst++, usrc++) 
    if (usrc >= (uint8_t *) PHYS_BASE || !get_user (dst, usrc)) 
      thread_exit ();
}

void exit(int status){
	struct thread *cur = thread_current();
	//if(thread_alive(cur->parent)){
	//	cur->cp->status = status;
	//}
	printf("%s: exit(%d)\n", "butts", status);
	thread_exit();
}

int wait(int waiter_id){
	/*
	struct child_process* cp = get_child_process(waiter_id);
	cp->wait = true;
	while(!cp->exit){}
	int status = cp->status;
	remove_child_process(cp);
	return status;
	*/
	return -1;
}

//waht types are those args in the test files?
//maybe int, ????, and int??
//jk found on manpage for write, unless we cant use that
int write(int fd, const void *buff, unsigned count){
  //whats this do?
  //get_arg(f, &fd, 3);

  //need to define check for valid buffer
  //check_valid_buffer((void *) buff, (unsigned) count);\
  //need to implement user_to_kernel_ptr
  //buff = user_to_kernel_ptr((const void *) buff);
  return write(fd, (const void *) buff, (unsigned) count);
  //break;
}
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  unsigned callNum;
  //3 because arg amount capped at 3?
  int args[3];
  int numOfArgs;

  //make sure user passed in pointers are valid
  //so validate f->esp+1 maybe? since copy_in uses +1
  if(!verify_user(f->esp) || !verify_user(f->esp + 1)){
    exit(-1);
  }
  //Get syscall number
  copy_in(&callNum, f->esp, sizeof callNum);

  //use if statements to choose which to use
  if(callNum == SYS_EXIT){
  	  numOfArgs = 1;
  	  copy_in(args, (uint32_t*) f->esp+1, sizeof *args * numOfArgs);
  	  //no f->eax since void
      exit(args[0]);
  }
  else if(callNum == SYS_WAIT){
  	  numOfArgs = 1;
  	  copy_in(args, (uint32_t*) f->esp+1, sizeof *args * numOfArgs);
      f->eax = wait(args[0]);
  }
  else if(callNum == SYS_WRITE){
  	  numOfArgs = 3;
  	  copy_in(args, (uint32_t*) f->esp+1, sizeof *args * numOfArgs);
      f->eax = write(args[0], args[1], args[2]);
  }
  printf ("system call!\n");
  thread_exit ();
}
