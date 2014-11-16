#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/pid.h>
#include <linux/mount.h>
#include <linux/genhd.h>

#define __STDOUT 1
#define BUF_SIZE 200

asmlinkage int sys_project(long pid) {

	/*
	 * make sure the sys_project be called
	 */
	printk(KERN_INFO "Hello Linux Project\n");

	/*
	 * find the task_struct
	 * using find_get_pid and pid_task
	 */
	struct pid *p = find_get_pid(pid);
	struct task_struct *task = pid_task(p, PIDTYPE_PID);

	/*
	 * write to stdout
	 * using sys_write
	 * TODO char comm[TASK_COMM_LEN] isn't the full name for process
	 */
	size_t len = strnlen(task->comm, TASK_COMM_LEN);
	sys_write(__STDOUT, task->comm, len);

	/*
	 * print vm_start and vm_end
	 *
	 */
	char buf[BUF_SIZE];
	unsigned long vm_flags;
	struct mm_struct *mm = task->mm;
	struct vm_area_struct *vm = mm->mmap;

	while(vm) {
		len = 0;
		vm_flags = vm->vm_flags;

		/*
		 * linear address from vm_start to vm_end
		 */
		len += snprintf(buf + len, BUF_SIZE - len, "%08lx-%08lx ", vm->vm_start, vm->vm_end);

		/*
		 * permission
		 * r read
		 * w write
		 * x execute
		 * s shared
		 * p private
		 */
		len += snprintf(buf + len, BUF_SIZE - len, "%c", (vm_flags & VM_READ ? 'r' : '-'));
		len += snprintf(buf + len, BUF_SIZE - len, "%c", (vm_flags & VM_WRITE ? 'w' : '-'));
		len += snprintf(buf + len, BUF_SIZE - len, "%c", (vm_flags & VM_EXEC ? 'x' : '-'));
		len += snprintf(buf + len, BUF_SIZE - len, "%c", (vm_flags & VM_SHARED ? 's' : 'p'));

		if(vm->vm_file) {
			/*
			 * offset
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " %08lx", vm->vm_pgoff << 12);

			/*
			 * device
			 * TODO don't know what 'device' is.
			 * $ man proc
			 */
			struct file *f = vm->vm_file;
			struct path *f_path = &f->f_path;
			struct vfsmount *mnt = f_path->mnt;
			struct super_block *mnt_sb = mnt->mnt_sb;
			struct block_device *s_bdev = mnt_sb->s_bdev;
			struct gendisk *bd_disk = s_bdev->bd_disk;

			len += snprintf(buf + len, BUF_SIZE - len, " %02x:%02x"	, bd_disk->major, bd_disk->minors);

			/*
			 * inode
			 * TODO I don't know I choose the right value for it.
			 * From the result of test, I think I found the right value for it.
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " %lu", f->f_inode->i_ino);

			/*
			 * pathname
			 */
			struct dentry *dir = f_path->dentry;
			struct qstr *d_name = &dir->d_name;
			len += snprintf(buf + len, BUF_SIZE - len, " %s", d_name->name);
		}
		else {
			/*
			 * offset, device and inode
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " 00000000 00:00 0");
		}

		printk(KERN_INFO "%s\n", buf);
		vm = vm->vm_next;
	}

	return 1;
}
