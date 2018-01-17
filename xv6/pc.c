#include "types.h"
#include "stat.h"
#include "user.h"
#include "pc_def.c"

#define MAX_MSG 3

void closeAllSems();

int main(int argc, char **argv)
{
    int prodNum = atoi(argv[1]);
    int conNum = atoi(argv[2]);
    int msgNum = atoi(argv[3]);
    int childProcNum = prodNum + conNum;

    int sem_access = sem_open("access", 1, 1);
    int sem_occupy = sem_open("occupy", 0, MAX_MSG);
    int sem_free = sem_open("free", MAX_MSG, MAX_MSG);
    int sem_parent = sem_open("parent", 0, childProcNum); //semaphore to block children until parent write his message
    int sem_msgs = sem_open("msgs", prodNum * msgNum, prodNum * msgNum); //semaphore to count msgs that consumed.

    int i, j;
    for (i = 0; i < prodNum; i++)
    {
        if (fork() == 0)
        {
            sem_wait(sem_parent);
            for (j = 0; j < msgNum; j++)
            {
                sem_wait(sem_free);
                sem_wait(sem_access);
                write_prod_msg();
                sem_post(sem_access);
                sem_post(sem_occupy);
            }
            sem_close(sem_access);
            sem_close(sem_occupy);
            sem_close(sem_free);
            sem_close(sem_parent);
            exit();
        }
    }

    for (i = 0; i < conNum; i++)
    {
        if (fork() == 0)
        {
            sem_wait(sem_parent);
            while (sem_try_wait(sem_msgs) == 0)
            {
                sem_wait(sem_occupy);
                sem_wait(sem_access);
                write_cons_msg();
                sem_post(sem_access);
                sem_post(sem_free);
            }
            sem_close(sem_access);
            sem_close(sem_occupy);
            sem_close(sem_free);
            sem_close(sem_parent);
            exit();
        }
    }

    write_parent_msg();
    sem_reset(sem_parent, childProcNum, childProcNum);
    sem_close(sem_access);
    sem_close(sem_occupy);
    sem_close(sem_free);
    sem_close(sem_parent);
    // sem_unlink("access");
    // sem_unlink("occupy");
    // sem_unlink("free");
    // sem_unlink("parent");
    for (i = 0; i < childProcNum; i++)
        wait();
    exit();
}

void closeAllSems()
{
    // sem_close(sem_access);
    // sem_close(sem_occupy);
    // sem_close(sem_free);
    // sem_close(sem_parent);
}