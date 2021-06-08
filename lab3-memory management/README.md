# Lab3 mtest
### ----A relatively hard lab

Referenced:  [This repo](https://github.com/DeanAlkene/CS353-Linux-Kernel)

## Before Start
Linux kernel has been updated very fast. Code for memory management is different between linux5.5.11 and 5.12.4


## ListVMA
Begin from line 53

Memory regions for a given process is organized as many continuous VM areas. 

VMA info in stored in `current->mm->mmap`.

## FindPage
Begin from line 65

We need to use a series of macros for page table walk.

These macros are defined in `linux/pgtable.h` in linux 5.12.4.

But they are defined in `asm/pgtable_types.h` in linux 5.5.11.

Also, notice that the macro for pte is `pte_offset_map` instead of `pte_offset`.

Use macro `__pte_to_phys` to convert pte to its physical addr.

## Writeeval
Begin from line 97

This function can be achieved with the famous function `copy_to_user`.

Although in kernel space we have the information of user space address mapping, we cannot directly access them for isolation reasons. We need to use `copy_to_user` and `copy_from_user` instead.

Before using `copy_to_user`, we need to do a check using `access_ok`, as recommended in documentation.

Then use `copy_from_user` we can verify whether the written has succeeded.