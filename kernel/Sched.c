#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

struct RunList {
	struct Task *head;
	struct Task *tail;
};

struct RunList runList = {NULL, NULL};
struct Task *Current = NULL;

struct SlabAllocator addressSpaceSlab;

void TaskAdd(struct Task *task)
{
	if(runList.tail == NULL) {
		runList.head = task;
	} else {
		runList.tail->next = task;
	}

	runList.tail = task;
}

static struct Task *removeHead()
{
	struct Task *task;

	task = runList.head;
	runList.head = runList.head->next;
	if(runList.head == NULL) {
		runList.tail = NULL;
	}

	return task;
}

static void initAddressSpace(struct AddressSpace *space)
{
	int i;
	unsigned *base;
	unsigned *kernelTable;
	int kernel_nr;

	space->table = PageAllocContig(4, 4);
	space->tablePAddr = PAGE_TO_PADDR(space->table);
	base = (unsigned*)PAGE_TO_VADDR(space->table);

	kernel_nr = KERNEL_START >> PTE_SECTION_BASE_SHIFT;
	for(i=0; i<kernel_nr; i++) {
		base[i] = 0;
	}

	kernelTable = (unsigned*)PAGE_TO_VADDR(KernelSpace.table);
	for(i=kernel_nr; i<PAGE_TABLE_SIZE; i++) {
		base[i] = kernelTable[i];
	}
}

struct Task *TaskCreate(void (*start)())
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = PageAlloc(1);
	task = (struct Task*)(PAGE_TO_VADDR(stack) + PAGE_SIZE - sizeof(struct Task));

	task->stack = stack;
	memset(task->regs, 0, 16 * 4);
	task->regs[R_PC] = (unsigned int)start;
	task->regs[R_SP] = (unsigned int)task;

	task->addressSpace = SlabAllocate(&addressSpaceSlab);
	initAddressSpace(task->addressSpace);

	return task;
}

static void switchTo(struct Task *current, struct Task *next)
{
	SetMMUBase(next->addressSpace->tablePAddr);
	SwitchToAsm(current, next);
}

static void runFirst(struct Task *next)
{
	SetMMUBase(next->addressSpace->tablePAddr);
	RunFirstAsm(next);
}

void Schedule()
{
	struct Task *next;
	struct Task *old;
	
	TaskAdd(Current);

	next = removeHead();

	if(next != Current) {
		old = Current;
		Current = next;
		switchTo(old, Current);
	}
}

void ScheduleFirst()
{
	Current = removeHead();
	runFirst(Current);
}

void SchedInit()
{
	SlabInit(&addressSpaceSlab, sizeof(struct AddressSpace));
}