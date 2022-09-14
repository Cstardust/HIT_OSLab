/*
 * 1. dfs/迭代
 * 2. 埃氏筛法：用遍历到的质数去筛除合数
 */
#include "kernel/types.h"
#include "user/user.h"

#define N 35

const int my_eof = -1;

void Pipe(int *pipefd);
pid_t Fork();

//  函数功能：parent进程接收左侧输入管道，并创建child，与该child建立输出管道，将左侧输入的数字筛选之后输出给child。
void primeFilter(int inputPipe[2])
{
    //  read the prime
    int primes = 0;
    read(inputPipe[0],&primes,sizeof(int));
    
    //  the last child exit
    if(primes == my_eof)
    {
        // printf("===DEBUG==the last child exit====\n");
        exit(0);        
    }
    
    //  由埃氏筛法可知，当前遍历到的第一个数字一定是质数。
    printf("prime %d\n",primes);  

    //  本进程以及子进程不会写输入管道
    close(inputPipe[1]);

    //  create output pipe
    int outputPipe[2];
    Pipe(outputPipe);

    pid_t pid = Fork();
    if(pid == 0)                    //  child
    {
        close(inputPipe[0]);        //  parent的input对child无意义
        primeFilter(outputPipe);    //  parent的output就是child的input
        // exit(0);                 //  never reach
    }
    else                            //  parent
    {
        int num = 0;
        while(read(inputPipe[0],&num,sizeof(int))>0 && num!=-1)
        {
            if(num % primes == 0) continue;
            write(outputPipe[1],&num,sizeof(int));
        }
        write(outputPipe[1],&my_eof,sizeof(int));

        close(inputPipe[0]);        //  finish read
        // close(outputPipe[1]);    //  can not finish write 我们不能假定子进程和父进程谁先运行
        wait(nullptr);                 //  wait for child
        
        close(outputPipe[1]);       //  finish write
        exit(0);                    //  exit
    }

}

int main()
{
    int pipefd[2];
    Pipe(pipefd);

    pid_t pid = Fork();
    if (pid == 0)   //  child
    {
        primeFilter(pipefd);    //  dfs
    }
    else            //  parent  不可进入递归函数 因为左侧没有管道
    {
        close(pipefd[0]);       //  parent not read
        printf("prime 2\n");
        for (int i = 2; i <= 35; ++i)
        {
            if(i%2==0) continue;
            write(pipefd[1], &i, sizeof(int));
        }

        write(pipefd[1],&my_eof,sizeof(int));
        // close(pipefd[1]);       //  not finish write 我们不能假定子进程和父进程谁先运行
        wait(nullptr);
        close(pipefd[1]);           //  finish write
    }

    exit(0);
}

void Pipe(int *pipefd)
{
    if (pipe(pipefd) == -1)
    {
        printf("pipefd error\n");
        exit(1);
    }
}


pid_t Fork()
{
    pid_t pid = fork();
    if(pid == -1)
    {
        printf("fork error!\n");
        exit(1);
    }
    return pid;
}
