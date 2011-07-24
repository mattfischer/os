#include "Sched.h"

struct RunList {
	struct Task *head;
	struct Task *tail;
};

struct RunList runList = {NULL, NULL};
struct Task *Current = NULL;

struct SlabAllocator taskSlab;
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

struct Task *removeHead()
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
	int kernel_nr;

	space->pageTable = PageAllocContig(4, 4);
	base = (unsigned*)PAGE_TO_VADDR(space->pageTable);

	kernel_nr = KERNEL_START >> PTE_SECTION_BASE_SHIFT;
	for(i=0; i<kernel_nr; i++) {
		base[i] = 0;
	}

	for(i=kernel_nr; i<PAGE_TABLE_SIZE; i++) {
		base[i] = KernelMap[i];
	}
}

void EnterUser(void (*userStart)(), void* userStack);
static void kickstart()
{
	void* userStack = PAGE_TO_VADDR(Current->userStack) + PAGE_SIZE;
	EnterUser(Current->userStart, userStack);
}

struct Task *TaskCreate(void (*start)())
{
	int i;
	struct Task *task;

	task = SlabAllocate(&taskSlab);

	task->stack = PageAlloc(1);
	task->userStack = PageAlloc(1);
	task->userStart = start;
	for(i=0; i<16; i++) {
		task->regs[i] = 0;
	}
	task->regs[R_IP] = (unsigned int)kickstart;
	task->regs[R_SP] = (unsigned int)PAGE_TO_VADDR(task->stack) + PAGE_SIZE;

	task->addressSpace = SlabAllocate(&addressSpaceSlab);
	initAddressSpace(task->addressSpace);

	return task;
}

void setMMUBase(void *pageTable);
void switchToAsm(struct Task *current, struct Task *next);
void runFirstAsm(struct Task *next);

static void switchTo(struct Task *current, struct Task *next)
{
	setMMUBase(PAGE_TO_PADDR(next->addressSpace->pageTable));
	switchToAsm(current, next);
}

static void runFirst(struct Task *next)
{
	setMMUBase(PAGE_TO_PADDR(next->addressSpace->pageTable));
	runFirstAsm(next);
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
	SlabInit(&taskSlab, sizeof(struct Task));
	SlabInit(&addressSpaceSlab, sizeof(struct AddressSpace));
}