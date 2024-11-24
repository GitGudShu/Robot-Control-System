#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define NUM_PRODUCTS 10 // Total number of products
#define SHM_KEY 1234

typedef struct {
    sem_t make_sem;
    sem_t paint_sem;
    sem_t transport_sem;
    int product_id;
} shared_semaphores;

int main() {
    // Create shared memory segment
    int shm_id = shmget(SHM_KEY, sizeof(shared_semaphores), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Shared memory creation failed");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    shared_semaphores *sems = shmat(shm_id, NULL, 0);
    if (sems == (void *)-1) {
        perror("Shared memory attachment failed");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphores and shared product ID
    sem_init(&sems->make_sem, 1, 0);  // Initially 0, waits for "make" robots to start
    sem_init(&sems->paint_sem, 1, 0); // Initially 0, waits for "paint" robots to start
    sem_init(&sems->transport_sem, 1, 0); // Initially 0, waits for "transport" robots to start
    sems->product_id = 0; // Product ID starts from 0

    printf("Server: Starting production line...\n");

    for (int i = 1; i <= NUM_PRODUCTS; i++) {
        sems->product_id = i; // Assign product ID for this cycle
        printf("Server: Processing product %d...\n", sems->product_id);

        printf("Server: Signaling 'make' robots...\n");
        sem_post(&sems->make_sem);
        sem_wait(&sems->make_sem);

        printf("Server: Signaling 'paint' robots...\n");
        sem_post(&sems->paint_sem);
        sem_wait(&sems->paint_sem);

        printf("Server: Signaling 'transport' robots...\n");
        sem_post(&sems->transport_sem);
        sem_wait(&sems->transport_sem);

        printf("Server: Product %d completed.\n", i);
    }

    // Cleanup
    sem_destroy(&sems->make_sem);
    sem_destroy(&sems->paint_sem);
    sem_destroy(&sems->transport_sem);
    shmdt(sems);
    shmctl(shm_id, IPC_RMID, NULL);

    printf("Server: All products processed. Shutting down.\n");
    return 0;
}
