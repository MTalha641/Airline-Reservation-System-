#include<linux/kernel.h>
asmlinkage long sys_sem_wait(int S){
while(S<=0);
S--;
return 0;
}
