#include <unistd.h>
#include <system/fs.h>
#include <stdio.h>
#include <string.h>

int main()
{
    struct s_stat s;
    char type[4];
    stat("/HELLO1", &s);
    switch (s.mode) {
    case FILE_MODE_DIR:
        strcpy(type, "dr-x");
        break;
    case FILE_MODE_BLK:
        strcpy(type, "br--");
        break;
    case FILE_MODE_CHR:
        strcpy(type, "cr--");
        break;
    case FILE_MODE_REG:
        strcpy(type, "-r--");
        break;
    }
    printf("%s %8ld %s\n", type, s.size, "HELLO1");

    stat("/a.txt", &s);
    switch (s.mode) {
    case FILE_MODE_DIR:
        strcpy(type, "dr-x");
        break;
    case FILE_MODE_BLK:
        strcpy(type, "br--");
        break;
    case FILE_MODE_CHR:
        strcpy(type, "cr--");
        break;
    case FILE_MODE_REG:
        strcpy(type, "-r--");
        break;
    }
    printf("%s %8ld %s\n", type, s.size, "a.txt");

    stat("/b", &s);
    switch (s.mode) {
    case FILE_MODE_DIR:
        strcpy(type, "dr-x");
        break;
    case FILE_MODE_BLK:
        strcpy(type, "br--");
        break;
    case FILE_MODE_CHR:
        strcpy(type, "cr--");
        break;
    case FILE_MODE_REG:
        strcpy(type, "-r--");
        break;
    }
    printf("%s %8ld %s\n", type, s.size, "b");
    // struct linux_dirent a[10];
    // int fd = open("/b", 0);
    // int readb = getdents(fd, a, 10 * sizeof(struct linux_dirent));
    // struct linux_dirent *d;
    // char path[256];
    // for (int bpos = 0; bpos < readb;)
    // {
    //     d = (struct linux_dirent *) ((int)a + bpos);
    //     memset(path, 0, sizeof(path));
    // while(1){}
    //     sprintf(path, "/%s", d->name);
    //     // printf("%s\n", d->name);
    //     while (1)
    //     {
    //     }
    //     stat(path, &s);
    //     printf("%-10s ", (s.mode == FILE_MODE_REG) ?  "regular" :
    //                         (s.mode == FILE_MODE_DIR) ?  "directory" :
    //                         (s.mode == FILE_MODE_BLK) ?  "block dev" :
    //                         (s.mode == FILE_MODE_CHR) ?  "char dev" : "???");
    //     printf("%4d   %s\n", s.size,
    //             d->name);
    //     bpos += d->d_reclen;
    // }
    // printf("%d\n", readb);
    return 0;
}