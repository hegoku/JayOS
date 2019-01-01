#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char input[1024];
    while (1)
    {
        memset(input, 0, sizeof(input));
        printf("jnix_$: ");
        scanf("%s", input);

        if (strlen(input)==0) {
            continue;
        }
        if (input[0]!='/' || (input[0]=='.' && input[1]!='/')) {
            printf("%s: command not exist\n", input);
            continue;
        }

        int fd = open(input, 0);
        if (fd==-1) {
            printf("%s: command not exist\n", input);
        } else {
            printf("sh_child:%d\n", getpid());
            while(1){}
            int pid = fork();
            // __asm__(
            //     "1:jmpl $2f\n\t"
            //     "2:jmpl $1f"::
            // );
            if (pid != 0)
            {
                printf("sh_child:%d\n", pid);
                while(1){}
                int s;
                waitpid(-1, &s, 0);
            } else {
                // printf("2131");
                while(1){}
                printf("%x\n", input);
                exit(1);
                // execve("/root/HELLO1", 0, 0);
            }
        }
        
    }
}