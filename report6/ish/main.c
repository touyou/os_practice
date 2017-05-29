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
job *joblist = NULL;
job *curr_job;

void print_job_list(job*);

void sigchld_handler(int signal) {
    waitpid(-1, NULL, WNOHANG | WUNTRACED);
}

void sigtstp_chld_handler(int signal) {
    grab_cont(ish_pid);
}

void fork_process(job* j, process *p, char *envp[]) {
    int pid;
    if ((pid = fork()) == 0) {
        p->pid = getpid();
        new_child(j, p);
        if (p->input_fd != STDIN_FILENO) dup2(p->input_fd, STDIN_FILENO);
        if (p->output_fd != STDOUT_FILENO) dup2(p->output_fd, STDOUT_FILENO);
        if (j->mode == FOREGROUND) signal_sethandler(SIGTSTP, sigtstp_chld_handler);
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

    if (j == NULL) return;

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

    if (j->mode == FOREGROUND) {
        int status;
        waitpid(-1, &status, WUNTRACED);
        if (WIFEXITED(status)) {
            free_job(j);
        } else if (WIFSTOPPED(status)) {
            j->next = joblist;
            joblist = j;
        }
    }
}

int main(int argc, char *argv[], char *envp[]) {
    char s[LINELEN];

    ish_pid = getpid();
    if (setpgid(ish_pid, ish_pid) < 0) {
        perror("ish init");
        return -1;
    }
    grab_cont(ish_pid);

    // signal action
    signal_sethandler(SIGINT, sigint_handler);
    signal_sethandler(SIGTSTP, sigtstp_handler);
    signal_sethandler(SIGCHLD, sigchld_handler);
    signal_sethandler(SIGTTIN, SIG_IGN);
    signal_sethandler(SIGTTOU, SIG_IGN);

    while(get_line(s, LINELEN)) {
        if(!strcmp(s, "exit\n"))
            break;
        if(!strcmp(s, "fg\n")) {
        }
        if(!strcmp(s, "bg\n")) {
            // print_job_list(joblist);
            if (joblist != NULL) {
                kill(joblist->pgid, SIGCONT);
                joblist = joblist->next;
            }
            continue;
        }

        curr_job = parse_line(s);

        spawn_job(curr_job, envp);

        grab_cont(ish_pid);
    }
    if (!joblist) free_job(joblist);
    if (!curr_job) free_job(curr_job);

    return 0;
}
