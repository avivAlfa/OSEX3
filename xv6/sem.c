#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "sem.h"

struct
{
    struct spinlock gslock; //global semaphore lock
    struct sem sem[NSEM];
} stable;

struct sem *isExistSem(char *name)
{
    struct sem *s;

    acquire(&stable.gslock);
    for (s = stable.sem; s < stable.sem + NSEM; s++)
    {
        if (strncmp(s->name, name, SEM_NAME_LENGTH))
        {
            s->ref++;
            release(&stable.gslock);
            return s;
        }
    }
    release(&stable.gslock);
    s = 0;   
    return s;
}

struct sem *semalloc(char *name, int init, int maxVal)
{
    struct sem *s;

    acquire(&stable.gslock);
    for (s = stable.sem; s < stable.sem + NSEM; s++)
    {
        if (s->owner_pid == 0)
        {
            s->ref = 1;
            s->owner_pid = myproc()->pid;

            release(&stable.gslock);
            return s;
        }
    }

    release(&stable.gslock);
    return s;
}


int semclose(struct sem *s)
{
    if (s->ref <= 0)
        return -1;

    acquire(&stable.gslock);
    s->ref--;
    if (s->ref == 0)
    {
        wakeup(&s->ref);
    }
    release(&stable.gslock);
    return 0;
}

// Increment ref count for file f.
struct sem* semdup(struct sem *s)
{
  acquire(&stable.gslock);
  if(s->ref < 1)
    panic("semdup");
  s->ref++;
  release(&stable.gslock);
  return s;
}
