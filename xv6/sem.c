#include "sem.h"



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
    return -1;
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

// Allocate a sem descriptor for the given semapore.
static int sdalloc(struct sem *s)
{
    int sd;
    struct proc *curproc = myproc();
    for (sd = 0; sd < NOSEM; sd++)
    {
        if (curproc->osem[sd] == 0)
        {
            curproc->osem[sd] = s;
            return sd;
        }
    }
    return -1;
}

int semclose(struct sem *s)
{
    if (s->ref <= 0)
        return -1;

    acquire(&stable.gslock);
    s->ref--;
    if (s->ref == 0)
    {
        wakeup(s);
    }
    release(&stable.gslock);
    return 0;
}
