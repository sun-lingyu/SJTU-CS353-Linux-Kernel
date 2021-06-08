#include<linux/module.h>// included for all kernel modules
#include<linux/init.h>// included for __init and __exit macros
#include<linux/proc_fs.h>
#include<linux/seq_file.h>
#include<linux/version.h>
#include<linux/kernel.h>// included for KERN_INFO
#include<linux/slab.h>
#include<linux/string.h>// included for string splitting
#include<linux/sched.h>// included for task_struct
#include<linux/mm_types.h>// included for mm_struct

#include<linux/pgtable.h>// included for page table


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

//copied back from linux5.5.11 source code
/*
//get pgd entry
#define pgd_index(addr)		(((addr) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))

#define pgd_offset_raw(pgd, addr)	((pgd) + pgd_index(addr))

#define pgd_offset(mm, addr)	(pgd_offset_raw((mm)->pgd, (addr)))

#define pgd_none(pgd)		(!pgd_val(pgd))
#define pgd_bad(pgd)		(!(pgd_val(pgd) & 2))

//get pud entry
#define pud_offset(dir, addr)		((pud_t *)__va(pud_offset_phys((dir), (addr))))

//get pmd entry
#define pmd_offset(dir, addr)		((pmd_t *)__va(pmd_offset_phys((dir), (addr))))

//get pte entry
#define pte_offset_kernel(dir,addr)	((pte_t *)__va(pte_offset_phys((dir), (addr))))

#define pte_offset_map(dir,addr)	pte_offset_kernel((dir), (addr))

//???
#define pmd_index(addr)		(((addr) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))
#define pud_index(addr)		(((addr) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))
#define pte_index(addr)		(((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))*/

static int my_proc_show(struct seq_file *m,void *v){
	seq_printf(m,"usage: echo the following cmds to this file:\n\
	listvma: list all vma of current process\n\
	findpage addr: convert addr(vma) to pma and print pma\n\
	writeeval addr val: write val to addr\n");
	return 0;
}

static ssize_t my_proc_write(struct file* file,const char __user *buffer,size_t count,loff_t *f_pos){
	char *tmp = kzalloc((count+1),GFP_KERNEL);
	char* const delim = " ";
	char *token, *cur = tmp;
	struct vm_area_struct *mmap;
	char* read_permission, *write_permission, *exe_permission;

	long unsigned int addr;
	pgd_t *pgd = NULL;
	p4d_t *p4d = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;

	char* val = NULL;
	unsigned long copied;
	unsigned long length;

	if(!tmp)return -ENOMEM;
	if(copy_from_user(tmp,buffer,count)){
		kfree(tmp);
		return EFAULT;
	}
	
	token = strsep(&cur, delim);
	if(!strcmp(token,"listvma\n")){
		printk(KERN_INFO "listvma\n");

		mmap=current->mm->mmap;
		while(mmap!=NULL){
			read_permission=mmap->vm_flags & 0x00000001?"r":"-";
			write_permission=mmap->vm_flags & 0x00000002?"w":"-";
			exe_permission=mmap->vm_flags & 0x00000004?"x":"-";
			printk(KERN_INFO "VMA %lx - %lx\n%s\n%s\n%s\n ",\
			mmap->vm_start,mmap->vm_end,read_permission,write_permission,exe_permission);
			mmap=mmap->vm_next;
		} 

	} else if(!strcmp(token,"findpage")){
		sscanf(cur, "%lx", &addr);
		printk(KERN_INFO "%lx\n",addr);

		pgd=pgd_offset(current->mm,addr);
		if(pgd_none(*pgd) || pgd_bad(*pgd)){
			printk(KERN_ERR "PGD translation not found\n");
			return count;
		}
		p4d = p4d_offset(pgd, addr);
    	if(p4d_none(*p4d) || p4d_bad(*p4d)){
			printk(KERN_ERR "P4D translation not found2\n");
			return count;
		}
		pud = pud_offset(p4d, addr);
		if(pud_none(*pud) || pud_bad(*pud)){
			printk(KERN_ERR "PUD translation not found3\n");
			return count;
		}
		pmd = pmd_offset(pud, addr);
		if(pmd_none(*pmd) || pmd_bad(*pmd)){
			printk(KERN_ERR "PMD translation not found4\n");
			return count;
		}
		pte = pte_offset_map(pmd, addr);
		if(pte_none(*pte) || !pte_present(*pte)){
			printk(KERN_ERR "PTE translation not found5\n");
			return count;
		}

		printk(KERN_INFO "%llx\n",__pte_to_phys(*pte));
		
	} else if(!strcmp(token,"writeeval")){
		token = strsep(&cur, delim);
		sscanf(token, "%lx", &addr);
		printk(KERN_INFO "%lx\n",addr);
		val = cur;
		printk(KERN_INFO "%s\n",cur);

		length = strlen(val)+1;

		if(access_ok((void*)addr,length)){
			copied = copy_to_user((void*)addr,val,length);
		} else{
			printk(KERN_ERR "CAN NOT WRITE AT %lx\n",addr);
		}

		if(copied!=0){
			printk(KERN_ERR "%ld bytes can not be copied to %lx\n",copied,addr);
		} else{
			printk(KERN_INFO "successfully written!\n");
			if(copy_from_user(val,(char*)addr,length)){
				kfree(val);
				printk(KERN_ERR "verification failed!");
				return EFAULT;
			}else{
				printk(KERN_INFO "verified using copy_from_user: %s\n",val);
			}
			
		}
		
		
	}
	
	return count;
}

static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops my_fops = {
  .proc_open = my_proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
  .proc_write = my_proc_write,
};
#else
static struct file_operations my_fops={
	.owner = THIS_MODULE,
	.open = my_proc_open,
	.release = single_release,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = my_proc_write
};
#endif

static int __init mtest_init(void){
	struct proc_dir_entry *entry;
	entry = proc_create("mtest",0777,NULL,&my_fops);
	if(!entry){
		return -1;	
	}else{
		printk(KERN_INFO "create proc file \"mtest\" successfully\n");
	}
	return 0;
}

static void __exit mtest_exit(void){
	remove_proc_entry("mtest",NULL);
	printk(KERN_INFO "Goodbye world!\n");
}

module_init(mtest_init);
module_exit(mtest_exit);
MODULE_LICENSE("GPL");