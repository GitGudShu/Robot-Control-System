#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_TASKS 10 // Number of tasks to be performed
#define NUM_ROBOTS 3 // Number of robots
#define SHARED_RESOURCES 2 // Number of shared resources

sem_t sem_task_available; // Tracks available tasks
sem_t sem_shared_resource; // Limits access to shared resources

// Robot function (each thread runs this)
void* robot(void* arg) {
    int id = *(int*)arg;

    while (1) {
        // Is a task available?
        if (sem_wait(&sem_task_available) != 0) {
            perror("Error waiting for task semaphore");
            pthread_exit(NULL);
        }

        sem_wait(&sem_shared_resource);
        printf("Robot %d: Acquired a shared resource to perform a task.\n", id);

        sleep(1);
        printf("Robot %d: Completed the task and released the resource.\n", id);

        // Release the shared resource
        sem_post(&sem_shared_resource);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_ROBOTS];
    int ids[NUM_ROBOTS];

    // Create our task semaphore
    sem_init(&sem_task_available, 0, NUM_TASKS);  // Start with NUM_TASKS
    sem_init(&sem_shared_resource, 0, SHARED_RESOURCES); // Shared resources

    // Create the robots
    for (int i = 0; i < NUM_ROBOTS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, robot, &ids[i]);
    }

    // Waiting for the robots to finish
    for (int i = 0; i < NUM_ROBOTS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the task semaphore
    sem_destroy(&sem_task_available);
    sem_destroy(&sem_shared_resource);

    printf("All tasks completed. Shutting down.\n");
    return 0;
}
