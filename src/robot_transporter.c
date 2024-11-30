#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "capacity.h"
#include "tasks.h"

// Constants
// Energy consumed by task
#define ENERGY_USED_PER_TASK 4
// Capacity of robot
#define ENERGY_CAPACITY 8
// The energy capacity at which the robot is starting
#define STARTING_ENERGY 8
// The robot time to execute the task
#define TASK_TIME 3
// The semaphore we are waiting on to start a task
#define USED_SEMAPHORE semaphores->sem_transport_rooms
// Queue id to use
#define QUEUE_OFFSET TRANSPORTERS_QUEUE_OFFSET
// 30% chance of breakdown
#define BREAKDOWN_PROBABILITY 30

struct Task *tasks;
struct Semaphores *semaphores;
int energy = STARTING_ENERGY;

long int robot_id;

void bye() {
    shmdt(tasks);
    shmdt(semaphores);
    exit(0);
}

void log_message(const char* msg, int task_id) {
    if (task_id != -1) {
        printf("[Transporter %ld (T%d)] %s\n", robot_id, task_id, msg);
    } else {
        printf("[Transporter %ld] %s\n", robot_id, msg);
    }
}

void recharge() {
    log_message("Recharging...", -1);
    sem_wait(&semaphores->sem_recharge_slots);
    sleep(5);
    energy = ENERGY_CAPACITY;
    sem_post(&semaphores->sem_recharge_slots);
    log_message("Recharged!", -1);
}

void repair() {
    log_message("Breaking down, going to repair room...", -1);
    sem_wait(&semaphores->sem_repair_slots);
    sleep(5);
    sem_post(&semaphores->sem_repair_slots);
    log_message("Repaired and operational!", -1);
}

int coin_toss(int probability) {
    return rand() % 100 < probability;
}

void send_message(int queue_id, long type, int task_index, int ready_to_work) {
    struct RobotMessage message;
    message.message_type = type;
    message.task_index = task_index;
    message.ready_to_work = ready_to_work;
    msgsnd(queue_id, &message, sizeof(message), 0);
}


int main(int argc, char *argv[]) {
    signal(SIGTERM, bye);
    signal(SIGINT, bye);
    robot_id = strtol(argv[0], NULL, 10);
    log_message("Hello!", -1);

    const int task_shmid = shmget(TASK_SHM_KEY, sizeof(struct Task) * NUM_TASKS, 0666);
    tasks = shmat(task_shmid, NULL, 0);

    const int semaphores_shmid = shmget(SEM_SHM_KEY, sizeof(struct Semaphores), 0666);
    semaphores = shmat(semaphores_shmid, NULL, 0);

    const int queue_id = msgget(QUEUE_OFFSET + robot_id, 0666);

    for (;;) {
        struct RobotMessage message;
        msgrcv(queue_id, &message, sizeof(message), 1, 0);

        tasks[message.task_index].working = 1;
        log_message("Waiting for dock...", tasks[message.task_index].task_id);
        sem_wait(&USED_SEMAPHORE);

        log_message("Working...", tasks[message.task_index].task_id);

        sleep(2); // Making time
        tasks[message.task_index].stage = tasks[message.task_index].stage + 1;
        tasks[message.task_index].working = 0;

        log_message("Transported!", tasks[message.task_index].task_id);

        energy -= ENERGY_USED_PER_TASK;
        sem_post(&USED_SEMAPHORE);

        if (energy < ENERGY_USED_PER_TASK) {
            send_message(queue_id, 2, message.task_index, 0);
            recharge();
        }

        if (coin_toss(BREAKDOWN_PROBABILITY)) {
            send_message(queue_id, 2, message.task_index, 0);
            repair();
        }

        send_message(queue_id, 2, message.task_index, 1);
    }
}
