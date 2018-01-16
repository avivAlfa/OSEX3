#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"


#define SEM_NAME_LENGTH 6

struct sem *isExistSem(char *name);
struct sem *semalloc(char *name, int init, int maxVal);
static int sdalloc(struct sem *s);
int semclose(struct sem *s);

struct sem
{
    struct spinlock sslock; // single semaphore lock
    char name[SEM_NAME_LENGTH];
    int owner_pid;
    int ref; // reference count
    int available_locks;
    int max;
};

struct
{
    struct spinlock gslock; //global semaphore lock
    struct sem sem[NSEM];
} stable;