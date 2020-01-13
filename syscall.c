#include <stdlib.h>
#include <errno.h>
#include "kt.h"
#include "simulator.h"
#include "scheduler.h"
#include "kos.h"
#include "console_buf.h"
#include "syscall.h"
#include "memory.h"
void syscall_return(struct PCB* pcb, int returnVal) 
{
  pcb->registers[PCReg]=pcb->registers[NextPCReg];
  //pcb->registers[NextPCReg]=pcb->registers[PCReg]+4;
  pcb->registers[2]=returnVal;
  dll_append(readyq,new_jval_v(pcb));
  kt_exit();
}



void do_write(struct PCB *pcb)
{
  int r5 = pcb->registers[5];
	int r6 = pcb->registers[6];
	int r7 = pcb->registers[7];
  if(r5>100)
  {DEBUG('e', "EBADF write\n");
    syscall_return(pcb, -EBADF);}
  if(r6<0)
  {DEBUG('e', "EFAULT write\n");
    syscall_return(pcb, -EFAULT);}
  if(r6>pcb->Base+pcb->Limit)
  {DEBUG('e', "EFBIG write\n");
    syscall_return(pcb, -EFBIG );}
  if(r7<0)
  {DEBUG('e', "EINVAL write\n");
    syscall_return(pcb, -EINVAL);}
  if(r7>pcb->Base+pcb->Limit)
  {DEBUG('e', "EACCES write\n");
    syscall_return(pcb, -EACCES);}
    DEBUG('e', "write:%d\n",kt_getval(writer));
  User_Base=pcb->Base;
  DEBUG('e', "this process'sbrk:%d\n",pcb->sbrk);
  DEBUG('e', "this process'limit:%d\n",pcb->Limit);
  DEBUG('e', "this process'Base:%d\n",pcb->Base);
  DEBUG('e', "r7:%d\n",r7);
  int i;
  if(r5==1||r5==2)
  {
    P_kt_sem(writer);
    for(i=0;i<r7;i++)
    {
      console_write(main_memory[pcb->Base+r6+i]);
      DEBUG('e', "writerOk:%d\n",kt_getval(writeOk));
      P_kt_sem(writeOk);
      User_Base=pcb->Base;
      DEBUG('e', "this process goes through writeOk:%d\n",pcb->PID);
    }
    V_kt_sem(writer);
    //////////////returning from sys_call
    syscall_return(pcb,i);
  }
  if(r5>=3)
  {
    struct fd* Fd=LookupFDTable(pcb,r5);
    if(Fd==0)
    {
      //V_kt_sem(writer);
      syscall_return(pcb,-EBADF);
    }
      if(Fd->permit!=0)
    {
      syscall_return(pcb,-EBADF);
    }
    P_kt_sem(Fd->bufferWriter);
      if(*(Fd->read)==0)
      {
       V_kt_sem(Fd->bufferWriter);
        syscall_return(pcb,-EBADF);
      }
     for(i=0;i<r7;i++)
    {
      *(*(Fd->offset))=main_memory[pcb->Base+r6+i];
      *(Fd->offset)+=1;
      if(*(Fd->offset)-Fd->buffer>buffersize)
      {
        *(Fd->offset)=Fd->buffer;
      }
      V_kt_sem(Fd->readOK);
      P_kt_sem(Fd->slots); //slots=256
      User_Base=pcb->Base;
      if(*(Fd->read)==0)
      {
       V_kt_sem(Fd->bufferWriter);
        syscall_return(pcb,-EBADF);
      }
    }
    //V_kt_sem(writer);
    //////////////returning from sys_call
    V_kt_sem(Fd->bufferWriter);
    syscall_return(pcb,i);
  }
}


