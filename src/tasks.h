#ifndef TASKS_H
#define TASKS_H

#include <sys/types.h>

#define TASK_SHM_KEY 240
#define SEM_SHM_KEY 250

#define MAKERS_QUEUE_OFFSET 10
#define PAINTERS_QUEUE_OFFSET 20
#define TRANSPORTERS_QUEUE_OFFSET 30

struct Semaphores {
    sem_t sem_recharge_slots;
    sem_t sem_make_rooms;
    sem_t sem_paint_rooms;
    sem_t sem_transport_rooms;
    sem_t sem_repair_slots;
};

struct Task {
    int task_id;
    // 0: Pending
    // 1: Made
    // 2: Painted
    // 3: Transported
    int stage;
    int working;
};

struct Robot {
    int id;
    pid_t pid;
    int queue_id;
    int available;
};

struct RobotMessage {
    long message_type;
    int task_index;
    int ready_to_work;
};

#endif //TASKS_H
