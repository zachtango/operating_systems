#include <semaphore.h>
#include <queue>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	/* for strerror() */
#include <fstream>

#define errexit(code,str)                          \
  fprintf(stderr,"%s: %s\n",(str),strerror(code)); \
  exit(1);

constexpr unsigned int MAX_NUM_GUESTS {25};
constexpr const int NUM_GUESTS {25};
constexpr unsigned int NUM_DESK_EMPLOYEES {2};
constexpr unsigned int NUM_BELLHOP_EMPLOYEES {2};

int guestCount {0};
int roomCount {1};
int deskEmployeeCount {0};
int bellhopEmployeeCount {0};

int guestDeskEmployeeNumber[MAX_NUM_GUESTS];
int guestBellhopEmployeeNumber[MAX_NUM_GUESTS];
int guestRoom[MAX_NUM_GUESTS];

std::queue<int> guestDeskQueue;
std::queue<int> guestBellhopQueue;

sem_t coutMutex;
sem_t start;
sem_t created;

sem_t guestDeskQueueMutex;
sem_t guestBellhopQueueMutex;
sem_t guestDeskReady;
sem_t guestBellhopReady;
sem_t guestAssigned[MAX_NUM_GUESTS];
sem_t guestDelivered[MAX_NUM_GUESTS];

sem_t desk[NUM_DESK_EMPLOYEES];
sem_t deskEmployees;

sem_t tip[NUM_BELLHOP_EMPLOYEES];
sem_t bellhopEmployees;

sem_t roomCountMutex;

int getValue(sem_t &s) {
    int value;
    sem_getvalue(&s, &value);
    return value;
}

void *guest(void *arg) {
    int guestNumber = * (int *) arg;
    
    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " created\n";
    sem_post(&coutMutex);
    sem_post(&created);

    sem_wait(&start);

    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " enters hotel with 1 bag\n";
    sem_post(&coutMutex);

    // Wait for desk employee at front desk to be available
    sem_wait(&deskEmployees);

    // Let desk employee at front desk know you're ready
    sem_wait(&guestDeskQueueMutex);
    guestDeskQueue.push(guestNumber);
    sem_post(&guestDeskReady);
    sem_post(&guestDeskQueueMutex);

    // Wait for desk employee to assign room
    sem_wait(&guestAssigned[guestNumber]);

    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " receives room key for room " << guestRoom[guestNumber]
        << " from front desk employee " << guestDeskEmployeeNumber[guestNumber] << '\n';
    sem_post(&coutMutex);

    // Leave front desk to bellhop or room
    sem_post(&desk[guestDeskEmployeeNumber[guestNumber]]);

    // Wait for bellhop employee to be available
    sem_wait(&bellhopEmployees);

    // Let bellhop employee at front desk know you're ready
    sem_wait(&guestBellhopQueueMutex);
    guestBellhopQueue.push(guestNumber);
    sem_post(&guestBellhopReady);
    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " queued bags\n";
    sem_post(&coutMutex);
    sem_post(&guestBellhopQueueMutex);

    // Enter room
    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " enters room " << guestRoom[guestNumber] << '\n';
    sem_post(&coutMutex);

    // Wait for bellhop employee to deliver bags
    sem_wait(&guestDelivered[guestNumber]);

    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " receives bags from bellhop " << guestBellhopEmployeeNumber[guestNumber]
        << " and gives tip\n";
    sem_post(&coutMutex);

    // Send tip to bellhop employee
    sem_post(&tip[guestBellhopEmployeeNumber[guestNumber]]);

    sem_wait(&coutMutex);
    std::cout << "Guest " << guestNumber << " retires for the evening\n";
    sem_post(&coutMutex);

    return 0;
}

void *deskEmployee(void *arg) {

    int guestNumber;
    int deskEmployeeNumber = * (int *) arg;

    sem_wait(&coutMutex);
    std::cout << "Front desk employee " << deskEmployeeNumber << " created\n";
    sem_post(&coutMutex);
    sem_post(&created);

    sem_wait(&start);

    while(true) {
        sem_wait(&guestDeskReady);

        // Wait for guest to be available in queue
        sem_wait(&guestDeskQueueMutex);
        guestNumber = guestDeskQueue.front();
        guestDeskQueue.pop();
        sem_post(&guestDeskQueueMutex);

        // Let guest know what desk they're at
        guestDeskEmployeeNumber[guestNumber] = deskEmployeeNumber;

        // Assign room
        sem_wait(&roomCountMutex);
        guestRoom[guestNumber] = roomCount;
        roomCount += 1;
        sem_post(&roomCountMutex);

        sem_wait(&coutMutex);
        std::cout << "Front desk employee " << deskEmployeeNumber << " registers guest "
            << guestNumber << " and assigns room " << guestRoom[guestNumber] << '\n';
        sem_post(&coutMutex);

        // Let guest know their room was assigned
        sem_post(&guestAssigned[guestNumber]);

        // Wait for guest to leave
        sem_wait(&desk[deskEmployeeNumber]);
        
        sem_post(&deskEmployees);
    }

    return 0;
}