void do_read(struct PCB *pcb)
{
  int r5 = pcb->registers[5];
	int r6 = pcb->registers[6];
	int r7 = pcb->registers[7];
  if(r5>100) 
  {syscall_return(pcb, -EBADF);}
  if(r6<0)
  {syscall_return(pcb, -EFAULT);}
  if(r6>MemorySize-24)
  {syscall_return(pcb, -EFBIG);}
  if(r7<=0)
  {syscall_return(pcb, -EINVAL);}
  if(r7>MemorySize-24)
  {syscall_return(pcb, -EACCES);}
  P_kt_sem(reader);
  User_Base=pcb->Base;
  if(r5==0)
  {
    int k=0;
    while(1)
      {
        P_kt_sem(cBuffer->nelem);
        User_Base=pcb->Base;
        if(cBuffer->head==256)
        {
          cBuffer->head=0;
        }
        if(cBuffer->buffer[cBuffer->head]==EOF)
        {
          break;
        }
        main_memory[pcb->Base+r6+k]=cBuffer->buffer[cBuffer->head];
        //  printf("head:%d\n",cBuffer->head);
        cBuffer->head+=1;
        k+=1;
        V_kt_sem(cBuffer->nslots);
        if(k>=r7)
        {
          break;
        }
      }
    cBuffer->written=0;
    V_kt_sem(reader);
    //cBuffer->now=0;
    //////////////returning from sys_call
    syscall_return(pcb, k);
  }
  if(r5>=3)
  {
    struct fd* Fd=LookupFDTable(pcb,r5);
    if(Fd==0)
    {
      V_kt_sem(reader);
      syscall_return(pcb,-EBADF);
    }
    if(Fd->permit!=1)
    {
       V_kt_sem(reader);
      syscall_return(pcb,-EBADF);
    }
    int i;
    for(i=0;i<r7;i++)
    {
      if(*(Fd->write)==0)
      {
        V_kt_sem(reader);
        syscall_return(pcb,0);
      }
      P_kt_sem(Fd->readOK);
      if(*(Fd->write)==0)
      {
        V_kt_sem(reader);
        syscall_return(pcb,0);
      }
      main_memory[pcb->Base+r6+i]=*(*(Fd->offset));
      // if(*(*(Fd->offset))==EOF)
      // {
      //   *(Fd->offset)+=1;
      //   V_kt_sem(Fd->slots);
      //   i+=1;
      //   break;
      // }
      *(Fd->offset)+=1;
      if(*(Fd->offset)-Fd->buffer>buffersize)
      {
        *(Fd->offset)=Fd->buffer;
      }
      V_kt_sem(Fd->slots);
      if(kt_getval(Fd->slots)==buffersize)
      {
        i+=1;
        break;
      }
    }
    V_kt_sem(reader);
    syscall_return(pcb,i);
  }
}


void handle_iotcl(struct PCB *pcb)
{
  int r5 =pcb->registers[5];
  int r6 =pcb->registers[6];
  int r7 =pcb->registers[7];
  if(r5!=1||r6!=JOS_TCGETP)
  {
    syscall_return(pcb, -EINVAL);
  }
  ioctl_console_fill((struct JOStermios *)(main_memory+pcb->Base+r7)); //getting the pointer stored at r7 in main memory
  syscall_return(pcb, 0);
}

void handle_fstat(struct PCB *pcb)
{
  int blk_size=0;
  int r5 =pcb->registers[5];
  int r6 =pcb->registers[6];
  if(r5==0)
  {
    blk_size=1;
  }
  if(r5==1||r5==2)
  {
    blk_size=256;
  }
  
  if(r5>2||r5<0)
  {
    syscall_return(pcb, -EBADF);
  }
  if(*(int*)(r6+pcb->Base+main_memory)==0)
  {
    syscall_return(pcb, -EFAULT);
  }
  if(ValidateAddress(blk_size,pcb))
  {
    stat_buf_fill((struct KOSstat *)(r6+pcb->Base+main_memory),blk_size);
    syscall_return(pcb, 0);
  }
  else
  {
    syscall_return(pcb, -EFAULT);
  }
}

void handle_pagesize(struct PCB *pcb)
{
  syscall_return(pcb, PageSize);
}


void handle_sbrk(struct PCB *pcb)
{
  int r5 =pcb->registers[5];
  int now=pcb->sbrk; //the address of the currently running process
  DEBUG('e', "PID:%d\n",pcb->PID);
  DEBUG('e', "sbrk:%d\n",pcb->sbrk);
  DEBUG('e', "r5:%d\n",r5);
  if(ValidateAddress(r5,pcb))
  {
    pcb->sbrk+=r5;    //the end of the newly allocated space
  }
  else
  {
    syscall_return(pcb, -ENOMEM);
  }
  //DEBUG('e', "AfterCurrentsbrk:%d\n",CurrentPCB->sbrk);
  syscall_return(pcb, now);
}

