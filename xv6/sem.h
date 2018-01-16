#define SEM_NAME_LENGTH 6

struct sem
{
    struct spinlock sslock; // single semaphore lock
    char name[SEM_NAME_LENGTH];
    int owner_pid;
    int ref; // reference count
    int available_locks;
    int max;
};