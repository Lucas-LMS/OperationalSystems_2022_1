#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MAX_FRAMES 128
#define MAX_PROCESSES 32
#define MAX_PAGES 64

#define FRAMES getFrames()
#define PROCESSES getMaxProcesses()
#define NUM_PAGES getNumPages()
#define WORKING_SET_LIMIT getWorkingSetLimit()
#define WAIT_TIME getWaitTime()
#define STOPPING_LIMIT getStoppingLimit()

typedef struct PageElement PageElement;
typedef struct Processo Processo;
typedef struct WS WS;
typedef struct RAM RAM;
typedef struct PageValues PageValues;

struct PageElement{
    int pageNumber;
    struct PageElement *prev;
    struct PageElement *next;
};

struct Processo{
    int pid;
    int ppid;
    int status;
    int priority;

    WS *workingSet;
};

struct WS{
    PageElement *head;
    PageElement *tail;
    int remainingSlots;
    int rows[MAX_PAGES];
};

struct RAM{
    int addresses[MAX_FRAMES];
    int remainingSlots;
};

struct PageValues{
    int page;
    int address;
};

void terminaPrograma(int error, char *errorMessage);