void handle_execve(struct PCB *pcb)
{
  int r5=pcb->registers[5];
  int r6=pcb->registers[6];
  char *path=main_memory+pcb->Base+r5;
 // char *argument;
  int* nextP=main_memory+pcb->Base+r6;
  int count=0;
  while(nextP[count]!=0)
  {
    count+=1;
  }
  char **temp2=(char**)malloc((count+1)*sizeof(char*));
  for(int i=0;i<count;i++)
  {
    temp2[i]=(char*)malloc(strlen(main_memory+pcb->Base+nextP[i])+1);
    strcpy(temp2[i],(char*)main_memory+pcb->Base+nextP[i]);
  }
  temp2[count]=NULL;
  char *filename[1];
  char *pathInKOS=malloc(strlen(path));
  filename[0]=pathInKOS;
  strcpy(pathInKOS,path);
  if(perform_execve(pcb,filename,temp2)==-1)
  {
    syscall_return(pcb, -EFAULT);
  }
   for(int i=0;i<=count;i++)
   {
     free(temp2[i]);
   }
  free(temp2);
  free(pathInKOS);
  pcb->registers[2]=0;
  dll_append(readyq,new_jval_v(pcb));
  kt_exit();
}


void handle_getpid(struct PCB *pcb)
{
  syscall_return(pcb,pcb->PID);
}

void handle_fork(struct PCB *pcb)
{
  if(dll_empty(usedlist))
  {
    syscall_return(pcb,-EAGAIN);
  }
  struct PCB* child=malloc(sizeof(struct PCB));
  dll_append(pcb->children,new_jval_v(child));
  int thisProcessStartAt=(int)jval_i(dll_val(dll_first(usedlist))); ////searching for usable segment of mem
  dll_delete_node(dll_first(usedlist)); //delete that mem from list
  child->Limit=User_Limit;  //setting child's limit
  child->Base=thisProcessStartAt;  //setting child's base mem
  child->waiter_sem=make_kt_sem(0); //setting waiter_sem
  child->waiters=new_dllist(); //setting the waiterlist of this process
  child->children=new_dllist(); //setting the children list of this process
  child->sbrk=pcb->sbrk; //setting end of heap pointer
  for(int i=0;i<NumTotalRegs;i++)
  {
    child->registers[i]=pcb->registers[i];  //copying child' registers from parent
  }
  child->PID=get_new_pid();  //assign new PID
  child->parent=pcb;      //assign parent PID
  memcpy(main_memory+child->Base,main_memory+pcb->Base,User_Limit); //memcopy from parent to child
  copytable(pcb,child);
  copystorage(pcb,child);
  kt_fork((void *)finish_fork,(void *)child);
  syscall_return(pcb,child->PID);
}

void finish_fork(struct PCB *child)
{
  //kt_yield();
  syscall_return(child,0);
}

