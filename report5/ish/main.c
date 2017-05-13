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

void exec_process(process *curr_prc) {
    char *filename;
    int fds[2];
    int fd;
    
    if ((filename = get_input_redirection(curr_prc)) != NULL) {
        curr_prc->input_fd = open(filename, O_RDONLY);
    }
    if ((filename = get_output_redirection(curr_prc)) != NULL) {
        if (get_write_opt(curr_prc) == TRUNC) {
            curr_prc->output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        } else {
            curr_prc->output_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        }
    }
    
    if (curr_prc->next != NULL) {
        if (pipe(fds) < 0) {
            perror("pipe");
            exit(-1);
        }
        curr_prc->output_fd = fds[1];
        curr_prc->next->input_fd = fds[0];
    }
    fork_process(curr_prc);
    if ((fd = curr_prc->input_fd) != 0) close(curr_prc->input_fd);
    if ((fd = curr_prc->output_fd) != 1) close(curr_prc->output_fd);
    exec_process(curr_prc->next);
}

void fork_process(process *curr_prc) {
    int pid;
    
    if ((pid = fork()) == 0) {
        if (curr_prc->input_fd != 0) dup2(curr_prc->input_fd, 0);
        if (curr_prc->output_fd != 1) dup2(curr_prc->output_fd, 1);
        execve(get_program_name(curr_prc), get_arg_list(curr_prc), envp);
        perror("failed running");
        exit(-1);
    }
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
        exec_process(curr_prc);
        wait(&status);

        free_job(curr_job);
    }

    return 0;
}
