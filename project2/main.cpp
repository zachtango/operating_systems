#include <semaphore.h>
#include <queue>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fstream>
#include <string>
#include <stdint.h>

#define errexit(code,str)                          \
  fprintf(stderr,"%s: %s\n",(str),strerror(code)); \
  exit(1);

constexpr unsigned int MAX_NUM_GUESTS {25};
constexpr unsigned int NUM_DESK_EMPLOYEES {2};
constexpr unsigned int NUM_BELLHOP_EMPLOYEES {2};

int NUM_GUESTS;

int peopleCount {0};
int roomCount {1};

int guestDeskEmployeeNumber[MAX_NUM_GUESTS];
int guestBellhopEmployeeNumber[MAX_NUM_GUESTS];
int guestRoom[MAX_NUM_GUESTS];

std::queue<int> guestDeskQueue;
std::queue<int> guestBellhopQueue;

/* Guest semaphores */
sem_t guestDeskQueueMutex;
sem_t guestBellhopQueueMutex;
sem_t guestDeskReady;
sem_t guestBellhopReady;
sem_t guestAssigned[MAX_NUM_GUESTS];
sem_t guestTaken[MAX_NUM_GUESTS];
sem_t guestDelivered[MAX_NUM_GUESTS];

/* Desk employee semaphores */
sem_t desk[NUM_DESK_EMPLOYEES];
sem_t deskEmployees;

/* Bellhop employee semaphores */
sem_t tip[NUM_BELLHOP_EMPLOYEES];
sem_t bellhopEmployees;

/* Misc semaphores */
sem_t printMutex;
sem_t start;
sem_t created;
sem_t roomCountMutex;
sem_t peopleCountMutex;

void createPerson(const std::string &p, int id);

/* Guest helper functions */
void enterHotel(int guestNumber, int bags);
void enqueue(std::queue<int> &q, int guestNumber, sem_t &queueMutex, sem_t &readyMutex);
void receiveKey(int guestNumber);
void requestHelp(int guestNumber);
void enterRoom(int guestNumber);
void receiveBags(int guestNumber);
void retire(int guestNumber);

/* Employee helper functions */
void dequeue(std::queue<int> &q, int &guestNumber, sem_t &queueMutex);
void assignRoom(int deskEmployeeNumber, int guestNumber);
void takeBags(int bellhopEmployeeNumber, int guestNumber);

