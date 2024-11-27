#ifndef TASKS_H
#define TASKS_H

#include <sys/types.h>

#define TASK_SHM_KEY 240
#define MAKERS_QUEUE_ID 10

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
    int working;
};

struct RobotMessage {
    long message_type;
    int task_index;
};

#endif //TASKS_H
