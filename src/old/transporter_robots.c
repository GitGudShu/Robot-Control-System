#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define NUM_TRANSPORT_ROBOTS 2 // Number of transporter robots
#define SHM_KEY 1234

typedef struct {
    sem_t make_sem;
    sem_t paint_sem;
    sem_t transport_sem;
    int product_id;
} shared_semaphores;

int main() {
    // Attach to shared memory
    int shm_id = shmget(SHM_KEY, sizeof(shared_semaphores), 0666);
    if (shm_id == -1) {
        perror("Shared memory attachment failed");
        exit(EXIT_FAILURE);
    }

    shared_semaphores *sems = shmat(shm_id, NULL, 0);
    if (sems == (void *)-1) {
        perror("Shared memory mapping failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Wait for a task signal from the server
        sem_wait(&sems->transport_sem);

        // Access the current product ID
        int current_product_id = sems->product_id;
        printf("Transport Robot: Transporting product %d...\n", current_product_id);
        sleep(1); // Simulate transportation time
        printf("Transport Robot: Finished transporting product %d.\n", current_product_id);

        // Signal the server that transportation is done
        sem_post(&sems->transport_sem);
    }

    shmdt(sems);
    return 0;
}
