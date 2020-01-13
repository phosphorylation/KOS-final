#include "dllist.h"
#include "simulator.h"
#include "scheduler.h"
#include "kt.h"

 void run_scheduler()
{
  if(dll_empty(readyq))
    {
    DEBUG('e', "scheduler doing nothing\n");
    noopF=1;
      noop();
    }
    else
    {
     noopF=0;
     DEBUG('e', "in readyq\n");
     struct PCB* temp=jval_v(dll_val(dll_first(readyq)));
     CurrentPCB=temp; /////copying the running PCB into the global variable
     DEBUG('e', " readyqSize:%d\n",dll_size(readyq));
     dll_delete_node(dll_first(readyq));
     User_Base=CurrentPCB->Base;
     DEBUG('e', " User_Base:%d\n",User_Base);
     DEBUG('e', " myPID:%d\n",CurrentPCB->PID);
     DEBUG('e', " sbrk:%d\n",CurrentPCB->sbrk);
     User_Limit=CurrentPCB->Limit;
     run_user_code(CurrentPCB->registers);
    }
}

void initialize_PID(int PIDstoragesize)
{
  PIDstorage=new_dllist();
  for(int i=0;i<PIDstoragesize;i++)
  {
    dll_append(PIDstorage,new_jval_i(i));
  }
}

int get_new_pid()
{
  if(!dll_empty(PIDstorage))
  {
    int i=(int)jval_i(dll_val(dll_first(PIDstorage)));
    //DEBUG('e', " PIDstorage:%d\n",dll_size(PIDstorage));
    Dllist to_return=dll_first(PIDstorage);
    Dllist to_delete=to_return;
    for(int j=0;j<dll_size(PIDstorage);j++)
    {
      //DEBUG('e', " getnewpid:%d\n",(int)jval_i(dll_val(to_return)));
      if((int)jval_i(dll_val(to_return))<i)
      {
        to_delete=to_return;
        i=(int)jval_i(dll_val(to_return));
      }
      to_return=dll_next(to_return);
    }
    dll_delete_node(to_delete);
    return i;
  }
  else
  {
    return -1;
  }
}

void return_pid(int PID)
{
  dll_append(PIDstorage,new_jval_i(PID));
}