#include"kernel/types.h"
#include"user/user.h"
#include"kernel/param.h"

pid_t Fork();


int main(int argc,char *argv[])
{
    if(argc==1)
    {
        printf("Usage : xargs command\n");
        exit(0);
    }    

    // for(int i=0;i<argc;++i)
    // {
    //     printf("argv[%d] = %s\n",i,argv[i]);
    // }

    //  pass to the execve program
    char *newargv[MAXARG];
    //  newargv[0] : echo or some command
    newargv[0] = argv[1];
    //  newargv[1..] : other arguments for 0 command
    int idx = 1;
    while(argv[idx+1]!=nullptr)
    {
        newargv[idx] = argv[idx+1];
        ++idx;
    }
    int backup = idx;    //  where should we rollback to each time

    while(1)
    {
        idx = backup;    //  reload backup 
        //  buf里的每一个空格分割开的参数，都占用一个newargv位
        char buf[512]={0};  
        int len = read(STDIN_FILENO,buf,512);
        // printf("len = %d strlen(buf) = %d last = %d\n",len,strlen(buf),buf[len-1]);
        if(len == 0)
        {
            printf("\n");
            // exit(0);                    //  parent break循环
            break;
        }
        else
        {
            //  child负责execve
            //  parent继续向下走 读取下一轮command
            pid_t pid = Fork();
            if(pid==0)                 //  child
            {
                //  文件末尾
                buf[len-1] = 0;   //  the end of string \n -> nullptr
                char *cur = buf;
                char *last_idx = buf;
                while((*cur)!=0)
                {
                    if(*cur == ' ' || *cur == '\n') //  猜测：因为文件可能有多行 所以可能有多个'\n'? 
                    {
                        *cur = 0;
                        newargv[idx++] = last_idx;
                        last_idx = cur+1;
                    }
                    ++cur;                    
                }

                newargv[idx++] = last_idx;
                newargv[idx++] = 0;
                // for(int i=0;i<idx;++i)
                // {
                //     printf("newargv[%d] = %s\n",i,newargv[i]);
                // }
                exec(argv[1],newargv);      //  退出child
                printf("never reach!\n");
            }
            // else                        //  parent wait for child
            // {
                // wait(nullptr);          
                //  一个子进程结束之后立刻回收
                //  缺点：
                    //  回收了第一个子进程才会回收第二个子进程
                    //  如果第一个子进程的任务执行时间很长，那么第二个子进程的命令就不会被读入，直到这个wait结束。
            // }
        }
    }

    //  当所有子进程都结束时
    //  parent wait for all the child
    	//  脑子抽筋的想法想法：sleep(1);  防止child还没execve，parent就wait
    	//  没必要，因此子进程没execve也是在执行，parent的wait不会=-1
    	//  因此 回收子进程方法：fork之后parent循环阻塞等待即可。
    //  回收所有子进程
    while(wait(nullptr)!=-1);
    
    exit(0);
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
