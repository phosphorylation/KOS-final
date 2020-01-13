#include "scheduler.h"
#include "memory.h"
#include <stdlib.h>
void initFd(struct PCB* pcb,int fDstoragesize) //initialize the storage
{
  buffersize=4*1024;
  pcb->fDstorage=new_dllist();
  for(int i=3;i<fDstoragesize;i++)
  {
    dll_append(pcb->fDstorage,new_jval_i(i));
  }
  pcb->fDtable=new_dllist();
}

int getFdnum(struct PCB* pcb)  //assign a fD number from list
{
    if(!dll_empty(pcb->fDstorage))
  {
    int i=(int)jval_i(dll_val(dll_first(pcb->fDstorage)));
    DEBUG('e', " PIDstorage:%d\n",dll_size(pcb->fDstorage));
    Dllist to_return=dll_first(pcb->fDstorage);
    Dllist to_delete=to_return;
    for(int j=0;j<dll_size(pcb->fDstorage);j++)
    {
      DEBUG('e', " getnewpid:%d\n",(int)jval_i(dll_val(to_return)));
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

void returnFdnum(struct PCB* pcb,int fdnum) //return a fD number to storage
{
  dll_append(pcb->fDstorage,new_jval_i(fdnum));
}

struct fd* createFdDup(struct PCB* pcb,char *buffer,char **offset,kt_sem* nslots,kt_sem* readOK,kt_sem* bufferWriter, int* openTime,int Row,int* read,int* write) //create a new fd off a buffer and a offset.
{
  struct fd* Afd=malloc(sizeof(struct fd));
  Afd->fdnum=getFdnum(pcb);
  Afd->slots=nslots;
  Afd->readOK=readOK;
  Afd->bufferWriter=bufferWriter;
  Afd->permit=Row;
  if(Row==0)
  {
    *(write)+=1;
  }
  else if(Row==1)
  {
    *(read)+=1;
  }
  Afd->read=read;
  Afd->write=write;
  (*openTime)+=1;
  Afd->openTime=openTime;
  Afd->buffer=buffer;
  Afd->offset=offset;
  dll_append(pcb->fDtable,new_jval_v(Afd));
  return Afd;
}

struct fd* createFdDup2(struct PCB* pcb,char *buffer,char **offset,kt_sem* nslots,kt_sem* readOK,kt_sem* bufferWriter, int* openTime,int Row,int* read,int* write) //create a new fd off a buffer and a offset.
{
  struct fd* Afd=malloc(sizeof(struct fd));
  Afd->fdnum=-1;
  Afd->slots=nslots;
  Afd->readOK=readOK;
  Afd->bufferWriter=bufferWriter;
  Afd->permit=Row;
  if(Row==0)
  {
    *(write)+=1;
  }
  else if(Row==1)
  {
    *(read)+=1;
  }
  Afd->read=read;
  Afd->write=write;
  (*openTime)+=1;
  Afd->openTime=openTime;
  Afd->buffer=buffer;
  Afd->offset=offset;
  dll_append(pcb->fDtable,new_jval_v(Afd));
  return Afd;
}

struct fd* LookupFDTable(struct PCB* pcb,int fdnum)
{
  if(dll_size(pcb->fDtable)<=0)
  {
    return(0);
  }
  Dllist to_return=dll_first(pcb->fDtable);
  struct fd* Afd;
  for(int i=0;i<dll_size(pcb->fDtable);i++)
  {
    Afd=jval_v(dll_val(to_return));
    DEBUG('e', "fdnum: %d\n",Afd->fdnum);
    if(Afd->fdnum==fdnum)
    {
      return Afd;
    }
    to_return=dll_next(to_return);
  }
  return(0);
}

void deleteFDTable(struct PCB* pcb,int fdnum)
{
  Dllist to_delete=dll_first(pcb->fDtable);
  struct fd* Afd;
  for(int i=0;i<dll_size(pcb->fDtable);i++)
  {
    Afd=jval_v(dll_val(to_delete));
    if(Afd->fdnum==fdnum)
    {
      dll_delete_node(to_delete);
      break;
    }
    to_delete=dll_next(to_delete);
  }
}

void copytable(struct PCB* pcb,struct PCB* child)
{
  child->fDtable=new_dllist();
  child->fDstorage=new_dllist();
  Dllist eachTable=dll_first(pcb->fDtable);
  int a[fDstoragesize];
  for(int i=3;i<fDstoragesize;i++)
  {
    a[i-3]=i;
  }
  int j=0;
  struct fd* Afd;
  for(int i=0;i<dll_size(pcb->fDtable);i++) //find each table of pcb, then create a corresponding one in child
  {
    Afd=jval_v(dll_val(eachTable));
    DEBUG('e', "copyfdnum: %d\n",Afd->fdnum);
    struct fd* newfd=malloc(sizeof(struct fd));
    newfd->fdnum=Afd->fdnum;
    a[Afd->fdnum-3]=-1;
    j++;
    newfd->permit=Afd->permit;
    newfd->openTime=Afd->openTime;
    *newfd->openTime+=1;
    newfd->read=Afd->read;
    newfd->write=Afd->write;
    if(newfd->permit==0)
    {
      *newfd->write+=1;
    }
    else if(newfd->permit==1)
    {
      *newfd->read+=1;
    }
    newfd->slots=Afd->slots;
    newfd->readOK=Afd->readOK;
    newfd->bufferWriter=Afd->bufferWriter;
    newfd->buffer=Afd->buffer;
    newfd->offset=Afd->offset;
    dll_append(child->fDtable,new_jval_v(newfd));
    eachTable=dll_next(eachTable);
  }
  for(int i=0; i<fDstoragesize;i++)
  {
    if(a[i]!=-1)
    {
      dll_append(child->fDstorage,new_jval_i(a[i]));
    }
  }
}

void copystorage(struct PCB* pcb,struct PCB* child)
{

}

int ValidateAddress(int r5,struct PCB * pcb)
{
  if(pcb->sbrk+r5>=pcb->Base+pcb->Limit)
  {
    return 0;
  }
  else if(pcb->Base+pcb->sbrk+r5<pcb->Base)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}