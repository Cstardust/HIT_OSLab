#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

//   获取fileName:截取最后一个/后面的字符串存入buf并返回
char *fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;
    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ) // ? buf存不下就不存了,直接用path里的. 也没啥提示，这不是会影响用户程序？
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), 0, DIRSIZ - strlen(p));
    return buf;
}

/*
 * 不查找文件夹，只查找文件。
 * 即文件夹同名也不算入结果
 * 题没说清楚，我就这么写了。
 */

void find(char *path, const char *name)
{
    // printf("DEBUG === FIND ==== PATH = %s\n",path);
    //  当path为目录时起到作用。buf = 目录path+获取的filename
    char buf[512] = {0}, *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
        //  path为完整文件名，而非目录，故不需要再用buf拼接出完整fileName
        //  直接fmtname(path)即可获得单独的文件名称
    case T_FILE:
        // printf("fmtname(path) = %s\ttargetName = %s\tstrlenof(path)=%d\tstrlenof(name)=%d\tcomp=%d\n",fmtname(path),name,strlen(fmtname(path)),strlen(name),strcmp(fmtname(path), name));
        if (!strcmp(fmtname(path), name))
        {
            printf("%s\n", path);
        }
        break;
        //  path为目录
        //  遍历目录下的所有文件 对每个文件进行递归
        //  在下一层递归中，文件是目录，继续递归
        //  在下一层递归中，文件不是目录，不再递归，获取name，退出。
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }

        //  利用buf拼接fileName
        strcpy(buf, path);
        p = buf + strlen(buf); //  buf是path（此处path是目录）+文件名称
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            // printf("debug fd = %d: de.inum=%d de.name=%s \n",fd,de.inum,de.name);
            if (de.inum == 0 || (!strcmp(de.name, ".")) || (!strcmp(de.name, "..")))
                continue;
            //  拼接fileName
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            //  get a file then dfs
            find(buf, name);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage : find path filename!\n");
        exit(0);
    }
    find(argv[1], argv[2]);
    exit(0);
}