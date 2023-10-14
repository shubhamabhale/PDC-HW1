#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simulator.h"


#define MAX_CPU_CYCLES 1000000
#define NUM_MEMORY_MODULES 512
#define STANDARD_DEVIATION 5

double PI_VAL = 3.14159265358979323846;

struct Node* head = NULL;

// Node defined for linked list
typedef struct Node {
    int pNum; // processor ID
    int mem_mod; 
    int numAccess;
    struct Node* next;
} Node;

// Utility function to insert nodes in the circular linked list (queue)
void insertNode(int pNum) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->pNum = pNum;
    newNode->numAccess = 0;

    if (head == NULL) {
        newNode->next = newNode;
        head = newNode;
    } else {
        newNode->next = head;
        struct Node* temp = head;
        while (temp->next != head) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

//Function to assign random value considering the type of distribution
int assignRandomValue(char dist, int mean, int mem_size)
{
    if(dist=='u')
        return rand_uniform(mem_size);
    else 
        return rand_normal_wrap(mean, STANDARD_DEVIATION, mem_size);
}

// Function to allocate memory modules and update number of accesses for each process
Node* allocateMemoryModules(int memory_modules[], int mean, char dist, int mem_size) {

    Node* curr = head;
    Node* first_denied = NULL;
    do {
        if(memory_modules[curr->mem_mod] == 0) 
        {
            curr->numAccess += 1;
            memory_modules[curr->mem_mod] = curr->pNum;
            curr->mem_mod = assignRandomValue(dist, mean, mem_size);
            
        } else {
            if(first_denied==NULL)
                first_denied = curr;
        }
        curr = curr->next;
    } while(curr!=head);

    return first_denied;
}

//Function to form a circular queue using linked list
void createCircularQueue(int procs) {

    for (int i = 1; i <= procs; i++) {
        insertNode(i);
    }
}



void simulate(double* avg_access_time, int avg_access_time_len, int procs, char dist) 
{
    
    double T_previous = 0;
    const double epsilon = 0.02;
    int cpu_cycle;
    

    createCircularQueue(procs);

    for(int i=1; i<=NUM_MEMORY_MODULES; i++) 
    {
        int mean = rand_uniform(i);

        // code to assign random memory modules to processors in circular queue
        Node* temp  = head;
        do{ 
            temp->mem_mod = assignRandomValue(dist, mean , i);
        }while (temp!=head);


        float tot_tc_pi;
        float T_current;
        int memory_modules[i]; //array to keep track of free memory modules and set it 0

        for(int j=0;j<i;j++)
            memory_modules[j]=0;
        
        for(int cpu_cycle=1; cpu_cycle<=MAX_CPU_CYCLES; cpu_cycle++) 
        {
            // change head of the linked list to the first process denied memory module
            Node* denied_process = allocateMemoryModules(memory_modules, mean, dist, i);
            if (denied_process != NULL) {
                head = denied_process;
            }
            
            // Calculate T(S) for the current cycle
            Node* curr = head;
            tot_tc_pi = 0;
            float tc_pi = 0;

            do {
                if(curr->numAccess!=0)
                    tc_pi = (float) cpu_cycle/curr->numAccess;
                else 
                    tc_pi=0;

                tot_tc_pi += tc_pi;
                curr = curr->next;

            } while (curr != head);
            
            T_current = (float)(tot_tc_pi / procs);

            // Check the change between T_previous and T_current
            if (fabs((T_previous - T_current) / T_current) < epsilon) {
                break;
            }
            T_previous = T_current;   
            //free up memory modules after each cycle
            for(int itr = 0; itr<i;itr++) 
            {
                memory_modules[itr] = 0;
            }
        }

        Node* curr = head;
        do {
            curr->numAccess=0;
            curr = curr->next;
        }while(curr!=head);

        avg_access_time[i-1] = T_current;

        if (cpu_cycle == MAX_CPU_CYCLES) {
            printf("Simulation ended due to reaching maximum CPU cycles.\n");
        }
    }
}

int rand_uniform(int max) {
    return rand() % max;
}

int rand_normal_wrap(int mean, int dev, int max) {
    static double U, V;
    static int phase = 0;
    double Z;
    if (phase == 0) {
        U = (rand() + 1.) / (RAND_MAX + 2.);
        V = rand() / (RAND_MAX + 1.);
        Z = sqrt(-2 * log(U)) * sin(2 * PI_VAL * V);
    } else {
        Z = sqrt(-2 * log(U)) * cos(2 * PI_VAL * V);
    }
    phase = 1 - phase;
    double res = dev * Z + mean;

    int res_int;
    if ((int)res % 2 == 0)
        res_int = (int)(res + 1);
    else
        res_int = (int)(res);

    int res_wrapped = res_int % max;
    if (res_wrapped < 0)
        res_wrapped += max;
    return res_wrapped;
}