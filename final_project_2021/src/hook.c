/*
 *  hook.c  --  hook linux syscall
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>// for kallsyms_lookup_name
#include <linux/syscalls.h>// for definition of sys_clone
#include <asm/tlbflush.h>// flush_tlb_kernel_range()

#include <linux/uaccess.h>
#include <linux/mm_types.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hook linux syscall");
MODULE_VERSION("0.01");

void **p_sys_call_table;
void (*set_kernel_text_rw)(void);
void (*set_kernel_text_ro)(void);
int sys_call_num = __NR_clone;//__NR_clone
pte_t * sys_call_table_pte;//pte of sys_call_table

// Copied from syscall.h
// Define a func pointer that has the same prototype as original sys_clone.
/* arch/example/kernel/sys_example.c */
#ifdef CONFIG_CLONE_BACKWARDS
asmlinkage long (*original_sys_clone)(unsigned long, unsigned long, int __user *, unsigned long,
	       int __user *);
asmlinkage long my_sys_clone(unsigned long p1, unsigned long p2, int __user * p3, unsigned long p4,
	       int __user * p5)
           {
               printk(KERN_INFO "hello，I have hacked this syscall: %d\n",sys_call_num);
               return original_sys_clone(p1,p2,p3,p4,p5);
           }
#else



#ifdef CONFIG_CLONE_BACKWARDS3
asmlinkage long (*original_sys_clone)(unsigned long, unsigned long, int, int __user *,
			  int __user *, unsigned long);
asmlinkage long my_sys_clone(unsigned long p1, unsigned long p2, int p3, int __user * p4,
			  int __user * p5, unsigned long p6)
           {
               printk(KERN_INFO "hello，I have hacked this syscall: %d\n",sys_call_num);
               return original_sys_clone(p1,p2,p3,p4,p5,p6);
           }
#else
asmlinkage long (*original_sys_clone)(unsigned long, unsigned long, int __user *,
	       int __user *, unsigned long);
asmlinkage long my_sys_clone(unsigned long p1, unsigned long p2, int __user * p3,
	       int __user * p4, unsigned long p5)
           {
               printk(KERN_INFO "hello，I have hacked this syscall: %d\n",sys_call_num);
               return original_sys_clone(p1,p2,p3,p4,p5);
           }
#endif
#endif 

// Copied from lab3 mtest
/* find page of va */
static pte_t * _find_page(unsigned long vaddr) {
    struct mm_struct *mm = (struct mm_struct *)kallsyms_lookup_name("init_mm");
    //struct page *curr_page;

    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    //printk(KERN_INFO "vaddr: %lx\n",vaddr);
    //printk(KERN_INFO "mm: %lx\n",(unsigned long)mm);

    // walk the page table
    // 1. get [page global directory, pgd]
    pgd = pgd_offset(mm, vaddr);
        // printk("pgd: %llx\n", pgd_val(*pgd));
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk("[pgd] not available\n");
        return NULL;
    }
    // 2. get [page upper directory, pud]
    pud = pud_offset(pgd, vaddr);
        // printk("pud: %llx\n", pud_val(*pud));
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk("[pud] not available\n");
        return NULL;
    }
    // 3. get [page middle directory, pmd]
    pmd = pmd_offset(pud, vaddr);
        // printk("pmd: %llx\n", pmd_val(*pmd));
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk("[pmd] not available\n");
        return NULL;
    }
    // 4. get [page table entry, pte]
    pte = pte_offset_kernel(pmd, vaddr);
        // printk("pte: %llx\n", pte_val(*pte));
    if (pte_none(*pte)) {
        printk("[pte] not available\n");
        return NULL;
    }
    
    //curr_page = pte_page(*pte);
    //return curr_page;
    return pte;
}

static int __init hook_init(void) {
    /* Aquire system calls table address */
    p_sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
    set_kernel_text_rw = (void *) kallsyms_lookup_name("set_kernel_text_rw");
    set_kernel_text_ro = (void *) kallsyms_lookup_name("set_kernel_text_ro");
    
    //printk(KERN_INFO "sys_call_num: %d\n",sys_call_num);
    //printk(KERN_INFO "Got sys_call_table address: %lx\n",(long unsigned int)p_sys_call_table);
    
    original_sys_clone = p_sys_call_table[sys_call_num];

    //printk(KERN_INFO "p_sys_call_table + sys_call_num: %lx\n",(unsigned long)(p_sys_call_table + sys_call_num));

    sys_call_table_pte = _find_page((unsigned long)(p_sys_call_table + sys_call_num));
    if(sys_call_table_pte == NULL){
        printk(KERN_INFO "sys_call_table_pte is NULL\n");
        return 0;
    }

    // Temporarily disable write protection
    *sys_call_table_pte = pte_mkwrite(*sys_call_table_pte);
    flush_tlb_kernel_range((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK, ((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK) + PAGE_SIZE);
    // Overwrite the syscall table entry
    p_sys_call_table[sys_call_num] = my_sys_clone;
    // Re-enable write protection
    *sys_call_table_pte = pte_wrprotect(*sys_call_table_pte);
    flush_tlb_kernel_range((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK, ((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK) + PAGE_SIZE);
    return 0;
}

static void __exit hook_exit(void) {
    // Temporarily disable write protection
    *sys_call_table_pte = pte_mkwrite(*sys_call_table_pte);
    flush_tlb_kernel_range((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK, ((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK) + PAGE_SIZE);
    // Overwrite the syscall table entry
    p_sys_call_table[sys_call_num] = original_sys_clone;
    // Re-enable write protection
    *sys_call_table_pte = pte_wrprotect(*sys_call_table_pte);
    flush_tlb_kernel_range((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK, ((unsigned long)(p_sys_call_table + sys_call_num) & PAGE_MASK) + PAGE_SIZE);
    printk(KERN_INFO "Syscall recovered!\n");
}

module_init(hook_init);
module_exit(hook_exit);
//module_param(sys_call_num, int, 0644);