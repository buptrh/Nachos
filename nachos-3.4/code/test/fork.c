#include "syscall.h"

int global_cnt=0;

void sum(){
  int i;
  
  for (i=0;i<100;i++)
    {
      global_cnt++;
    }	
  Exit(global_cnt);
}

int main()
{
  global_cnt++;
  
  Fork(sum);
  Yield();
 
  global_cnt++;

  Fork(sum);
  Yield();
  
  global_cnt++;

  Fork(sum);
  Yield();
  
  global_cnt++;
  
  Exit(global_cnt); 
}
