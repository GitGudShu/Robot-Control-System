#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "capacity.h"
#include "tasks.h"

struct Task *tasks;

void signal_handler() {
    printf("Robot bye bye\n");
    shmdt(tasks);
    exit(0);
}

int main(int argc, char *argv[]) {
    printf("hello robot %s !\n", argv[0]);

    const long int robot_id = strtol(argv[0], NULL, 10);

    const key_t key = TASK_SHM_KEY;
    int shmid = shmget(key, sizeof(struct Task) * NUM_TASKS, 0666);
    tasks = shmat(shmid, NULL, 0);

    const int queue_id = msgget(MAKERS_QUEUE_ID + robot_id, 0666);

    for (;;) {
        struct RobotMessage message;
        msgrcv(queue_id, &message, sizeof(message), 1, 0);
        printf("Wokring on Task ID %d\n", message.task_index);
        tasks[message.task_index].stage = tasks[message.task_index].stage + 1;
    }
}
