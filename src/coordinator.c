#include <stdio.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <unistd.h>
#include <wait.h>

#include "capacity.h"
#include "tasks.h"

sem_t sem_recharge_slots;
sem_t sem_make_rooms;
sem_t sem_paint_rooms;
sem_t sem_transport_rooms;

void print_tasks_state(const struct Task *tasks) {
    for (int i = 0; i < NUM_TASKS; i++) {
        printf("Tâche %d : stage %d\n", tasks[i].task_id, tasks[i].stage);
    }
}

int are_tasks_done(const struct Task *tasks) {
    for (int i = 0; i < NUM_TASKS; i++) {
        if (tasks[i].stage != 3)
            return 0;
    }
    return 1;
}

int main() {
    const key_t key = TASK_SHM_KEY;
    int shmid = shmget(key, sizeof(struct Task) * NUM_TASKS, 0666 | IPC_CREAT);
    struct Task *tasks = shmat(shmid, NULL, 0);

    // Init tasks
    for (int i = 0; i < NUM_TASKS; i++) {
        tasks[i].task_id = i + 1;
        tasks[i].stage = 0;
    }

    print_tasks_state(tasks);

    struct Robot makers[NUM_MAKERS];

    // Init robots
    for (int i = 0; i < NUM_MAKERS; i++) {
        const pid_t forked_pid = fork();
        const int queue_id = msgget(MAKERS_QUEUE_ID + i, IPC_CREAT | 0666);
        if (forked_pid == 0) {
            char robot_id[2];
            sprintf(robot_id, "%d", i);
            execlp("./robot_maker", robot_id, NULL);
            return 0;
        }
        makers[i].id = i + 1;
        makers[i].pid = forked_pid;
        makers[i].queue_id = queue_id;
    }

    while (are_tasks_done(tasks) == 0) {
        for (int i = 0; i < NUM_TASKS; i++) {
            struct RobotMessage robot_message = {1,i};
            makers[0].working = 1;
            msgsnd(makers[0].queue_id, &robot_message, sizeof(robot_message), 0);
            sleep(1);
        }
        printf("bam loop\n");
        print_tasks_state(tasks);
    }

    // Destroying all resources
    for (int i = 0; i < NUM_MAKERS; i++) {
        waitpid(makers[i].pid, NULL, 0);
    }

    shmdt(tasks);
    shmctl(shmid, IPC_RMID, NULL);
}