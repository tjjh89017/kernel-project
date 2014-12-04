#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/path.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/dcache.h>
#include <asm/page.h>
#include <linux/page-flags.h>
#include <asm/pgtable.h>	
#include <linux/highmem.h>

asmlinkage int sys_nonwritable(unsigned long begin,unsigned long end) {

	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma = mm->mmap ;
	struct page *page ;
	unsigned long vm_addr ;

	pgd_t *pgd ;
	pud_t *pud ;
	pmd_t *pmd ;
	pte_t *pte ;
	for(vm_addr = begin & PAGE_MASK ; vm_addr < end ; vm_addr += PAGE_SIZE){
		pgd = pgd_offset(mm,vm_addr) ;
		pud = pud_offset(pgd,vm_addr) ;
		pmd = pmd_offset(pud,vm_addr) ;
		pte = pte_offset_map(pmd,vm_addr) ;
	
	 	//ptep_set_wrprotect(mm,vm_addr,pte) ;
		set_pte(pte, pte_wrprotect(*pte));	
		//printk(KERN_INFO "%08lx",pte->pte) 	;
	}
	return 0 ;
}
