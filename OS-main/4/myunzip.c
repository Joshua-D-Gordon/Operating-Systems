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

    // MYUNZIP

    strcpy(inputFile, "myunzip.gpg");
    printf("\nmyunzip.gpg\n\n");
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
    // child one
    if (!fork())
    {
        close(1);              // close STDOUT fd[1] = NULL
        dup2(pipefdone[1], 1); // fd[1] = pipefdone[1] --> write to end of first pipe;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[0]);
        close(pipefdtwo[0]);
        close(pipefdtwo[1]);
        // comand line for gpg dycrypt
        execlp("gpg", "gpg", "--decrypt", "--output", "-", inputFile, NULL);
        perror("execlp gpg");
        exit(EXIT_FAILURE);
    }
    // child two
    if (!fork())
    {
        close(0);              // close STDIN in child
        close(1);              // close STDOUT in child
        dup2(pipefdone[0], 0); // in child fd[0]=pipefdone[0] --> duplicate the read end of first pipe to STDIN;
        dup2(pipefdtwo[1], 1); // in child fd[1]=pipetwo[0] --> duplicate the write end of second pipe STDOUT;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[1]);
        close(pipefdtwo[0]);
        // use comand gunzip to unzip contents
        execlp("gunzip", "gunzip", NULL);
        perror("execlp gunzip");
        exit(EXIT_FAILURE);
    }
    // child three
    if (!fork())
    {
        close(0);              // close STDIN - fd[0] = NULL
        dup2(pipefdtwo[0], 0); // fd[0] = pipefdtwo[0] --> duplicate read end of second pipe;

        // close unused ends of pipes in child process to avoid hanging
        close(pipefdone[0]);
        close(pipefdone[1]);
        close(pipefdtwo[1]);
        // Todo use tar xzf -C ./myunzip to put contents in myunzip folder
        // TODO TO MAKE A DIR

        execlp("tar", "tar", "xzf", "-", "-C", "myunzipFolder", NULL);
        perror("execlp tar");
        // printf(stderr, "errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // close unused ends of pipes in process to avoid hanging
    close(pipefdone[0]);
    close(pipefdone[1]);
    close(pipefdtwo[0]);
    close(pipefdtwo[1]);
    // wait for 3 child processes
    wait(NULL);
    wait(NULL);
    wait(NULL);
    // print finished unzip terminiating and return 0
    printf("finished unzip terminiating");

    return 0;
    // no folder/ command myzip or myunzip
}


// fdarry THIS ARRARY DOES NOT EXIST
// fd[0] = STDIN
// fd[1] = STDOUT
// fd[2] = STDERR
// fd[3] = pipefdone[0];
// fd[4] = pipefdone[1];
// fd[5] = pipefdtwo[0];
// fd[6] = pipefdtwo[1];
