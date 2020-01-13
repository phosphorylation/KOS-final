/*
 * kos.c -- starting point for student's os.
 * 
 */
#include <stdlib.h>
#include "kt.h"
#include "simulator.h"
#include "scheduler.h"
#include"dllist.h"
#include "kos.h"
#include "console_buf.h"
#include "memory.h"

//static char *Argv[5] = { "argtest", "Rex,", "my", "man!", NULL };

int perform_execve(struct PCB *to_be,char* filename[],char* argvx[])
{
  User_Base=to_be->Base;
  User_Limit=to_be->Limit;
  int endofheap=load_user_program((filename[0]));
  if(endofheap==-1)
  {
    DEBUG('e', "loading program fail\n");
    return -1;
  }
  to_be->sbrk=endofheap;

  int tos =User_Limit-8;
  int count2=0;

 while(1)  //counting numbers of argV
  {
      if(argvx[count2]==NULL)
      {
          break;
      }
      DEBUG('e', "argvx:%s\n",argvx[count2]);
      count2++;
  }
   int argc=count2;
   DEBUG('e', "argc!!!:%d\n",argc);
  int argv=0;
  int argvP[argc];
   // tos -= 3;
  for(int i=0;i!=argc;i++)
  {
      tos -= (strlen(argvx[argc-1-i])+1);
      argvP[argc-1-i]=tos;
      strcpy(main_memory+User_Base+tos, argvx[argc-1-i]);
  }
    while (tos % 4)
    {
        tos--;
    }
////////////////
    //tos -=4;/////////////////////////this line is just to match gradescope
///////////////
    tos -= 4;
    int k = 0;
    memcpy(main_memory+User_Base+tos, &k, 4);  ///null entry which signals end of argv
    for(int i=0;i<argc;i++)
    {
        tos -= 4;
        memcpy(main_memory+User_Base+tos, &argvP[argc-1-i], 4);
	      DEBUG('e', "&argv:%d\n", main_memory+User_Base+tos);
    }
    argv=tos;
//////////////////////////////sliding argv, argc, envp
    tos -= 4;
    k = 0;
    memcpy(main_memory+User_Base+tos, &k, 4);  ///&envp
    tos -= 4;
    memcpy(main_memory+User_Base+tos, &argv, 4);  ///&argv
     tos -= 4;
     memcpy(main_memory+User_Base+tos, &argc, 4); ///&argc
      DEBUG('e', "&argc:%d\n",&argc);
      DEBUG('e', "argc:%d\n",*(main_memory+User_Base+tos));
      //DEBUG('e', "argv:%s\n",*(char*)(main_memory+argv+8));
      //DEBUG('e', "argv:%d\n",*(argv));
      DEBUG('e', "&k:%d\n",&k);
      for(int i=0;i<argc;i++)
      {
        DEBUG('e', "&argvP:%d\n",&argvP[i]);
        DEBUG('e', "argv:%s\n",main_memory+argvP[i]);
      }
      //DEBUG('e', "argvx:%s\n",argvx[count2]);
     //main_memory[tos]=argc;
  to_be->registers[PCReg] =0;
  to_be->registers[NextPCReg] =4;
  to_be->registers[StackReg] =tos-12;
 DEBUG('e', "Userbase:%d\n",User_Base);
  return 1;
}


 struct PCB *initialize_user_process(char *filename[])
{
  int thisProcessStartAt=(int)jval_i(dll_val(dll_first(usedlist)));
  dll_delete_node(dll_first(usedlist));
  User_Base=thisProcessStartAt;
  User_Limit=MemorySize/8;
  bzero(main_memory, MemorySize);
  struct PCB *to_be=malloc(sizeof(struct PCB));
  for(int i=0;i<NumTotalRegs;i++)
  {
  to_be->registers[i]=0;
  }
  to_be->Base=User_Base;
  to_be->Limit=User_Limit;
  to_be->PID=get_new_pid();
  to_be->parent=sentinel_PCB;
  dll_append(sentinel_PCB->children,new_jval_v(to_be));
  DEBUG('e', "sentinel_PCB->children:%d\n",dll_size(sentinel_PCB->children));
  to_be->waiter_sem=make_kt_sem(0);
  to_be->waiters=new_dllist();
  to_be->children=new_dllist();
  initFd(to_be,fDstoragesize);
//////////////////////
/////for now, one program will start at 0.
  //////////////////////setting argv and argc
  // int count=0;
  // while(1)  //counting numbers of argV
  // {
  //     if(filename[count]==NULL)
  //     {
  //         break;
  //     }
  //     count++;
  // }
  // //DEBUG('e', "argcduring%d\n",&argvP[i]);
  // char * argvs[count];
  // for(int i=0;i<count;i++)
  // {
  //     argvs[i]=filename[i];
  // }
  perform_execve(to_be,filename,filename);
///////////////////////////finished with argv and argc
  dll_append(readyq,new_jval_v(to_be));
  noopF=0;
  kt_exit();
}

void initialzie_sentinel()
{
  sentinel_PCB=malloc(sizeof(struct PCB));
  sentinel_PCB->PID=get_new_pid(); ///0
  sentinel_PCB->children=new_dllist();
  sentinel_PCB->waiters=new_dllist();
  sentinel_PCB->waiter_sem=make_kt_sem(0);
}

void cleaningInit()
{
  while(1)
  {
    if(dll_size(sentinel_PCB->children)==0)
    {
      break;
    }
    P_kt_sem(sentinel_PCB->waiter_sem);
    struct PCB* temp=jval_v(dll_val(dll_first(sentinel_PCB->waiters)));
    dll_delete_node(dll_first(sentinel_PCB->waiters));

    return_pid(temp->PID); //returning the PID to PIDstorage
    int returnvalue=temp->exitvalue;
    free(temp);
  }
  SYSHalt();
}

void KOS()
{
  //User_Base=0;
  //User_Limit=MemorySize/8;
   usedlist=new_dllist();
   for(int i=0;i<8;i++)
   {
     dll_append(usedlist,new_jval_i((i)*MemorySize/8));
   }
   PIDstoragesize=100;
   initialize_PID(PIDstoragesize);
   fDstoragesize=100;
   writeOk=make_kt_sem(0);
   writer=make_kt_sem(1);
   reader=make_kt_sem(1);
   //current=malloc(sizeof(struct PCB));
   readyq=new_dllist();
   initialize_cBuf();
   initialzie_sentinel();
   ////////////////////////this PID below acts for the init
    ///////////////////////// Initializing global variables
   kt_fork((void *)initialize_user_process,kos_argv);
   //initialize_user_process("a.out");
   kt_fork((void *)&console_buf_read,(void *)NULL);
   kt_fork((void*)cleaningInit,(void*)NULL);
   kt_joinall();
   DEBUG('e', "before scheduler\n");
   run_scheduler();
   //kt_joinall();
   SYSHalt();
}
