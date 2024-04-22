// tar -cv lalal | gzip - | gpg
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // Phase 1
    char *inputFile = argv[1]; // input folder name is myzip or myzip.gpg

    printf("the file / foldr is : %s\n", inputFile);

    // fdarry THIS ARRARY DOES NOT EXIST
    // fd[0] = STDIN
    // fd[1] = STDOUT
    // fd[2] = STDERR
    // fd[3] = pipefdone[0];
    // fd[4] = pipefdone[1];
    // fd[5] = pipefdtwo[0];
    // fd[6] = pipefdtwo[1];

    // ZIP
    printf("\nmyzip\n\n");
    // creating pipeline
    int pipefdone[2];
    int pipefdtwo[2];
    // cheecking pipes have not failed
    if (pipe(pipefdone) == -1)
    {
        perror("pipe");
        exit(1);
    }
    if (pipe(pipefdtwo) == -1)
    {
        perror("pipe");
        exit(1);
    }

    // first child process
    if (!fork())
    {
        close(1);              // closing STDOUT fd[1] = NULL
        dup2(pipefdone[1], 1); // fd[1] = pipefdone[1] --> write to end of first pipe;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[0]);
        close(pipefdtwo[0]);
        close(pipefdtwo[1]);
        // TODO need to create commandline
        execlp("tar", "tar", "czf", "-", inputFile, NULL); // command for tar
        perror("execlp tar");
        exit(EXIT_FAILURE);
    }
    // second child
    if (!fork())
    {
        close(0);              // close stdin // in child fd[0]=NULL
        close(1);              // close stdout // in child fd[1]= NULL
        dup2(pipefdone[0], 0); // in child fd[0]=pipefdone[0] --> duplicate the read end of first pipe to STDIN;
        dup2(pipefdtwo[1], 1); // in child fd[1]=pipetwo[0] --> duplicate the write end of second pipe STDOUT;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[1]);
        close(pipefdtwo[0]);
        // TODO need to create commandline
        execlp("gzip", "gzip", NULL); // command line for gzip
        perror("execpl gzip");
        exit(EXIT_FAILURE);
    }
    // third child
    if (!fork())
    {
        close(0);              // close STDIN - fd[0] = NULL
        dup2(pipefdtwo[0], 0); // fd[0] = pipefdtwo[1] --> duplicate read end of second pipe;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[0]);
        close(pipefdone[1]);
        close(pipefdtwo[1]);
        // TODO need to create commandline
        // output to myzip.gpg
        // TODO MAY NEED TO CHANGE TO SCANF TO GET GPG USER DETAILS UID
        // print(here how you ufnd your id....)
        // scnaf(...enter uid)

        execlp("gpg", "gpg", "--encrypt", "--recipient", "moria grohar <moriagro@gmail.com>", "--output", "myunzip.gpg", NULL); // comand line for gpg
        perror("execpl gpg");
        exit(EXIT_FAILURE);
    }
    // close unused ends of pipes in process to avoid hanging
    close(pipefdone[0]);
    close(pipefdone[1]);
    close(pipefdtwo[0]);
    close(pipefdtwo[1]);
    // wait fot 3 child processes
    wait(NULL);
    wait(NULL);
    wait(NULL);
    // print complete and return 0
    printf("compression completeted terminating\n");
    return 0;
}
