#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"
#include "sem.h"

static int argsd(int *psd, struct sem **ps)
{
    int sd;
    struct sem *s;

    if (argint(0, &sd) < 0)
        return -1;
    if (sd < 0 || sd >= NOSEM || (s = myproc()->osem[sd]) == 0)
        return -1;
    if (psd)
        *psd = sd;
    if (ps)
        *ps = s;
    return 0;
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

int sys_sem_open(void)
{
    int sd;
    struct sem *s;
    char *name;
    int init, maxVal;

    if (argstr(0, &name) < 0 || argint(1, &init) < 0 || argint(2, &maxVal) < 0)
    {
        //  fprintf(stderr, "process %d, failed in sem_open, cant load args from stack", getpid());
        return -1;
    }

    if (strlen(name) > SEM_NAME_LENGTH || init > maxVal || init < 0)
    {
        //  fprintf(stderr, "process %d, failed in sem_open, Invalid arguments", getpid());
        return -1;
    }

    if ((s = isExistSem(name)) != 0)
    {
        if ((sd = sdalloc(s)) < 0)
        {
            if (s)
                semclose(s);
            return -1;
        }
    }
    else
    {
        if ((s = semalloc(name, init, maxVal)) == 0 || (sd = sdalloc(s)) < 0)
        {
            if (s)
                semclose(s);
            return -1;
        }
    }

    safestrcpy(s->name, name, SEM_NAME_LENGTH);
    s->max = maxVal;
    s->available_locks = init;
    return 0;
}

int sys_sem_close(void)
{
    int sd;
    struct sem *s;

    // if (argstr(0, &sd) < 0)
    // {
    //     //  fprintf(stderr, "process %d, failed in sem_close, cant load args from stack", getpid());
    //     return -1;
    // }
    if (argsd(&sd, &s) < 0)
        return -1;
    return semclose(s);
}

int sys_sem_wait(void)
{
    int sd;
    struct sem *s;

    if (argsd(&sd, &s) < 0)
        return -1;

    acquire(&s->sslock);
    if (s->available_locks > 0)
    {
        s->available_locks--;
        release(&s->sslock);
    }
    else
    {
        sleep(s, &s->sslock);
    }
    release(&s->sslock);
    return 0;
}

int sys_sem_try_wait(void)
{
    int sd;
    struct sem *s;
    int returnVal = 0;

    if (argsd(&sd, &s) < 0)
        return -1;

    acquire(&s->sslock);
    if (s->available_locks > 0)
    {
        s->available_locks--;
    }
    else
    {
        returnVal = -1;
    }
    release(&s->sslock);
    return returnVal;
}

int sys_sem_post(void)
{
    int sd;
    struct sem *s;

    if (argsd(&sd, &s) < 0)
        return -1;

    if (s->available_locks == s->max)
        return -2;

    acquire(&s->sslock);
    if (++(s->available_locks) == 1)
        wakeup(s);
    release(&s->sslock);
    return 0;
}

int sys_sem_reset(void)
{
    int sd, newVal, newMaxVal;
    struct sem *s;
    int returnVal = -1;

    if (argsd(&sd, &s) < 0)
        return -1;

    if(s->owner_pid != myproc()->pid)
        return -1;

    if (argint(1, &newVal) < 0 || argint(2, &newMaxVal) < 0)
    {
        //  fprintf(stderr, "process %d, failed in sem_open, cant load args from stack", getpid());
        return -1;
    }

    acquire(&s->sslock);
    if (newVal >= 0)
    {
        int oldVal = s->available_locks;
        s->available_locks = newVal;
        if (oldVal == 0 && newVal > 0)
            wakeup(s);
        returnVal = 0;
    }

    if (newMaxVal >= s->available_locks)
    {
        s->max = newMaxVal;
        returnVal = 0;
    }
    else
        returnVal = -1;

    release(&s->sslock);
    return returnVal;
}

int 
sys_sem_unlink(void)
{
	char *name;
	struct sem *s;

	if (argstr(0, &name) < 0)
		return -1;

	if ((s = isExistSem(name)) == 0)
		return -1;
	
	acquire(&s->sslock);
	while (1)
	{
		if (s->ref == 0) {
			memset(s->name, 0, SEM_NAME_LENGTH);
			s->owner_pid = 0;
			s->available_locks = 0;
			s->max = 0;
			release(&s->sslock);
			return 0;
		}
		sleep(&s->ref, &s->sslock);
	}
}