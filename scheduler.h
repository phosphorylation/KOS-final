#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "dllist.h"
#include "simulator.h"
#include "kt.h"
#include "kos.h"
// void initialize_user_process();
 struct PCB{
  int registers[NumTotalRegs];
  int sbrk; //the address of the current Process's allocated end
  int Base; //the user memory base
  int Limit; //the user memory limit
  int PID; //Process ID
  struct PCB* parent; // parent process ID
  int exitvalue;
  kt_sem waiter_sem;
  Dllist waiters;
  Dllist children;
  Dllist fDtable; //it's own fDtable
  Dllist fDstorage; //100
};
 int noopF;
 //struct PCB *current;
 Dllist PIDstorage;
 int PIDstoragesize;
 Dllist readyq;
void run_scheduler();
void initialize_PID(int PIDstoragesize);
int get_new_pid();
void return_pid(int PID);
#endif