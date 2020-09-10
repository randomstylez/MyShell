#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

/* Author: Joseph Pietroluongo
// PID: 5901749
// I affirm that I wrote this program myself without any help
// from any other people or sources from the internet
// Summary: extend the myshell.c program and add pipelines and I/O redirections
*/

#define ARGS 20
#define SIZE 1024

typedef struct command
{
    int number_args;
    int position_args;
} Command;

//checks args
int get_args(char *cmdline, char *args[])
{
    int i = 0;

    //Sanity checks
    if ((args[0] = strtok(cmdline, "\n\t ")) == NULL)
        return 0;

    while ((args[++i] = strtok(NULL, "\n\t ")) != NULL)
    {
        if (i >= ARGS)
        {
            printf("Too many arguments!\n");
            exit(1);
        }
    }
    //null check
    return i;
}


//Takes args array, lists desired sections
int pipes_function(Command **list, int number_pipes, char **args)
{
    int p[2];
    pid_t pid;
    int file_descriptor_in = 0;

//Ocelot int outside loop
    int i = 0;

    while (i < number_pipes)
    {
        pipe(p);
        if ((pid = fork()) == -1)
            exit(EXIT_FAILURE);

        else if (pid == 0)
        {
            int j;

            char *current[(list[i]->number_args) + 1];
            current[(list[i]->number_args)] = NULL;

            for (j = (list[i]->position_args); j < (list[i]->position_args) + (list[i]->number_args); j++)
            {
                current[j - ((list[i]->position_args))] = args[j];
            }

            char *in_file = NULL;
            char *out_file = NULL;

            int redirect_in = 0;
            int redirect_out = 0;
            int append = 0;

            int file_descript;
            int file_descript_1;

            mode_t mode;
            mode = O_RDONLY;

//Sanity and error checks
            for (j = 0; current[j] != NULL; j++)
            {
                if (!strcmp(current[j], "<"))
                {
                    if (i != 0)
                    {
                        printf("Ambiguous input redirection, please check.\n");
                        return -1;
                    }

                    in_file = current[j + 1];

                    if (in_file == NULL)
                    {
                        printf("Invalid input file, please check.\n");
                        return -1;
                    }
                    redirect_in++;
                    current[j] = NULL;
                    j++;

                    file_descript = open(in_file, mode);
                    if (file_descript < 0)
                    {
                        fprintf(stderr, "error creating file, please check.\n");
                        return -1;
                    }

                    dup2(file_descript, 0);
                }
                else if (!strcmp(current[j], ">"))
                {
                    if (i != number_pipes - 1)
                    {
                        printf("Ambiguous output redirect, please check.\n");
                        return -1;
                    }
                    if (append == 1)
                    {
                        printf("Ambiguous output redirect, please check.\n");
                        return -1;
                    }

                    out_file = current[j + 1];

                    if (out_file == NULL)
                    {
                        printf("Invalid output file, please check.\n");
                        return -1;
                    }
                    redirect_out++;
                    current[j] = NULL;
                    j++;

                    file_descript_1 = creat(out_file, 0640);
                    if (file_descript_1 < 0)
                    {
                        fprintf(stderr, "error creating file, please check.\n");
                        exit(1);
                    }

                    dup2(file_descript_1, 1);
                }
                else if (!strcmp(current[j], ">>"))
                {
                    if (i != number_pipes - 1)
                    {
                        printf("Ambiguous output redirect, please check.\n");
                        return -1;
                    }
                    if (redirect_out == 1)
                    {
                        printf("Ambiguous output redirect, please check.\n");
                        return -1;
                    }

                    out_file = current[j + 1];

                    if (out_file == NULL)
                    {
                        printf("Invalid output file, please check.\n");
                        return -1;
                    }
                    append++;
                    current[j] = NULL;
                    j++;

                    file_descript_1 = fileno(fopen(out_file, "a"));
                    if (file_descript_1 < 0)
                    {
                        fprintf(stderr, "error creating file, please check.\n");
                        exit(1);
                    }

                    dup2(file_descript_1, 1);
                }
            }

            if (number_pipes != 1)
            {
                if (i == 0)
                    dup2(p[1], 1);
                else if (i != number_pipes - 1)
                {
                    dup2(file_descriptor_in, 0);
                    dup2(p[1], 1);
                }
                else if (i == number_pipes - 1)
                {
                    dup2(file_descriptor_in, 0);
                }
            }

            close(p[0]);
            execvp(current[0], current);
            exit(EXIT_FAILURE);
        }
        else
        {
            wait(NULL);
            close(p[1]);
            file_descriptor_in = p[0];
            i++;
        }
    }
    return 0;
}

void execute(char *cmdline)
{
    int i, count = 0, j = -1;
    int count_block = 0;
    char *args[ARGS];

    int nargs = get_args(cmdline, args);
    if (nargs <= 0)
        return;

    if (!strcmp(args[0], "quit") || !strcmp(args[0], "exit"))
        exit(0);

    for (i = 0; i < nargs; i++)
        if (!strcmp(args[i], "|"))
            count++;

    count++;

    Command *list[count];

    for (i = 0; i < nargs; i++)
    {
        if (!strcmp(args[i], "|"))
        {
            Command *command = (Command *)malloc(sizeof(Command));
            command->position_args = j + 1;
            command->number_args = i - (j + 1);
            j = i;

            list[count_block] = command;
            count_block++;
        }
        else if (i == nargs - 1)
        {
            Command *command = (Command *)malloc(sizeof(Command));
            command->position_args = j + 1;
            command->number_args = i - j;

            list[count_block] = command;
        }
    }

    pipes_function(list, count, args);
}

//desired output
int main(int argc, char *argv[])
{
    char cmdline[SIZE];

    for (;;)
    {
        printf("COP4338$ ");
        if (fgets(cmdline, SIZE, stdin) == NULL)
        {
            perror("Error, please check.");
            exit(1);
        }
        execute(cmdline);
    }
    return 0;
}