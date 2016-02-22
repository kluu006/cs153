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
#include <syscall-nr.h>


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

}
/*
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}*/

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

void sys_exit(int status){
	struct thread *cur = thread_current();
	printf("%s: exit(%i)\n", cur->name, status);
	thread_exit();
}

int wait(int waiter_id){
	return -1;
}

//waht types are those args in the test files?
//maybe int, ????, and int??
//jk found on manpage for write, unless we cant use that
int sys_write(int fd, const void *buff, unsigned count){
	if(!verify_user(buff)){
		sys_exit(-1);
	}
	if(fd == STDOUT_FILENO){
		putbuf(buff, count);
		return count;
	}
	return 1;
}
static void
syscall_handler (struct intr_frame *f) 
{
	unsigned callNum;
	int args[3];
	int numOfArgs;
   
    if(!verify_user(f->esp)){
    	sys_exit(-1);
    }
  
	copy_in(&callNum, f->esp, sizeof callNum);

	if(callNum == SYS_WRITE){
		numOfArgs = 3;
		if(!verify_user(f->esp+1) || !verify_user(f->esp+2) || !verify_user(f->esp+3)){
    		sys_exit(-1);
    	}
	}
	else if(callNum == SYS_EXIT){
		numOfArgs = 1;
		if(!verify_user(f->esp+1)){
    		sys_exit(-1);
    	}
	}

	copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * numOfArgs);

	if(callNum == SYS_WRITE){
		f->eax = sys_write(args[0], args[1], args[2]);
	}
	else if(callNum == SYS_EXIT){
		//void so no f->eax
		sys_exit(args[0]);
	}
	//this will make many test cases fail for whatever reason
	/*else{
		thread_exit();
	}*/
}