/* Threads functions for the people */
void *guest(void *arg);
void *deskEmployee(void *arg);
void *bellhopEmployee(void *arg);


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ./hotel <number of guests>\n" <<
            "Example: ./hotel 3\n";
        return 0;
    }

    NUM_GUESTS = atoi(argv[1]);

    if (NUM_GUESTS < 1 || NUM_GUESTS > 25) {
        std::cout << "<number of guests> must be between 1 and 25 inclusive\n";
        return 0;
    }

    std::cout << "Simulation starts\n";

    // For random seed
    srand(time(NULL));

    // Initialize semaphores
    sem_init(&printMutex, 0, 1);
    sem_init(&start, 0, 0);
    sem_init(&created, 0, 0);

    sem_init(&guestDeskQueueMutex, 0, 1);
    sem_init(&guestBellhopQueueMutex, 0, 1);
    sem_init(&guestDeskReady, 0, 0);
    for (int i = 0; i < NUM_GUESTS; i++) {
        sem_init(&guestAssigned[i], 0, 0);
    }
    for (int i = 0; i < NUM_GUESTS; i++) {
        sem_init(&guestTaken[i], 0, 0);
    }
    for (int i = 0; i < NUM_GUESTS; i++) {
        sem_init(&guestDelivered[i], 0, 0);
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
    sem_init(&peopleCountMutex, 0, 1);

    // Thread info
    int errcode;
    int *status;
    int deskEmployeeIds[NUM_DESK_EMPLOYEES];
    int bellhopEmployeeIds[NUM_BELLHOP_EMPLOYEES];
    int guestIds[NUM_GUESTS];

    // Create desk employees
    pthread_t deskEmployeeThreads[NUM_DESK_EMPLOYEES];
    for (int worker = 0; worker < NUM_DESK_EMPLOYEES; worker++) {
        deskEmployeeIds[worker] = worker;

        if (pthread_create(&deskEmployeeThreads[worker], NULL, deskEmployee, &deskEmployeeIds[worker])) {
            errexit(errcode, "pthread_create");
        }
    }

    // Create bellhop employees
    pthread_t bellhopEmployeeThreads[NUM_BELLHOP_EMPLOYEES];
    for (int worker = 0; worker < NUM_BELLHOP_EMPLOYEES; worker++) {
        bellhopEmployeeIds[worker] = worker;

        if (errcode = pthread_create(&bellhopEmployeeThreads[worker], NULL, bellhopEmployee, &bellhopEmployeeIds[worker])) {         /* arg to routine            */
            errexit(errcode, "pthread_create");
        }
    }

    // Create guests
    pthread_t guestThreads[NUM_GUESTS];
    for (int worker = 0; worker < NUM_GUESTS; worker++) {
        guestIds[worker] = worker;

        if (errcode = pthread_create(&guestThreads[worker], NULL, guest, &guestIds[worker])) {         /* arg to routine            */
            errexit(errcode, "pthread_create");
        }
    }

    sem_wait(&created);

    // All people have been created, unblock all the threads
    for (int i = 0; i < (NUM_GUESTS + NUM_DESK_EMPLOYEES + NUM_BELLHOP_EMPLOYEES); i++) {
        sem_post(&start);
    }

    // Join the guests back
    for (int worker = 0; worker < NUM_GUESTS; worker++) {
        if (errcode = pthread_join(guestThreads[worker],(void **) &status)) { 
            errexit(errcode,"pthread_join");
        }
        
        // Thread's exist status should be the same as the id assigned
        if (*status != worker) {
            fprintf(stderr,"thread %d terminated abnormally\n",worker);
            exit(1);
        } else {
            sem_wait(&printMutex);
            std::cout << "Guest " << worker << " joined\n";
            sem_post(&printMutex);
        }
    }

    std::cout << "Simulation ends\n";

    return 0;
}

void createPerson(const std::string &p, int id) {
    sem_wait(&printMutex);
    std::cout << p << ' ' << id << " created\n";
    sem_post(&printMutex);

    sem_wait(&peopleCountMutex);
    peopleCount += 1;

    if (peopleCount == (NUM_GUESTS + NUM_DESK_EMPLOYEES + NUM_BELLHOP_EMPLOYEES)) {
        // All the necessary people in the simulation have been created
        //  let the main thread know the simulation can start
        sem_post(&created);
    }
    sem_post(&peopleCountMutex);

    sem_wait(&start);
}

void enterHotel(int guestNumber, int bags) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " enters hotel with " << bags <<
        (bags == 1 ? " bag\n" : " bags\n");
    sem_post(&printMutex);
}

void enqueue(std::queue<int> &q, int guestNumber, sem_t &queueMutex, sem_t &readyMutex) {
    sem_wait(&queueMutex);
    q.push(guestNumber);
    sem_post(&readyMutex);
    sem_post(&queueMutex);
}

void receiveKey(int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " receives room key for room " << guestRoom[guestNumber]
        << " from front desk employee " << guestDeskEmployeeNumber[guestNumber] << '\n';
    sem_post(&printMutex);
}

void requestHelp(int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " requests help with bags\n";
    sem_post(&printMutex);
}

void enterRoom(int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " enters room " << guestRoom[guestNumber] << '\n';
    sem_post(&printMutex);
}

void receiveBags(int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " receives bags from bellhop " << guestBellhopEmployeeNumber[guestNumber]
        << " and gives tip\n";
    sem_post(&printMutex);
}

void retire(int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Guest " << guestNumber << " retires for the evening\n";
    sem_post(&printMutex);
}

void dequeue(std::queue<int> &q, int &guestNumber, sem_t &queueMutex) {
    sem_wait(&queueMutex);
    guestNumber = q.front();
    q.pop();
    sem_post(&queueMutex);
}

