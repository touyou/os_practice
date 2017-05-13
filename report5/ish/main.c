#include <string.h>
#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void print_job_list(job*);
char *get_program_name(job *j) {
    return j->process_list->program_name;
}
char **get_arg_list(job *j) {
    return j->process_list->argument_list;
}
int has_next_program(job *j) {
    return j->process_list->next != NULL;
}
char *get_input_redirection(job *j) {
    return j->process_list->input_redirection;
}
char *get_output_redirection(job *j) {
    return j->process_list->output_redirection;
}
write_option get_write_opt(job *j) {
    return j->process_list->output_option;
}

int main(int argc, char *argv[], char *envp[]) {
    char s[LINELEN];
    job *curr_job;
    int pid;
    int status;
    int fd[2];

    while(get_line(s, LINELEN)) {
        if(!strcmp(s, "exit\n"))
            break;

        curr_job = parse_line(s);

        // print_job_list(curr_job);
        if (has_next_program(curr_job)) {
            if (get_input_redirection(curr_job) != NULL) {
            } else if (get_output_redirection(curr_job) != NULL) {
            } else {
                // pipe
                if (pipe(fd) < 0) {
                    perror("pipe");
                    return -1;
                }

                if ((pid = fork()) == 0) {
                    close(fd[0]);
                    dup2(fd[1], 1);
                    execve(get_program_name(curr_job), get_arg_list(curr_job), envp);
                    perror("failed running pipe 1");
                    return -1;
                }
                if ((pid = fork()) == 0) {
                    close(fd[1]);
                    dup2(fd[0], 0);
                    execve(get_program_name(curr_job), get_arg_list(curr_job), envp);
                    perror("failed running pipe 2");
                    return -1;
                }
                close(fd[0]);
                close(fd[1]);
                wait(&status);
            }
        } else {
            if ((pid = fork()) == 0) {
                execve(get_program_name(curr_job), get_arg_list(curr_job), envp);
                perror("failed running");
                return -1;
            } else {
                waitpid(pid, &status, WUNTRACED);
            }
        }

        free_job(curr_job);
    }

    return 0;
}
