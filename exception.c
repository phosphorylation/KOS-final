/*
 * exception.c -- stub to handle user mode exceptions, including system calls
 * 
 * Everything else core dumps.
 * 
 * Copyright (c) 1992 The Regents of the University of California. All rights
 * reserved.  See copyright.h for copyright notice and limitation of
 * liability and disclaimer of warranty provisions.
 */
#include <stdlib.h>
#include <errno.h>
#include "kt.h"
#include "simulator.h"
#include "scheduler.h"
#include "kos.h"
#include "console_buf.h"
#include "syscall.h"

void exceptionHandler(ExceptionType which)
{
  DEBUG('e', "IN EXCEPTION\n");
  int type, r5, r6, r7, newPC;
  int buf[NumTotalRegs];
  examine_registers(buf);
  type = buf[4];
  r5 = buf[5];
  r6 = buf[6];
  r7 = buf[7];
  newPC = buf[NextPCReg];
  User_Base=CurrentPCB->Base;
  User_Limit=CurrentPCB->Limit;
  for (int i = 0; i < NumTotalRegs; i++)
  {
      CurrentPCB->registers[i] = buf[i];
  }
	/*
	 * for system calls type is in r4, arg1 is in r5, arg2 is in r6, and
	 * arg3 is in r7 put result in r2 and don't forget to increment the
	 * pc before returning!
	 */

	switch (which) {
	case SyscallException:
		/* the numbers for system calls is in <sys/syscall.h> */
		switch (type) {
		case 0:
			/* 0 is our halt system call number */
			DEBUG('e', "Halt initiated by user program\n");
			SYSHalt();
		case SYS_exit:
			/* this is the _exit() system call */
			DEBUG('e', "_exit() system call\n");
			//printf("Program exited with value %d.\n", r5);
      //SYSHalt();
      kt_fork((void *)handle_exit,(void *)CurrentPCB);
			break;
   case SYS_ioctl:
     DEBUG('e', "ioctl system call\n");
     kt_fork((void *)handle_iotcl,(void *)CurrentPCB);
     break;
       
   case SYS_fstat:
     DEBUG('e', "SYS_fstat system call\n");
     kt_fork((void *)handle_fstat,(void *)CurrentPCB);
     break;
   
   case SYS_getpagesize:
     DEBUG('e', "SYS_getpagesize system call\n");
     kt_fork((void *)handle_pagesize,(void *)CurrentPCB);
     break;
     
   case SYS_sbrk:
     DEBUG('e', "SYS_sbrk system call\n");
     kt_fork((void *)handle_sbrk,(void *)CurrentPCB);
     break;

   case SYS_execve:
     DEBUG('e', "SYS_execve system call\n");
      kt_fork((void *)handle_execve,(void *)CurrentPCB);
      break;

   case SYS_getpid:
     DEBUG('e', "SYS_getpid system call\n");
      kt_fork((void *)handle_getpid,(void *)CurrentPCB);
      break;
    case SYS_fork:
    DEBUG('e', "SYS_fork system call\n");
       kt_fork((void *)handle_fork,(void *)CurrentPCB);
      break;

    case SYS_getdtablesize:
    DEBUG('e', "SYS_getdtablesize system call\n");
       kt_fork((void *)handle_tablesize,(void *)CurrentPCB);
      break;

    case SYS_close:
    DEBUG('e', "SYS_close system call\n");
       kt_fork((void *)handle_close,(void *)CurrentPCB);
      break;

    case SYS_getppid:
    kt_fork((void *)&handle_getppid,(void *)CurrentPCB);
     DEBUG('e', "SYS_getppid system call\n");
     break;
    
    case SYS_wait:
    kt_fork((void *)&handle_wait,(void *)CurrentPCB);
     DEBUG('e', "SYS_wait system call\n");
     break;
    
    case SYS_dup:
    kt_fork((void *)&handle_dup,(void *)CurrentPCB);
     DEBUG('e', "SYS_dup system call\n");
     break;
     
    case SYS_dup2:
    kt_fork((void *)&handle_dup2,(void *)CurrentPCB);
     DEBUG('e', "SYS_dup2 system call\n");
     break;

    case SYS_pipe:
    kt_fork((void *)&handle_pipe,(void *)CurrentPCB);
     DEBUG('e', "SYS_pipe system call\n");
     break;

   case SYS_write:
     kt_fork((void *)&do_write,(void *)CurrentPCB);
     DEBUG('e', "SYS_write system call\n");
     break;
   case SYS_read:
     kt_fork((void *)&do_read,(void *)CurrentPCB);
     DEBUG('e', "SYS_read system call\n");
     break;
    
		default:
			DEBUG('e', "Unknown system call\n");
			SYSHalt();
			break;
		}
		break;
   ////////////////////above are syscalls
   ////////////////////below are other exceptions
	case PageFaultException:
		DEBUG('e', "Exception PageFaultException\n");
		break;
	case BusErrorException:
		DEBUG('e', "Exception BusErrorException\n");
		break;
	case AddressErrorException:
		DEBUG('e', "Exception AddressErrorException\n");
     //syscall_return(CurrentPCB,0);
		break;
	case OverflowException:
		DEBUG('e', "Exception OverflowException\n");
		break;
	case IllegalInstrException:
		DEBUG('e', "Exception IllegalInstrException\n");
		SYSHalt();
		break;
	default:
		printf("Unexpected user mode exception %d %d\n", which, type);
		exit(1);
	}
    kt_joinall();
	run_scheduler();
}

void interruptHandler(IntType which)
{
    //struct PCB *interrupt_block = malloc(sizeof(struct PCB));
    // for(int i=0;i<NumTotalRegs;i++)
    // {
    //     CurrentPCB->registers[i]=0;
    // }
    if(noopF==0)
    {
        int type, r5, r6, r7, newPC;
        int buf[NumTotalRegs];
         examine_registers(buf);
        type = buf[4];
        r5 = buf[5];
        r6 = buf[6];
        r7 = buf[7];
        newPC = buf[NextPCReg];
        //buf[PCReg]=newPC;
        for (int i = 0; i < NumTotalRegs; i++) {
            CurrentPCB->registers[i] = buf[i];
        }
         //CurrentPCB->sbrk=CurrentPCB->sbrk;
        dll_append(readyq,new_jval_v(CurrentPCB));
    }
    else
    {
        //free(CurrentPCB);
    }

	switch (which) {
	case ConsoleReadInt:
		DEBUG('e', "ConsoleReadInt interrupt\n");

    V_kt_sem(cBuffer->consoleWait);
		//kt_yield();
    //P_kt_sem(cBuffer->nelem);
		break;
   
	case ConsoleWriteInt:
		DEBUG('e', "ConsoleWriteInt interrupt\n");

    V_kt_sem(writeOk);
		break;
   
  case TimerInt:
    DEBUG('e', "TimeInt interrupt\n");
    start_timer(10);
    break;
  
	default:
		DEBUG('e', "Unknown interrupt\n");
		break;
   
	}
    kt_joinall();
	run_scheduler();
 DEBUG('e', "shoud stop\n");
}
