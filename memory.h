#include "dllist.h"
#include "simulator.h"
#include "kt.h"
#include "kos.h"
struct fd{
    int fdnum;
    int permit; //0 for write, 1 for read
    int* openTime;
    int* read; //how many is reading this pipe
    int* write; //how many is writing this pipe
    kt_sem* slots; //4*1024
    kt_sem* readOK;//0
    kt_sem* bufferWriter; //1
    char* buffer; //starting positon of buffer 
    char** offset; //current offset of this fd
};
//Dllist fDstorage;  //total available fD
int fDstoragesize; //100
int buffersize;
//Dllist fDtable;    //assigned fDs
void initFd(struct PCB* pcb,int fDstoragesize);
int getFd(struct PCB* pcb);
void returnFdnum(struct PCB* pcb,int fdnum);
struct fd* createFdDup(struct PCB* pcb,char *buffer,char **offset,kt_sem* nslots,kt_sem* readOK,kt_sem* bufferWriter,int* openTime,int RoW,int* read,int* write);
struct fd* createFdDup2(struct PCB* pcb,char *buffer,char **offset,kt_sem* nslots,kt_sem* readOK,kt_sem* bufferWriter,int* openTime,int RoW,int* read,int* write);
struct fd* LookupFDTable(struct PCB* pcb,int fdnum);
void copytable(struct PCB* pcb,struct PCB* child);
void copystorage(struct PCB* pcb,struct PCB* child);
void deleteFDTable(struct PCB* pcb,int fdnum);
int ValidateAddress(int r5,struct PCB *);
