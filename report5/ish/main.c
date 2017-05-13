#include <string.h>
#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

void print_job_list(job*);
char *get_program_name(process *j) {
    return j->program_name;
}
char **get_arg_list(process *j) {
    return j->argument_list;
}
int has_next_program(process *j) {
    return j->next != NULL;
}
char *get_input_redirection(process *j) {
    return j->input_redirection;
}
char *get_output_redirection(process *j) {
    return j->output_redirection;
}
write_option get_write_opt(process *j) {
    return j->output_option;
}

int main(int argc, char *argv[], char *envp[]) {
    char s[LINELEN];
    job *curr_job;
    process *curr_prc;
    int pid;
    int status;
    int fd[2];
    int rfd;

    while(get_line(s, LINELEN)) {
        if(!strcmp(s, "exit\n"))
            break;

        curr_job = parse_line(s);
        curr_prc = curr_job->process_list;

        print_job_list(curr_job);
        if (has_next_program(curr_prc)) {
            while (has_next_program(curr_prc)) {
                // pipe
                if (pipe(fd) < 0) {
                    perror("pipe");
                    return -1;
                }

                if ((pid = fork()) == 0) {
                    close(fd[0]);
                    dup2(fd[1], 1);
                    execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
                    perror("failed running pipe 1");
                    return -1;
                }
                if ((pid = fork()) == 0) {
                    close(fd[1]);
                    dup2(fd[0], 0);
                    curr_prc = curr_prc->next;
                    if (get_output_redirection(curr_prc) != NULL) {
                        rfd = open(get_output_redirection(curr_prc), )
                        dup2()
                    }
                    execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
                    perror("failed running pipe 2");
                    return -1;
                }
                close(fd[0]);
                close(fd[1]);
                wait(&status);
                if (get_input_redirection(curr_prc) != NULL) {
                    if ((pid = fork()) == 0) {
                        fd[0] = open(get_input_redirection(curr_prc), O_RDONLY);
                        dup2(fd[0], 0);
                        execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
                        perror("failed running in redirection");
                        return -1;
                    }
                    close(fd[0]);
                    wait(&status);
                } else if (get_output_redirection(curr_prc) != NULL) {
                    if ((pid = fork()) == 0) {
                        fd[1] = open(get_input_redirection(curr_prc), O_RDONLY);
                        dup2(fd[1], 1);
                        execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
                        perror("failed running in redirection");
                        return -1;
                    }
                    close(fd[1]);
                    wait(&status);
                } else {
                    
                }
            }
        } else {
            if ((pid = fork()) == 0) {
                execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
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
