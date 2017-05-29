#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "parse.h"
#include "helper.h"

pid_t ish_pid;

void print_job_list(job*);

void fork_process(job* j, process *p, char *envp[]) {
    int pid;
    if ((pid = fork()) == 0) {
        p->pid = getpid();
        new_child(j, p);
        if (p->input_fd != STDIN_FILENO) dup2(p->input_fd, STDIN_FILENO);
        if (p->output_fd != STDOUT_FILENO) dup2(p->output_fd, STDOUT_FILENO);
        execve(get_program_name(p), get_arg_list(p), envp);
        perror("failed running");
        exit(-1);
    } else if (pid < 0) {
        perror("fork");
        exit(-1);
    } else {
        p->pid = pid;
        set_child_pgid(j,p);
    }
}

void spawn_job(job* j, char *envp[]) {
    char *filename;
    process *p;
    int fd;
    int fds[2];

    for (p = j->process_list; p != NULL; p = p->next) {
        if ((filename = get_input_redirection(p)) != NULL) {
            p->input_fd = open(filename, O_RDONLY);
        }
        if ((filename = get_output_redirection(p)) != NULL) {
            if (get_write_opt(p) == TRUNC) {
                p->output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            } else {
                p->output_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
            }
        }

        if (p->next != NULL) {
            if (pipe(fds) < 0) {
                perror("pipe");
                exit(-1);
            }
            p->output_fd = fds[1];
            p->next->input_fd = fds[0];
        }
        fork_process(j, p, envp);
        if ((fd = p->input_fd) != 0) close(p->input_fd);
        if ((fd = p->output_fd) != 1) close(p->output_fd);
    }

    if (j->mode == FOREGROUND) wait(NULL);
    else waitpid(-j->pgid, NULL, WNOHANG);
}

int main(int argc, char *argv[], char *envp[]) {
    char s[LINELEN];
    job *bgjoblist = NULL;
    job *curr_job;

    if (setpgid(0, 0) < 0) {
        perror("ish init");
        return -1;
    }
    ish_pid = getpid();
    grab_cont(ish_pid);

    // signal action
    if (signal_sethandler(SIGINT, sigint_handler) < 0) {
        perror("SIGINT");
        return -1;
    }
    if (signal_sethandler(SIGTSTP, sigtstp_handler) < 0) {
        perror("SIGTSTP");
        return -1;
    }
    if (signal_sethandler(SIGTTOU, SIG_IGN) < 0) {
        perror("SIGTTOU");
        return -1;
    }

    while(get_line(s, LINELEN)) {
        if(!strcmp(s, "exit\n"))
            break;
        if(!strcmp(s, "fg\n")) {}
        if(!strcmp(s, "bg\n")) {
        }

        curr_job = parse_line(s);
        print_job_list(curr_job);

        spawn_job(curr_job, envp);

        grab_cont(ish_pid);
        // if (curr_job->mode == BACKGROUND) append_job(bgjoblist, curr_job);
        // else free_job(curr_job);
        free_job(curr_job);
    }

    return 0;
}
