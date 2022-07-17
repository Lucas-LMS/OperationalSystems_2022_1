#include "headers.h"

int lendoPagsWS(Processo *processo, int pageNumber);
Processo* criarProcesso(int pid);
RAM* criarRAM();
void addPagsWS(Processo *processo, int pageNumber, int address);
PageValues removeLRUpag(Processo *processo);
void removePagRAM(RAM *ram, int page);
int addPagRAM(RAM *ram);
int RAMcheia(RAM *ram);
int WSVazio(Processo *processo);
void printTLB(Processo *processo);
int getFrames();
int getMaxProcesses();
int getNumPages();
int getWorkingSetLimit();
int getWaitTime();
int getStoppingLimit();

int frames, maxProcesses, numPages, workingSetLimit, waitTime, stoppingLimit;

/**
 * Getters para constantes
 */
int getFrames(){
    return frames;
}

int getMaxProcesses(){
    return maxProcesses;
}

int getNumPages(){
    return numPages;
}

int getWorkingSetLimit(){
    return workingSetLimit;
}

int getWaitTime(){
    return waitTime;
}

int getStoppingLimit(){
    return stoppingLimit;
}



/**
 * @brief Cria um processo
 * 
 * @param pid Id do processo
 * @return Processo* Processo criado
 */
Processo* criarProcesso(int pid){
    Processo *processo = (Processo *)malloc(sizeof(Processo));
    if(processo == NULL){
        exit(2);
    }

    processo->pid = pid;
    processo->workingSet = (WS *)malloc(sizeof(WS));

    if(processo->workingSet == NULL){
        exit(2);
    }

    processo->workingSet->head = NULL;
    processo->workingSet->tail = NULL;
    processo->workingSet->remainingSlots = WORKING_SET_LIMIT;

    for(int i = 0; i < NUM_PAGES; i++){
        processo->workingSet->rows[i] = -1;
    }

    return processo;
}

/**
 * @brief Cria vetor de memória principal
 * 
 * @return RAM* Vetor de memória
 */
RAM* criarRAM(){
    RAM* ram = (RAM *)malloc(sizeof(RAM));

    if(!ram){
        exit(3);
    }

    ram->remainingSlots = FRAMES;
    for(int i = 0; i < FRAMES; i++){
        ram->addresses[i] = 0;
    }
    
    return ram;
}

/**
 * @brief Remove a pagina menos recentemente usada do WS do processo
 * 
 * @param processo Processo
 * @return PageValues Endereco e pagina virtual da pagina removida
 */
PageValues removeLRUpag(Processo *processo){
    int page = processo->workingSet->head->pageNumber;
    int address = processo->workingSet->rows[page];

    PageValues pv;
    pv.address = address;
    pv.page = page;
    
    PageElement *aux = processo->workingSet->head;
    processo->workingSet->head = processo->workingSet->head->next;
    free(aux);
    if(processo->workingSet->head) processo->workingSet->head->prev = NULL;
    processo->workingSet->remainingSlots++;

    processo->workingSet->rows[page] = -1;

    return pv;
}

/**
 * @brief Adiciona uma pagina do working set do processo
 * 
 * @param processo Processo
 * @param pageNumber Numero da pagina a ser adicionada
 * @param address Endereco a ser adicionado
 */
void addPagsWS(Processo *processo, int pageNumber, int address){
    if(processo->workingSet->head == NULL){
        processo->workingSet->head = (PageElement *)malloc(sizeof(PageElement));
        processo->workingSet->tail = processo->workingSet->head;
        processo->workingSet->tail->next = NULL;
        processo->workingSet->tail->prev = NULL;
    }else{
        processo->workingSet->tail->next = (PageElement *)malloc(sizeof(PageElement));
        processo->workingSet->tail->next->prev = processo->workingSet->tail;
        processo->workingSet->tail->next->next = NULL;
        processo->workingSet->tail = processo->workingSet->tail->next;
    }
    
    processo->workingSet->remainingSlots--;
    processo->workingSet->tail->pageNumber = pageNumber;

    processo->workingSet->rows[pageNumber] = address;
}

/**
 * @brief Le uma pagina do working set do processo
 * 
 * @param processo Processo
 * @param pageNumber Numero da pagina a ser lida
 * @return int Endereco da pagina lida ou -1 se pagina nao encontrada
 */
int lendoPagsWS(Processo *processo, int pageNumber){
    PageElement *element = processo->workingSet->head;
    while(element != NULL){
        if(element->pageNumber == pageNumber){
            // Retira elemento da lista
            if(element->prev) element->prev->next = element->next;
            else processo->workingSet->head = element->next;

            if(element->next) element->next->prev = element->prev;
            else processo->workingSet->tail = processo->workingSet->tail->prev;

            // Adiciona no final
            if(processo->workingSet->head == NULL){
                processo->workingSet->head = element;
                element->prev = NULL;
            }else{
                processo->workingSet->tail->next = element;
                element->prev = processo->workingSet->tail;
            }
            element->next = NULL;
            processo->workingSet->tail = element;

            return processo->workingSet->rows[pageNumber];
        }
        element = element->next;
    }
    return -1;
}

int addPagRAM(RAM *ram){
    for(int i = 0; i < FRAMES; i++){
        if(ram->addresses[i] == 0){
            ram->addresses[i] = 1;
            ram->remainingSlots--;
            return i;
        }
    }
    return -1;
}

