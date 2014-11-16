#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/pid.h>

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
		len += snprintf(buf + len, BUF_SIZE - len, "%lu-%lu ", vm->vm_start, vm->vm_end);

		/*
		 * permission
		 */
		len += snprintf(buf + len, BUF_SIZE, "%c", (vm_flags & VM_READ ? 'r' : '-'));
		len += snprintf(buf + len, BUF_SIZE, "%c", (vm_flags & VM_WRITE ? 'w' : '-'));
		len += snprintf(buf + len, BUF_SIZE, "%c", (vm_flags & VM_EXEC ? 'x' : '-'));
		len += snprintf(buf + len, BUF_SIZE, "%c", (vm_flags & VM_SHARED ? 'p' : '-'));

		printk(KERN_INFO "%s\n", buf);
		vm = vm->next;
	}

	return 1;
}
