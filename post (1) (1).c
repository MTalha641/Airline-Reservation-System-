#include<linux/kernel.h>
asmlinkage long sys_sem_post(int S){
S++;
return 0;
}