void removePagRAM(RAM *ram, int page){
    ram->addresses[page] = 0;
    ram->remainingSlots++;
}

int RAMcheia(RAM *ram){
    return ram->remainingSlots == 0;
}

int WSVazio(Processo *processo){
    return processo->workingSet->head == NULL;
}

void printTLB(Processo *processo){
    printf("-------\n");
    printf("| Processo %-2d |\n",processo->pid); //processo
    printf("-------\n");
    for(int i = 0; i < NUM_PAGES; i++){
        int address = processo->workingSet->rows[i];
        if(address == -1) continue;
        printf("|Página %02d|Endereço %02d|\n", i, address); //pagina | endereco
        printf("-------\n");
    }
}

/*
 * Verifica se o parametro e um numero
 */
int handleParameter(char *ps){
    if(ps[0] == '\0') terminaPrograma(4, "Parametro nao possui numero");

    int value = 0;
    for(char *s = ps; *s != '\0'; s++){
        if(*s < '0' || *s > '9') terminaPrograma(4, "Parametro nao possui um numero valido");

        value = value * 10 + (*s - '0');
    } 
    return value;
}

/*
 * Le os argumentos CLI
 */
void lerArgumentosConsole(int argc, char *argv[]){
    frames = 64;
    maxProcesses = 20;
    numPages = 50;
    workingSetLimit = 4;
    waitTime = 3;
    stoppingLimit = -1;

    for(int i = 1; i < argc; i++){
        char *arg = argv[i];
        if(arg[0] != '-' || arg[1] == '\0') terminaPrograma(5, "Argumento inválido");
        char flag = arg[1];
        arg += 2;
        
        switch(flag){
            case 'f':
                frames = handleParameter(arg);
                if(frames > MAX_FRAMES) terminaPrograma(6, "Frames máximos atingidos");
                break;
            case 'p':
                maxProcesses = handleParameter(arg);
                if(maxProcesses > MAX_PROCESSES) terminaPrograma(6, "Processos máximos atingidos");
                break;
            case 'v':
                numPages = handleParameter(arg);
                if(numPages > MAX_PAGES) terminaPrograma(6, "Páginas máximas atingidas");
                break;
            case 'w':
                workingSetLimit = handleParameter(arg);
                break;
            case 's':
                waitTime = handleParameter(arg);
                break;
            case 'l':
                stoppingLimit = handleParameter(arg);
                break;
            default:
                terminaPrograma(5, "Argumento inválido");
        }
    }
}

int requestPage();
void pageFault(Processo *processo, int pageNumber);
int getProcessoMaisAntigo();

int activeProcesses = 0;
int lastProcessRemoved = 0;

Processo* processes[MAX_PROCESSES];
RAM *ram;

int main(int argc, char *argv[]){
    srand(time(NULL));

    lerArgumentosConsole(argc, argv);

    ram = criarRAM();
    int count = 0;
    while(1){
        if(STOPPING_LIMIT != -1 && count++ >= STOPPING_LIMIT) terminaPrograma(7, "Contador terminou");
        for(int i = 0; i < activeProcesses; i++){
            int requestedPage = requestPage();
            printf("Processo %d solicitando página %d\n", i, requestedPage);

            int address = lendoPagsWS(processes[i], requestedPage);

            if(address != -1){
                printf("Página encontrada em %d\n",address);
            }else{
                pageFault(processes[i], requestedPage);
            }
            printTLB(processes[i]);
        }
        if(activeProcesses < PROCESSES) {
            processes[activeProcesses] = criarProcesso(activeProcesses);

            int requestedPage = requestPage();
            printf("Processo %d foi criado e esta solicitando página %d\n", activeProcesses, requestedPage);

            pageFault(processes[activeProcesses], requestedPage);

            printTLB(processes[activeProcesses]);

            activeProcesses++;
        }
        sleep(WAIT_TIME);
    }
    return 0;
}

int requestPage(){
    return rand() % NUM_PAGES;
}

/**
 * @brief Realiza a adicao de pagina na memoria
 * 
 * @param processo Processo que teve Page Fault
 * @param pageNumber Numero da pagina requisitada
 */
void pageFault(Processo *processo, int pageNumber){
    if(processo->workingSet->remainingSlots == 0){
        PageValues pv = removeLRUpag(processo);
        removePagRAM(ram, pv.address);

        int address = addPagRAM(ram);
        addPagsWS(processo, pageNumber, address);

        printf("Página %d removida. Página %d adicionada no endereço %d\n", pv.page, pageNumber, address);
    }else{
        if(RAMcheia(ram)){
            int oldestPid = getProcessoMaisAntigo();
            Processo *processoMaisAntigo = processes[oldestPid];
            while(!WSVazio(processoMaisAntigo)){
                PageValues pv = removeLRUpag(processoMaisAntigo);
                removePagRAM(ram, pv.address);
            }
            printf("Memória principal cheia, o processo %d foi removido\n", oldestPid);
        }
        int address = addPagRAM(ram);
        addPagsWS(processo, pageNumber, address);
        printf("Página %d adicionada no endereco %d\n", pageNumber, address);
    }
}

int getProcessoMaisAntigo(){
    int processoMaisAntigo = lastProcessRemoved;
    lastProcessRemoved = (lastProcessRemoved + 1) % activeProcesses;

    return processoMaisAntigo;
}

void terminaPrograma(int error, char *errorMessage){
    if(!error) return;
    printf("%s\n", errorMessage);
    exit(error);
}