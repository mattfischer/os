#include "Sched.h"
#include "Slab.h"

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

	space->table = PageAllocContig(4, 4);
	base = (unsigned*)PAGE_TO_VADDR(space->table);

	kernel_nr = KERNEL_START >> PTE_SECTION_BASE_SHIFT;
	for(i=0; i<kernel_nr; i++) {
		base[i] = 0;
	}

	for(i=kernel_nr; i<PAGE_TABLE_SIZE; i++) {
		base[i] = KernelMap[i];
	}
}

void EnterUser(void (*userStart)(), void* userStack);
static void kickstart(void (*start)())
{
	int stackSize = PAGE_SIZE;
	struct Page *stackPages = PageAlloc(stackSize >> PAGE_SHIFT);
	void *stack = (void*)(KERNEL_START - stackSize);
	MapPages(Current->addressSpace, stack, stackPages);

	EnterUser(start, (char*)stack + stackSize);
}

struct Task *TaskCreate(void (*start)())
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = PageAlloc(1);
	task = (struct Task*)(PAGE_TO_VADDR(stack) + PAGE_SIZE - sizeof(struct Task));

	task->stack = stack;
	for(i=0; i<16; i++) {
		task->regs[i] = 0;
	}
	task->regs[0]    = (unsigned int)start;
	task->regs[R_PC] = (unsigned int)kickstart;
	task->regs[R_SP] = (unsigned int)task;

	task->addressSpace = SlabAllocate(&addressSpaceSlab);
	initAddressSpace(task->addressSpace);

	return task;
}

void setMMUBase(void *table);
void switchToAsm(struct Task *current, struct Task *next);
void runFirstAsm(struct Task *next);

static void switchTo(struct Task *current, struct Task *next)
{
	setMMUBase(PAGE_TO_PADDR(next->addressSpace->table));
	switchToAsm(current, next);
}

static void runFirst(struct Task *next)
{
	setMMUBase(PAGE_TO_PADDR(next->addressSpace->table));
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
	SlabInit(&addressSpaceSlab, sizeof(struct AddressSpace));
}