void *bellhopEmployee(void *arg) {

    int guestNumber;
    int bellhopEmployeeNumber = * (int *) arg;

    sem_wait(&coutMutex);
    std::cout << "Bellhop " << bellhopEmployeeNumber << " created\n";
    sem_post(&coutMutex);
    sem_post(&created);

    sem_wait(&start);

    while(true) {
        sem_wait(&guestBellhopReady);

        // Wait for guest to be available in queue
        sem_wait(&guestBellhopQueueMutex);
        guestNumber = guestBellhopQueue.front();
        guestBellhopQueue.pop();
        sem_post(&guestBellhopQueueMutex);

        // Let guest know which bellhop employee they have
        guestBellhopEmployeeNumber[guestNumber] = bellhopEmployeeNumber;

        // Take bags from guest
        sem_wait(&coutMutex);
        std::cout << "Bellhop " << bellhopEmployeeNumber << " receives bags from guest " << guestNumber << '\n';
        sem_post(&coutMutex);

        // Let guest know their bags were delivered
        sem_post(&guestDelivered[guestNumber]);

        // Wait for guest to tip
        sem_wait(&tip[bellhopEmployeeNumber]);
        
        sem_post(&bellhopEmployees);
    }

    return 0;
}


int main() {
    // Initialize semaphores
    sem_init(&coutMutex, 0, 1);
    sem_init(&start, 0, 0);
    sem_init(&created, 0, -(NUM_GUESTS + NUM_DESK_EMPLOYEES + NUM_BELLHOP_EMPLOYEES - 1));

    sem_init(&guestDeskQueueMutex, 0, 1);
    sem_init(&guestBellhopQueueMutex, 0, 1);
    sem_init(&guestDeskReady, 0, 0);
    for (int i = 0; i < NUM_GUESTS; i++) {
        sem_init(&guestAssigned[i], 0, 0);
    }
    
    for (int i = 0; i < NUM_DESK_EMPLOYEES; i++) {
        sem_init(&desk[i], 0, 0);
    }
    sem_init(&deskEmployees, 0, 2);
    
    for (int i = 0; i < NUM_BELLHOP_EMPLOYEES; i++) {
        sem_init(&tip[i], 0, 0);
    }
    sem_init(&bellhopEmployees, 0, 2);

    sem_init(&roomCountMutex, 0, 1);

    int deskEmployeeIds[NUM_DESK_EMPLOYEES];
    // Create desk employees
    pthread_t deskEmployeeThreads[NUM_DESK_EMPLOYEES];
    for (int worker = 0; worker < NUM_DESK_EMPLOYEES; worker++) {
        deskEmployeeIds[worker] = worker;
        if (pthread_create(&deskEmployeeThreads[worker], NULL, deskEmployee, &deskEmployeeIds[worker])) {
            std::cout << "desk employee thread create error\n";
            return 1;
        }
    }

    int bellhopEmployeeIds[NUM_BELLHOP_EMPLOYEES];
    // Create bellhop employees
    pthread_t bellhopEmployeeThreads[NUM_BELLHOP_EMPLOYEES];
    for (int worker = 0; worker < NUM_BELLHOP_EMPLOYEES; worker++) {
        bellhopEmployeeIds[worker] = worker;
        if (pthread_create(&bellhopEmployeeThreads[worker], NULL, bellhopEmployee, &bellhopEmployeeIds[worker])) {
            std::cout << "bellhop employee thread create error\n";
            return 1;
        }
    }

    int guestIds[NUM_GUESTS];
    // Create guests
    pthread_t guestThreads[NUM_GUESTS];
    for (int worker = 0; worker < NUM_GUESTS; worker++) {
        guestIds[worker] = worker;
        if (pthread_create(&guestThreads[worker], NULL, guest, &guestIds[worker])) {
            std::cout << "guest thread create error\n";
            return 1;
        }
    }

    sem_wait(&created);

    // All people have been created, unblock all the threads
    for (int i = 0; i < (NUM_GUESTS + NUM_DESK_EMPLOYEES + NUM_BELLHOP_EMPLOYEES); i++) {
        sem_post(&start);
    }

    int errcode;
    int *status;
    for (int worker = 0; worker < NUM_GUESTS; worker++) {
        if (errcode = pthread_join(guestThreads[worker],(void **) &status)) { 
            errexit(errcode,"pthread_join");
        } else {
            sem_wait(&coutMutex);
            std::cout << "Guest " << worker << " joined\n";
            sem_post(&coutMutex);
            
        }
    }

    return 0;
}


