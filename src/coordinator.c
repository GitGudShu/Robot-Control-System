#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#include "capacity.h"
#include "tasks.h"

int task_shmid;
struct Task *tasks;

int semaphores_shmid;
struct Semaphores *semaphores;

struct Robot makers[NUM_MAKERS];
struct Robot painters[NUM_PAINTERS];
struct Robot transporters[NUM_TRANSPORTERS];

void print_tasks_state(const struct Task *tasks) {
    for (int i = 0; i < NUM_TASKS; i++) {
        printf("T%d: S%dW%d / ", tasks[i].task_id, tasks[i].stage, tasks[i].working);
    }
    printf("\n");
}

int are_tasks_done(const struct Task *tasks) {
    for (int i = 0; i < NUM_TASKS; i++) {
        if (tasks[i].stage != 3)
            return 0;
    }
    return 1;
}

void receive_messages(struct Robot robots[], int size) {
    for (int i = 0; i < size; i++) {
        struct RobotMessage rcv_message;
        errno = 0;
        msgrcv(robots[i].queue_id, &rcv_message, sizeof(rcv_message), 2, IPC_NOWAIT);
        if (errno != ENOMSG) {
            robots[i].available = rcv_message.recharging == 1 ? 0 : 1;
        }
    }
}

int send_message_to_available_robot(struct Robot robots[], const int size, const int task_index) {
    for (int i = 0; i < size; i++) {
        if (robots[i].available == 0) continue;
        const struct RobotMessage robot_message = {1,task_index, 0};
        msgsnd(robots[i].queue_id, &robot_message, sizeof(robot_message), 0);
        robots[i].available = 0;
        return i;
    }
    return -1;
}

void create_robots(struct Robot robots[], const int size, const int queue_id_offset, const char* prg) {
    for (int i = 0; i < size; i++) {
        const int queue_id = msgget(queue_id_offset + i, IPC_CREAT | 0666);

        const pid_t forked_pid = fork();
        if (forked_pid == 0) {
            char robot_id[2];
            sprintf(robot_id, "%d", i);
            execlp(prg, robot_id, NULL);
            return;
        }

        robots[i].id = i + 1;
        robots[i].pid = forked_pid;
        robots[i].queue_id = queue_id;
        robots[i].available = 1;
    }
}

void destroy_robots(struct Robot robots[], const int size) {
    for (int i = 0; i < size; i++) {
        kill(robots[i].pid, SIGTERM);
        waitpid(robots[i].pid, NULL, 0);
        msgctl(robots[i].queue_id, IPC_RMID, NULL);
    }
}

void print_robot_statuses(struct Robot robots[], const int size) {
    for (int i = 0; i < size; i++) {
        printf("R%d: A%d ", i, robots[i].available);
    }
    printf("\n");
}

void destroy_all_resources() {
    // Destroying all resources
    destroy_robots(makers, NUM_MAKERS);
    destroy_robots(painters, NUM_PAINTERS);
    destroy_robots(transporters, NUM_TRANSPORTERS);

    shmdt(tasks);
    shmctl(task_shmid, IPC_RMID, NULL);
    shmdt(semaphores);
    shmctl(semaphores_shmid, IPC_RMID, NULL);

    exit(0);
}

int main() {
    signal(SIGTERM, destroy_all_resources);
    signal(SIGINT, destroy_all_resources);

    task_shmid = shmget(TASK_SHM_KEY, sizeof(struct Task) * NUM_TASKS, 0666 | IPC_CREAT);
    tasks = shmat(task_shmid, NULL, 0);

    semaphores_shmid = shmget(SEM_SHM_KEY, sizeof(struct Semaphores), 0666 | IPC_CREAT);
    semaphores = shmat(semaphores_shmid, NULL, 0);

    sem_init(&semaphores->sem_recharge_slots, 1, NUM_RECHARGE_ROOMS);
    sem_init(&semaphores->sem_make_rooms, 1, NUM_MAKE_ROOMS);
    sem_init(&semaphores->sem_paint_rooms, 1, NUM_PAINT_ROOMS);
    sem_init(&semaphores->sem_transport_rooms, 1, NUM_TRANSPORT_ROOMS);

    // Init tasks
    for (int i = 0; i < NUM_TASKS; i++) {
        tasks[i].task_id = i + 1;
        tasks[i].stage = 0;
    }

    // Init robots
    create_robots(makers, NUM_MAKERS, MAKERS_QUEUE_OFFSET, "./robot_maker");
    create_robots(painters, NUM_PAINTERS, PAINTERS_QUEUE_OFFSET, "./robot_painter");
    create_robots(transporters, NUM_TRANSPORTERS, TRANSPORTERS_QUEUE_OFFSET, "./robot_transporter");

    while (are_tasks_done(tasks) == 0) {
        receive_messages(makers, NUM_MAKERS);
        receive_messages(painters, NUM_PAINTERS);
        receive_messages(transporters, NUM_TRANSPORTERS);

        print_tasks_state(tasks);
        printf("Makers\t\t\t| ");
        print_robot_statuses(makers, NUM_MAKERS);
        printf("Painters\t\t| ");
        print_robot_statuses(painters, NUM_PAINTERS);
        printf("Transporters\t| ");
        print_robot_statuses(transporters, NUM_TRANSPORTERS);

        for (int i = 0; i < NUM_TASKS; i++) {
            // Skip working tasks
            if (tasks[i].working == 1) continue;
            // Skip done tasks
            if (tasks[i].stage == 3) continue;
            // Task needs to be worked!
            if (tasks[i].stage == 0) {
                send_message_to_available_robot(makers, NUM_MAKERS, i);
            } else if (tasks[i].stage == 1) {
                send_message_to_available_robot(painters, NUM_PAINTERS, i);
            } else if (tasks[i].stage == 2) {
                send_message_to_available_robot(transporters, NUM_TRANSPORTERS, i);
            }
        }

        sleep(1);
    }

    print_tasks_state(tasks);
    printf("All tasks done!\n");

    destroy_all_resources();
}
