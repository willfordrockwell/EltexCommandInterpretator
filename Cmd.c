#include <sys/wait.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define CMD_SIZE 80

int Get_Entries(char *In, char Entry)
{
    int Count = 0;
    while (*In) {
        if (*In == Entry) {
            Count++;
        }
        In++;
    }
    return Count;
}

int main(int argc, char const *argv[])
{
    char Command[CMD_SIZE];
    char SubCommand[CMD_SIZE];
    int Length;

    int Pipe_Entries;
    int OldSTDOUT;
    int Status;
    int Stat;

    int *Pipes;
    pid_t *Pid;
    char **Tasks;

    while (1) {
        memset(Command, 0, CMD_SIZE);
        printf(">_ ");
        fgets(Command, CMD_SIZE, stdin);

        //replace last '\n' to 0
        Command[strlen(Command) - 1] = 0;

        if(strncmp(Command, "quit", 4) == 0) {
            break;
        }

        //counting num of pipes and creating it
        Pipe_Entries = Get_Entries(Command, '|');
        Pipes = (int *) calloc(2 * Pipe_Entries, sizeof(int));
        for (int i = 0; i < Pipe_Entries; i++) {
            pipe(&Pipes[2 * i]);
        }

        //saving stdout (does it need?)
        OldSTDOUT = dup(STDOUT_FILENO);

        //create tasks for each fork
        Tasks = (char**) calloc (Pipe_Entries + 1, sizeof(char*));

        memset(SubCommand, 0, CMD_SIZE);
        //parse tasks
        for (int i = 0; i < Pipe_Entries + 1; i++) {
            if (i == 0) {
                strcpy(SubCommand, strtok(Command, "|"));
            }
            else {
                strcpy(SubCommand, strtok(NULL, "|"));
            }

            Length = strlen(SubCommand);
            //remove first symbol if it space
            if (SubCommand[0] == ' ') {
                for (int j = 0; j < Length - 1; j++) {
                    SubCommand[j] = SubCommand[j + 1];
                }
                SubCommand[Length - 1] = '\0';
                Length--;
            }

            //remove last symbol if it space
            if (SubCommand[Length - 1] == ' ') {
                SubCommand[Length - 1] = '\0';
                Length--;
            }
            Tasks[i] = (char *) calloc (Length, sizeof(char));
            strcpy(Tasks[i], SubCommand);
            memset(SubCommand, 0, CMD_SIZE);
        }

        //forking
        Pid = (pid_t *) calloc (Pipe_Entries + 1, sizeof(pid_t));
        
        for (int i = 0; i < Pipe_Entries + 1; i++) {
            Pid[i] = fork();
            if (Pid[i] == 0) {
                //child
                int Arg_Entries = Get_Entries(Tasks[i], ' ');
                char **Args = (char **) calloc(Arg_Entries + 2, sizeof(char *));

                //parsing to execv
                for (int j = 0; j < Arg_Entries + 1; j++) {
                    if (j == 0) {
                        strcpy(SubCommand, strtok(Tasks[i], " "));
                    }
                    else {
                        strcpy(SubCommand, strtok(NULL, " "));
                    }
                    Args[j] = (char *) calloc (strlen(SubCommand), sizeof(char));
                    strcpy(Args[j], SubCommand);
                    memset(SubCommand, 0, CMD_SIZE);
                }
                Args[Arg_Entries + 1] = NULL;

                //operating with pipes and exec
                //first command
                if (i == 0) {
                    for (int j = 0; j < 2 * Pipe_Entries; j++) {
                        if (j == 1) {
                            if (Pipe_Entries != 0) {
                                dup2(Pipes[j], STDOUT_FILENO);
                            }
                        }
                    }
                    execvp(Args[0], Args);
                }
                //command in middle
                if (i > 0 && i < Pipe_Entries) {
                    for (int j = 0; j < 2 * Pipe_Entries; j++) {
                        if (2 * i - 2 == j) {
                            dup2(Pipes[j], STDIN_FILENO);
                        }
                        if (2 * i + 1 == j) {
                            dup2(Pipes[j], STDOUT_FILENO);
                        }
                    }
                    execvp(Args[0], Args);
                }
                //last command
                for (int j = 0; j < 2 * Pipe_Entries; j++) {
                    if (2 * i - 2 == j) {
                        dup2(Pipes[j], STDIN_FILENO);
                    }
                }
                dup2(OldSTDOUT, STDOUT_FILENO);
                execvp(Args[0], Args);
            }
            //parent wait to end of fork
            Status = waitpid(Pid[i], &Stat, 0);
            if (i < Pipe_Entries) {
                close(Pipes[2 * i + 1]);
            }
        }

        for (int i = 0; i < 2 * Pipe_Entries; i++) {
            if (i != 1) {
                close(Pipes[i]);
            }
        }

        for (int i = 0; i < Pipe_Entries + 1; i++) {
            free(Tasks[i]);
        }
        free(Tasks);

        free(Pipes);
        free(Pid);
    }
    return 0;
}
