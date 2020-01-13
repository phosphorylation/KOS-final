#include "kt.h"
#include "simulator.h"
#include "console_buf.h"
#include <stdlib.h>
void initialize_cBuf(){
  cBuffer=(struct console_buf *)malloc(sizeof(struct console_buf));
  cBuffer->now=0;
  cBuffer->head=0;
  cBuffer->written=0;
  //cBuufer->timesOfInput;
  for(int i=0;i<256;i++)
  {
    cBuffer->buffer[i]=0;
  }
  cBuffer->nelem=make_kt_sem(0);
  cBuffer->nslots=make_kt_sem(256);
  cBuffer->consoleWait=make_kt_sem(0);
  }
  
  void console_buf_read(){
  while(1)
  {
     DEBUG('e', "went before\n");
    P_kt_sem(cBuffer->consoleWait); //wait until a signal is sent from console_readInt()
     DEBUG('e', "went over\n");
    P_kt_sem(cBuffer->nslots);  //make sure there are still space to read
    cBuffer->buffer[cBuffer->now]=(int)console_read();
    cBuffer->now++; 
      cBuffer->written++;
    if(cBuffer->now>=256)
      {
        cBuffer->now=0;
      }
      V_kt_sem(cBuffer->nelem);
  }
    kt_exit();
  }