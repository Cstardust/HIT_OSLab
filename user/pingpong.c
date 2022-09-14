#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc,char *argv[])
{
    int pipefd_a[2];
    int pipefd_b[2];
    if(pipe(pipefd_a) == -1 || pipe(pipefd_b) == -1)
    {
        printf("failed to pipe!\n");
        exit(1);
    }

    pid_t pid = fork();
    if(pid == -1){
        printf("failed to fork!\n");
        exit(2);
    }

    char buf[512]={0}; //  cow buf
    if(pid == 0)    //  child
    {
        close(pipefd_a[1]);     //  close unused write a
        close(pipefd_b[0]);     //  close unused read b
      
        read(pipefd_a[0],buf,sizeof buf);
        
        
        printf("%d: received %s\n",getpid(),buf);
        write(pipefd_b[1],"pong",4);

        close(pipefd_a[0]);
        close(pipefd_b[1]);
    }   
    else            //  parent 
    {
        close(pipefd_a[0]);     //  close unused read a
        close(pipefd_b[1]);     //  close unused write b
      
        write(pipefd_a[1],"ping",4);
        read(pipefd_b[0],buf,sizeof buf);
        printf("%d: received %s\n",getpid(),buf);
    
        close(pipefd_a[1]);
        close(pipefd_b[0]);

        wait(nullptr);
    }
    exit(0);
}