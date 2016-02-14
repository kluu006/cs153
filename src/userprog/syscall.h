#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void exit(int status);
int write(int fd, const void *buffer, unsigned size);
//static void copy_in (void *dst_, const void *usrc_, size_t size); 
#endif /* userprog/syscall.h */
