#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/pid.h>
#include <asm/pgtable.h>

#define BUF_SIZE 200
#define STACK_SIZE BUF_SIZE

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
	 * filename
	 */
	printk(KERN_INFO "%s\n", task->mm->mmap->vm_file->f_path.dentry->d_name.name);

	/*
	 * print vm_start and vm_end
	 *
	 */
	size_t len = 0;
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
		len += snprintf(buf + len, BUF_SIZE - len, "%c", (vm_flags & VM_MAYSHARE ? 's' : 'p'));

		if(vm->vm_file) {
			/*
			 * offset
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " %08lx", vm->vm_pgoff << 12);

			/*
			 * device
			 * TODO don't know what 'device' is.
			 * still some problem here
			 */
			struct file *f = vm->vm_file;
			struct path *f_path = &f->f_path;
			struct inode *inode = f_path->dentry->d_inode;
			dev_t dev = inode->i_sb->s_dev;

			len += snprintf(buf + len, BUF_SIZE - len, " %02x:%02x"	, MAJOR(dev), MINOR(dev) & 0xff);

			/*
			 * inode
			 * TODO I don't know I choose the right value for it.
			 * From the result of test, I think I found the right value for it.
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " %lu", inode->i_ino);

			/*
			 * pathname
			 */
			size_t stack_len = 0;
			const unsigned char *dir_stack[STACK_SIZE];
			struct dentry *dir = f_path->dentry;
			struct qstr *d_name = &dir->d_name;
			while(dir != dir->d_parent){
				d_name = &dir->d_name;
				dir_stack[stack_len++] = d_name->name;
				dir = dir->d_parent;
			}
			len += snprintf(buf + len, BUF_SIZE - len, "\t");
			for(; stack_len > 0; stack_len--) {
				len += snprintf(buf + len, BUF_SIZE - len, "/%s", dir_stack[stack_len - 1]);
			}
		}
		else {
			/*
			 * offset, device and inode
			 */
			len += snprintf(buf + len, BUF_SIZE - len, " 00000000 00:00 0");

			/*
			 * heap, stack or vdso
			 */
			if(arch_vma_name(vm) || !vm->vm_mm) {
				/*
				 * vdso
				 */
				len += snprintf(buf + len, BUF_SIZE - len, "\t[vdso]");
			}
			else if(vm->vm_start <= mm->brk && vm->vm_end >= mm->start_brk) {
				/*
				 * heap
				 */
				len += snprintf(buf + len, BUF_SIZE - len, "\t[heap]");
			}
			else if(vm->vm_start <= mm->start_stack && vm->vm_end >= mm->start_stack) {
				/*
				 * stack
				 */
				len += snprintf(buf + len, BUF_SIZE - len, "\t[stack]");
			}
		}

		printk(KERN_INFO "%s\n", buf);
		vm = vm->vm_next;
	}

	/*
	 * page
	 * TODO there is some better way to find phyiscal address by reducing 'pgd' calculus
	 */
	vm = mm->mmap;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long phy_address = 0;
	unsigned long vm_start, vm_end;

	for(; vm; vm = vm->vm_next) {

		printk(KERN_INFO "%08lx-%08lx\n", vm->vm_start, vm->vm_end);

		for(vm_start = vm->vm_start, vm_end = vm->vm_end; vm_start < vm_end; vm_start += PAGE_SIZE) {
			pgd = pgd_offset(mm, vm_start);
			pud = pud_offset(pgd, vm_start);
			pmd = pmd_offset(pud, vm_start);
			pte = pte_offset_kernel(pmd, vm_start);
			phy_address = pte_val(*pte);
			if(!phy_address ) continue ;
			printk(KERN_INFO "  --> %08lx-%08lx\n", phy_address, phy_address + PAGE_SIZE);
		}
	}

/*Another method */
/*
	vma = mm->mmap ;
	unsigned long vm_addr,phy_addr ;
	struct page *page ;

	while(vma){

		printk(KERN_INFO "%08lx-%08lx\n", vma->vm_start, vma->vm_end);
		for(vm_addr = vma->vm_start ; vm_addr < vma->vm_end ; vm_addr += PAGE_SIZE){
			page = follow_page(vma,vm_addr,0) ;
			if(page == NULL ) continue ;
			phy_addr = page_to_phys(page) ;
			printk(KERN_INFO " --> %08lx-%08lx\n", phy_addr, phy_addr + PAGE_SIZE);

		}
		vma = vma->vm_next ;
*/
											                                                                              }i
	return 0;
}

asmlinkage int nonwritable(unsigned long begin, unsigned long end) {

	printk(KERN_INFO "Hello Nonwritable\n");

	struct mm_struct *mm = current->mm;

#define RO_MODE (~(unsigned long)2)

	/*
	 * set the page entry to read-only mode
	 * make sure begin is the page entry
	 * we use mask to do
	 */
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long vm_start, vm_end;
	for(vm_start = begin & PAGE_MASK, vm_end = end; vm_start < vm_end; vm_start += PAGE_SIZE) {
		pgd = pgd_offset(mm, vm_start);
		pud = pud_offset(pgd, vm_start);
		pmd = pmd_offset(pud, vm_start);
		pte = pte_offset_kernel(pmd, vm_start);

		pte->pte &= RO_MODE;
	}

	return 0;
}
