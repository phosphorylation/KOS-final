#include "kt.h"
#include "dllist.h"
kt_sem writeOk;
kt_sem writer;
kt_sem reader;
struct PCB* CurrentPCB;
int User_Base1;
int  User_Base2;
int  User_Base3;
int  User_Base4;
int  User_Base5;
int  User_Base6;
int  User_Base7;
int  User_Base8;
Dllist usedlist;
struct PCB *sentinel_PCB;
int perform_execve(struct PCB *to_be,char* filename[],char* argvx[]);
struct PCB *initialize_user_process(char *filename[]);