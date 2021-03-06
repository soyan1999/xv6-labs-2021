// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct spinlock ref_lock;
  char page_ref[(PHYSTOP-KERNBASE)/PGSIZE];
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kmem.ref_lock, "ref_lock");
  memset(kmem.page_ref, 1, (PHYSTOP-KERNBASE)/PGSIZE);
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  int ref_count;
  acquire(&kmem.ref_lock);
  if ((ref_count = kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE]) == 0)
    panic("kfree wrong page ref count");
  ref_count = --kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE];
  release(&kmem.ref_lock);
  if (ref_count > 0) return;
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    acquire(&kmem.ref_lock);
    kmem.page_ref[((uint64)r-KERNBASE)/PGSIZE] = 1;
    release(&kmem.ref_lock);
  }
  return (void*)r;
}

void
cow_ref(void *pa) {
  acquire(&kmem.ref_lock);
  if (kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE] == 0)
    panic("cow_ref wrong page ref count");
  kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE] ++;
  // printf("cow_ref %x\n",(uint64)pa);
  release(&kmem.ref_lock);
}

int
cow_dec(void *pa) {
  int ref_count;
  acquire(&kmem.ref_lock);
  if (kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE] == 0)
    panic("cow_dec wrong page ref count");
  if ((ref_count=kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE]) > 1)
    kmem.page_ref[((uint64)pa-KERNBASE)/PGSIZE] --;
  release(&kmem.ref_lock);
  return ref_count;
}