void assignRoom(int deskEmployeeNumber, int guestNumber) {
    sem_wait(&roomCountMutex);
    guestRoom[guestNumber] = roomCount;
    roomCount += 1;
    sem_post(&roomCountMutex);

    sem_wait(&printMutex);
    std::cout << "Front desk employee " << deskEmployeeNumber << " registers guest "
        << guestNumber << " and assigns room " << guestRoom[guestNumber] << '\n';
    sem_post(&printMutex);
}

void takeBags(int bellhopEmployeeNumber, int guestNumber) {
    sem_wait(&printMutex);
    std::cout << "Bellhop " << bellhopEmployeeNumber << " receives bags from guest " << guestNumber << '\n';
    sem_post(&printMutex);
}

void *guest(void *arg) {
    int guestNumber {* (int *) arg};
    int bags {rand() % 6}; // random number between 0 and 5

    createPerson("Guest", guestNumber);

    enterHotel(guestNumber, bags);

    // Wait for desk employee at front desk to be available
    sem_wait(&deskEmployees);

    // Let desk employee at front desk know you're ready
    enqueue(guestDeskQueue, guestNumber, guestDeskQueueMutex, guestDeskReady);

    // Wait for desk employee to assign room
    sem_wait(&guestAssigned[guestNumber]);

    receiveKey(guestNumber);

    // Leave front desk to bellhop or room
    sem_post(&desk[guestDeskEmployeeNumber[guestNumber]]);

    if (bags > 2) {
        requestHelp(guestNumber);

        // Wait for bellhop employee to be available
        sem_wait(&bellhopEmployees);

        // Let bellhop employee know you're ready
        enqueue(guestBellhopQueue, guestNumber, guestBellhopQueueMutex, guestBellhopReady);
        
        // Wait for bags to be taken
        sem_wait(&guestTaken[guestNumber]);
    }

    // Enter room
    enterRoom(guestNumber);

    if (bags > 2) {
        // Wait for bellhop employee to deliver bags
        sem_wait(&guestDelivered[guestNumber]);

        receiveBags(guestNumber);

        // Send tip to bellhop employee
        sem_post(&tip[guestBellhopEmployeeNumber[guestNumber]]);
    }

    retire(guestNumber);

    return arg;
}

void *deskEmployee(void *arg) {

    int guestNumber;
    int deskEmployeeNumber {* (int *) arg};

    createPerson("Front desk employee", deskEmployeeNumber);

    while(true) {
        sem_wait(&guestDeskReady);

        // Wait for guest to be available in queue
        dequeue(guestDeskQueue, guestNumber, guestDeskQueueMutex);

        // Let guest know what desk they're at
        guestDeskEmployeeNumber[guestNumber] = deskEmployeeNumber;

        // Assign room
        assignRoom(deskEmployeeNumber, guestNumber);

        // Let guest know their room was assigned
        sem_post(&guestAssigned[guestNumber]);

        // Wait for guest to leave
        sem_wait(&desk[deskEmployeeNumber]);
        
        // Become available for next guest
        sem_post(&deskEmployees);
    }

    return arg;
}

void *bellhopEmployee(void *arg) {

    int guestNumber;
    int bellhopEmployeeNumber {* (int *) arg};

    createPerson("Bellhop", bellhopEmployeeNumber);

    while(true) {
        sem_wait(&guestBellhopReady);

        // Wait for guest to be available in queue
        dequeue(guestBellhopQueue, guestNumber, guestBellhopQueueMutex);

        // Let guest know which bellhop employee they have
        guestBellhopEmployeeNumber[guestNumber] = bellhopEmployeeNumber;

        // Take bags from guest
        takeBags(bellhopEmployeeNumber, guestNumber);

        // Let guest know their bags are taken
        sem_post(&guestTaken[guestNumber]);

        // Let guest know their bags were delivered
        sem_post(&guestDelivered[guestNumber]);

        // Wait for guest to tip
        sem_wait(&tip[bellhopEmployeeNumber]);
        
        sem_post(&bellhopEmployees);
    }

    return arg;
}


