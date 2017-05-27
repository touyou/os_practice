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

pid_t ish_pid;

void print_job_list(job*);

int signal_sethandler(int signal, sa_sigaction_t handler) {
    sigset_t emptymask;
    sigemptyset(&emptymask);
    struct sigaction sa = {
        .sa_sigaction = handler,
        .sa_mask = emptymask,
        .sa_flags = SA_RESTART | SA_SIGINFO
    }
    return sigaction(signal, &sa, NULL);
}

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
void ignore_foreground_signal(int signal) {}

void grab_cont(pid_t pgid) {
    if(tcsetpgrp(0, pgid) < 0) {
        perror("tcsetpgrp grab_cont");
        exit(-1);
    }
}

void set_child_pgid(job *j, process *p) {
    if (j->pgid < 0) {
        j->pgid = p->pid;
    }

    setpgid(p->pid, j->pgid);
}

void new_child(job *j, process *p) {
    p->pid = getpid();

    set_child_pgid(j, p);

    if (j->mode == FOREGROUND) {
        grab_cont(j->pgid);
    }

    // SIGTTOUをデフォルトに直す？
    struct sigaction def;
    memset(&def, 0, sizeof(def));
    def.sa_handler = SIG_DFL;
    def.sa_flags = 0;
    if (sigaction(SIGTTOU, &def, NULL) < 0) {
        perror("SIGTTOU");
        exit(-1);
    }
}

void fork_process(job* j, process *p, char *envp[]) {
    int pid;
    if ((pid = fork()) == 0) {
        p->pid = getpid();
        new_child(j, p);
        if (p->input_fd != 0) dup2(p->input_fd, 0);
        if (p->output_fd != 1) dup2(p->output_fd, 1);
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
    else {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGCHLD);
        sigprocmask(SIG_BLOCK, &sigset, NULL);
        waitpid(-j->pgid, NULL, WNOHANG);
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }
}

void append_job(job *list, job *curr) {
    if (list->next == NULL) list->next = curr;
    else append_job(list->next, curr);
}

int main(int argc, char *argv[], char *envp[]) {
    char s[LINELEN];
    job *bgjoblist;
    job *curr_job;
    int status;
    struct sigaction act;
    struct sigaction none;
    memset(&act, 0, sizeof(act));
    memset(&none, 0, sizeof(none));
    act.sa_handler = ignore_foreground_signal;
    act.sa_flags = 0;
    none.sa_handler = SIG_IGN;
    none.sa_flags = 0;

    // signal action
    if ((status = sigaction(SIGINT, &act, NULL)) < 0) {
        perror("SIGINT");
        return -1;
    }
    if ((status = sigaction(SIGTSTP, &act, NULL)) < 0) {
        perror("SIGTSTP");
        return -1;
    }
    if ((status = sigaction(SIGTTOU, &none, NULL)) < 0) {
        perror("SIGTTOU");
        return -1;
    }

    ish_pid = getpid();
    if (setpgid(ish_pid, ish_pid) < 0) {
        perror("ish init");
        return -1;
    }

    while(get_line(s, LINELEN)) {
        if(!strcmp(s, "exit\n"))
            break;
        if(!strcmp(s, "fg\n")) {}
        if(!strcmp(s, "bg\n")) {
        }

        curr_job = parse_line(s);
        print_job_list(bgjoblist);

        spawn_job(curr_job, envp);

        grab_cont(ish_pid);
        if (curr_job->mode == BACKGROUND) append_job(bgjoblist, curr_job);
        else free_job(curr_job);
    }
    free_job(bgjoblist);

    return 0;
}
