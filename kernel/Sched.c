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

	task->state = TaskStateReady;
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
	memset(base, 0, kernel_nr * sizeof(unsigned));

	kernelTable = (unsigned*)PAGE_TO_VADDR(KernelSpace.table);
	memcpy(base + kernel_nr, kernelTable + kernel_nr, PAGE_TABLE_SIZE - kernel_nr);
}

struct Task *TaskCreate(void (*start)())
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = PageAlloc(1);
	task = (struct Task*)(PAGE_TO_VADDR(stack) + PAGE_SIZE - sizeof(struct Task));

	task->stack = stack;
	task->state = TaskStateInit;
	memset(task->regs, 0, 16 * sizeof(unsigned int));
	task->regs[R_PC] = (unsigned int)start;
	task->regs[R_SP] = (unsigned int)task;

	memset(task->channels, 0, sizeof(task->channels));
	memset(task->connections, 0, sizeof(task->connections));

	task->addressSpace = SlabAllocate(&addressSpaceSlab);
	initAddressSpace(task->addressSpace);

	return task;
}

static void switchTo(struct Task *current, struct Task *next)
{
	next->state = TaskStateRunning;

	SetMMUBase(next->addressSpace->tablePAddr);
	SwitchToAsm(current, next);
}

static void runFirst(struct Task *next)
{
	next->state = TaskStateRunning;

	SetMMUBase(next->addressSpace->tablePAddr);
	RunFirstAsm(next);
}

void Schedule()
{
	struct Task *next;
	struct Task *old;
	
	if(Current->state == TaskStateRunning) {
		TaskAdd(Current);
	}

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