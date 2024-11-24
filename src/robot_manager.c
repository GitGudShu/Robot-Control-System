#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_TASKS 10
#define NUM_MAKERS 2
#define NUM_PAINTERS 3
#define NUM_TRANSPORTERS 1

#define NUM_MAKE_ROOMS 2
#define NUM_PAINT_ROOMS 2
#define NUM_TRANSPORT_ROOMS 1
#define NUM_RECHARGE_ROOMS 2

typedef struct {
    int task_id;
    int stage; // 0: Made, 1: Painted, 2: Transported
} Task;

Task tasks[NUM_TASKS];
int made_count = 0, painted_count = 0, transported_count = 0;

sem_t sem_recharge_slots;
sem_t sem_make_rooms;
sem_t sem_paint_rooms;
sem_t sem_transport_rooms;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int energy;
    int max_energy;
    int id;
} Robot;

// Define robot energy levels
// TODO: We can loop this thing, unless we want to have different energy levels for different robots of the same type
Robot makers[NUM_MAKERS] = {{5, 5, 1}, {5, 5, 2}};
Robot painters[NUM_PAINTERS] = {{6, 6, 1}, {6, 6, 2}, {6, 6, 3}};
Robot transporters[NUM_TRANSPORTERS] = {{8, 8, 1}};

void log_message(const char *msg, int id, const char *type, int task_id) {
    printf("[%s Robot %d] %s Chair %d.\n", type, id, msg, task_id + 1);
    fflush(stdout);
}

void log_status(const char *msg, int id, const char *type) {
    printf("[%s Robot %d] %s\n", type, id, msg);
    fflush(stdout);
}

void recharge(Robot *robot, const char *type) {
    log_status("Energy depleted...", robot->id, type);
    sem_wait(&sem_recharge_slots);
    log_status("Recharging...", robot->id, type);
    sleep(5);
    robot->energy = robot->max_energy; // Fully recharge
    log_status("Functional again.", robot->id, type);
    sem_post(&sem_recharge_slots);
}

void* maker_robot(void* arg) {
    Robot *robot = (Robot *)arg;
    while (1) {
        sem_wait(&sem_make_rooms);
        pthread_mutex_lock(&task_mutex);

        if (made_count >= NUM_TASKS) {
            pthread_mutex_unlock(&task_mutex);
            sem_post(&sem_make_rooms);
            break;
        }

        if (robot->energy < 3) {
            pthread_mutex_unlock(&task_mutex);
            recharge(robot, "Maker");
            sem_post(&sem_make_rooms);
            continue;
        }

        robot->energy -= 3;
        int task_id = made_count++;
        log_message("Making", robot->id, "Maker", task_id);

        pthread_mutex_unlock(&task_mutex);
        sem_post(&sem_make_rooms);

        sleep(2); // Simulate making time
    }
    return NULL;
}

void* painter_robot(void* arg) {
    Robot *robot = (Robot *)arg;
    while (1) {
        sem_wait(&sem_paint_rooms);
        pthread_mutex_lock(&task_mutex);

        if (painted_count >= made_count) {
            if (made_count >= NUM_TASKS) {
                pthread_mutex_unlock(&task_mutex);
                sem_post(&sem_paint_rooms);
                break;
            }
            pthread_mutex_unlock(&task_mutex);
            sem_post(&sem_paint_rooms);
            continue;
        }

        if (robot->energy < 2) {
            pthread_mutex_unlock(&task_mutex);
            recharge(robot, "Painter");
            sem_post(&sem_paint_rooms);
            continue;
        }

        robot->energy -= 2;
        int task_id = painted_count++;
        log_message("Painting", robot->id, "Painter", task_id);

        pthread_mutex_unlock(&task_mutex);
        sem_post(&sem_paint_rooms);

        sleep(4); // Simulate painting time
    }
    return NULL;
}

void* transporter_robot(void* arg) {
    Robot *robot = (Robot *)arg;
    while (1) {
        sem_wait(&sem_transport_rooms);
        pthread_mutex_lock(&task_mutex);

        if (transported_count >= painted_count) {
            if (painted_count >= NUM_TASKS) {
                pthread_mutex_unlock(&task_mutex);
                sem_post(&sem_transport_rooms);
                break;
            }
            pthread_mutex_unlock(&task_mutex);
            sem_post(&sem_transport_rooms);
            continue;
        }

        if (robot->energy < 4) {
            pthread_mutex_unlock(&task_mutex);
            recharge(robot, "Transporter");
            sem_post(&sem_transport_rooms);
            continue;
        }

        robot->energy -= 4;
        int task_id = transported_count++;
        log_message("Transporting", robot->id, "Transporter", task_id);
        printf("Chair %d finished.\n", task_id + 1);

        pthread_mutex_unlock(&task_mutex);
        sem_post(&sem_transport_rooms);

        sleep(3); // Simulate transporting time
    }
    return NULL;
}

int main() {
    pthread_t maker_threads[NUM_MAKERS], painter_threads[NUM_PAINTERS], transporter_threads[NUM_TRANSPORTERS];

    sem_init(&sem_recharge_slots, 0, NUM_RECHARGE_ROOMS);
    sem_init(&sem_make_rooms, 0, NUM_MAKE_ROOMS);
    sem_init(&sem_paint_rooms, 0, NUM_PAINT_ROOMS);
    sem_init(&sem_transport_rooms, 0, NUM_TRANSPORT_ROOMS);

    for (int i = 0; i < NUM_MAKERS; i++) {
        pthread_create(&maker_threads[i], NULL, maker_robot, &makers[i]);
    }
    for (int i = 0; i < NUM_PAINTERS; i++) {
        pthread_create(&painter_threads[i], NULL, painter_robot, &painters[i]);
    }
    for (int i = 0; i < NUM_TRANSPORTERS; i++) {
        pthread_create(&transporter_threads[i], NULL, transporter_robot, &transporters[i]);
    }

    for (int i = 0; i < NUM_MAKERS; i++) {
        pthread_join(maker_threads[i], NULL);
    }
    for (int i = 0; i < NUM_PAINTERS; i++) {
        pthread_join(painter_threads[i], NULL);
    }
    for (int i = 0; i < NUM_TRANSPORTERS; i++) {
        pthread_join(transporter_threads[i], NULL);
    }

    sem_destroy(&sem_recharge_slots);
    sem_destroy(&sem_make_rooms);
    sem_destroy(&sem_paint_rooms);
    sem_destroy(&sem_transport_rooms);

    printf("All tasks are finished. Program terminated.\n");
    return 0;
}