void handle_exit(struct PCB *pcb)
{
  dll_append(usedlist,new_jval_i(pcb->Base));
  ///returning the user memory to the memorylist, not
  ///returning PID however.
  pcb->exitvalue=0;


  Dllist to_delete=dll_first(pcb->fDtable);
  Dllist to_delete2=to_delete;
  struct fd* to_delete_fd;
  int sizeFD=dll_size(pcb->fDtable);
  for(int i=0;i<sizeFD;i++)
  {
    to_delete=dll_next(to_delete);
    to_delete_fd=jval_v(dll_val(to_delete2));

///////////////////////////////////////deleting items in a fd and dcrement items
    *to_delete_fd->openTime-=1;  //decrease opentime
    if(to_delete_fd->permit==0)
    {
      *to_delete_fd->write-=1;
      if(*to_delete_fd->write==0)
      {
        V_kt_sem(to_delete_fd->readOK);
      }
    }
    else if(to_delete_fd->permit==1)
    {
      *to_delete_fd->read-=1;
      if( *to_delete_fd->read==0)
      {
        V_kt_sem(to_delete_fd->slots);
      }
    }
    if(*(to_delete_fd->openTime)==0)
  {
      free(to_delete_fd->openTime);
      free(to_delete_fd->read);
      free(to_delete_fd->write);
      bzero(to_delete_fd->buffer,buffersize); //if no more fd associated with this buffer, free
      free(to_delete_fd->slots);
      free(to_delete_fd->readOK);
      free(to_delete_fd->bufferWriter);
  }

    dll_delete_node(to_delete2);
    to_delete2=to_delete;
  }
  free_dllist(pcb->fDstorage);


  V_kt_sem(pcb->parent->waiter_sem);
  dll_append(pcb->parent->waiters,new_jval_v(pcb));
  ///////////////////////this segment switch parentage for all orphans on children list
  Dllist to_switch_parentage=dll_first(pcb->children);
  struct PCB* to_switch;
  int size=dll_size(pcb->children);
  while(size>0)
  {
    to_switch=(struct PCB *)jval_v(dll_val(to_switch_parentage));
    to_switch->parent=sentinel_PCB;
    dll_append(sentinel_PCB->children,new_jval_v(to_switch));
    to_switch_parentage=dll_next(to_switch_parentage);
    dll_delete_node(dll_first(pcb->children));
    size-=1;
  }
  /////////////////////////this segment switches parentage for zombies on this process
  Dllist to_switch_parentage_Z=dll_first(pcb->waiters);
  struct PCB* to_switch_Z;
  int size2=dll_size(pcb->waiters);
  while(size2>0)
  {
    to_switch_Z=(struct PCB *)jval_v(dll_val(to_switch_parentage_Z));
    dll_append(sentinel_PCB->waiters,new_jval_v(to_switch_Z));
    to_switch_parentage_Z=dll_next(to_switch_parentage_Z);
    dll_delete_node(dll_first(pcb->waiters));
    V_kt_sem(sentinel_PCB->waiter_sem);
    size2-=1;
  }
  ////////////////////this segment is to be tested, which pops pcb from its parent's child list
  struct PCB* to_remove=(struct PCB *)jval_v(dll_val(dll_first(pcb->parent->children)));
  Dllist to_remove_2=dll_first(pcb->parent->children);
  int size3=dll_size(pcb->parent->children);
  //DEBUG('e', "pcb->parent->children:%d",size2);
  while(size3>0)
  {
    if(to_remove==pcb)
    {
      break;
    }
    to_remove_2=dll_next(to_remove_2);
    to_remove=(struct PCB *)jval_v(dll_val(to_remove_2));
    size3-=1;
  }
  if(to_remove_2==NULL)
  {
    DEBUG('e', "the child doesn't exist in parent's children list:%d",pcb->PID);
    kt_exit();
  }
  if(to_remove_2!=NULL)
  {
    dll_delete_node(to_remove_2);
  }
  //////////////////
  bzero(main_memory+pcb->Base, pcb->Limit);
  kt_exit();
}

void handle_tablesize(struct PCB *pcb)
{
  syscall_return(pcb,64);
}

void handle_close(struct PCB *pcb)
{

  int r5=pcb->registers[5];
  struct fd* lookupFd=LookupFDTable(pcb,r5);  //find the fd assiciated with this num
  if(lookupFd==0)
  {
    syscall_return(pcb,-EBADF);
  }
  returnFdnum(pcb,lookupFd->fdnum);   //return this num
  *lookupFd->openTime-=1;  //decrease opentime
  if(lookupFd->permit==0)
  {
    *lookupFd->write-=1;
    if(*lookupFd->write==0)
    {
      V_kt_sem(lookupFd->readOK);
    }
  }
  else if(lookupFd->permit==1)
  {
    *lookupFd->read-=1;
    if( *lookupFd->read==0)
    {
      V_kt_sem(lookupFd->slots);
    }
  }
  if(*(lookupFd->openTime)==0)
  {
    free(lookupFd->openTime);
      free(lookupFd->read);
      free(lookupFd->write);
      bzero(lookupFd->buffer,buffersize);  //if no more fd associated with this buffer, free
      free(lookupFd->slots);
      free(lookupFd->readOK);
      free(lookupFd->bufferWriter);
  }
  deleteFDTable(pcb,lookupFd->fdnum);
  free(lookupFd); //free fd
  syscall_return(pcb,0);
}

