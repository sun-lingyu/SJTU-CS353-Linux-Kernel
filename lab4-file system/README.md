# Lab4 File System
### ----modify romfs

## Hide file
Following the hit in instruction pdf, add a strcmp in super.c line 215

## Encrypt file
Following the hit in instruction pdf, we need to first get the file name and then compare it to encrypted_file_name.

This can be done using the following code in super.c line 114:
```C
if(!strcmp(file->f_path.dentry->d_iname,encrypted_file_name)){
		encrypted = 1;
	}
```
This can get the corresponding file name from `struct file` (file descriptor). Notice that the file name is stored in dentry.

Then in super.c line 141, we can encrype the file content after read it from disk.

## Change permission to executable
Following the hit in instruction pdf, add a strcmp in super.c line 268.

Notice that the permission info ins stored in `i_mode` entry in `struct inode`. To make it executable, we need to set `i_mode` by `S_IRWXU`.

## Summary
Idea of this lab is simple, but the APIs are hard to find...