void handle_getppid(struct PCB *pcb)
{
  if(pcb->parent->PID<0)
  {
    syscall_return(pcb,-EFAULT);
  }
   syscall_return(pcb,pcb->parent->PID);
}

void handle_wait(struct PCB *pcb)
{
  P_kt_sem(pcb->waiter_sem);
  User_Base=pcb->Base;
  struct PCB* temp=jval_v(dll_val(dll_first(pcb->waiters)));
  DEBUG('e',"waiters:%d",temp->PID);
  dll_delete_node(dll_first(pcb->waiters));
  return_pid(temp->PID); //returning the PID to PIDstorage
  int returnvalue=temp->PID;
  free(temp);
  syscall_return(pcb,returnvalue);
}

void handle_dup(struct PCB *pcb)
{
  int r5=pcb->registers[5]; //the file descriptor to be copied.
  struct fd* lookupFd=LookupFDTable(pcb,r5);
  if(lookupFd==0)
  {
    syscall_return(pcb,-EBADF);
  }
  struct fd* newfd=createFdDup(pcb,lookupFd->buffer,lookupFd->offset,lookupFd->slots,lookupFd->readOK,lookupFd->bufferWriter,lookupFd->openTime,lookupFd->permit,lookupFd->read,lookupFd->write);
  syscall_return(pcb,newfd->fdnum);
}

void handle_dup2(struct PCB *pcb)
{
  int r5=pcb->registers[5];
  int r6=pcb->registers[6];
  if(r5==r6)
  {
    syscall_return(pcb,r5);
  }
  struct fd* oldFd=LookupFDTable(pcb,r5);
  struct fd* newFd=LookupFDTable(pcb,r6);
  if(oldFd==0)
  {
    syscall_return(pcb,-EBADF);
  }
  if(newFd!=0) //if newfd is already used, silently close it
  {
    *(newFd->openTime)-=1;  //decrease opentime
    if(newFd->permit==0)
    {
      *newFd->write-=1;
    }
    else if(newFd->permit==1)
    {
      *newFd->read-=1;
    }
    if(*(newFd->openTime)<=0)
    {
      free(newFd->openTime);
      free(newFd->read);
      free(newFd->write);
      free(newFd->buffer);  //if no more fd associated with this buffer, free
      free(newFd->slots);
      free(newFd->readOK);
      free(newFd->bufferWriter);
    }
    deleteFDTable(pcb,newFd->fdnum);
    free(newFd); //free fd
  }
  struct fd* createdFd=createFdDup2(pcb,oldFd->buffer,oldFd->offset,oldFd->slots,oldFd->readOK,oldFd->bufferWriter,oldFd->openTime,oldFd->permit,oldFd->read,oldFd->write);
  createdFd->fdnum=r6;
  syscall_return(pcb,createdFd->fdnum);
}

void handle_pipe(struct PCB *pcb)
{
  int r5=pcb->registers[5];
  char* buffer=malloc(buffersize);
  kt_sem* sem=malloc(sizeof(kt_sem));
  sem = make_kt_sem(buffersize);
  kt_sem* readOk=malloc(sizeof(kt_sem));
  readOk = make_kt_sem(0);
  kt_sem* bufferWriter=malloc(sizeof(kt_sem));
  bufferWriter = make_kt_sem(1);
  int *opened=malloc(4);
  int *read=malloc(4);
  int *write=malloc(4);
  *opened=0;
  *read=0;
  *write=0;
  char** offset=malloc(4);
  char** offset2=malloc(4);
  *offset=buffer;
  *offset2=buffer;
  struct fd* createdFd=createFdDup(pcb,buffer,offset,sem,readOk,bufferWriter,opened,1,read,write);
  struct fd* createdFd2=createFdDup(pcb,buffer,offset2,sem,readOk,bufferWriter,opened,0,read,write);
  int a=createdFd->fdnum;
  int b=createdFd2->fdnum;
  memcpy(main_memory+pcb->Base+r5,&a,4);
  memcpy(main_memory+pcb->Base+r5+4,&b,4);
  syscall_return(pcb,0);
}
