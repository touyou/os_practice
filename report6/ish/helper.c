#include "helper.h"

/* job getter and setter */
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

void append_job(job *list, job *curr) {
    if (list == NULL) {
        list = initialize_job();
        list = curr;
    } else append_job(list->next, curr);
}

/* signal utility */
void sigint_handler(int signal) {}
void sigtstp_handler(int signal) {}

char *signal_to_string(int signal) {
    switch (signal) {
        case SIGINT: return "SIGINT";
        case SIGTSTP: return "SIGTSTP";
        case SIGCHLD: return "SIGCHLD";
        case SIGTTIN: return "SIGTTIN";
        case SIGTTOU: return "SIGTTOU";
        default: return "UNKNOWN";
    }
}

void signal_sethandler(int signal, void (*handler)(int)) {
    sigset_t emptymask;
    sigemptyset(&emptymask);
    struct sigaction sa = {
        .sa_handler = handler,
        .sa_mask = emptymask,
        .sa_flags = 0
    };
    if (sigaction(signal, &sa, NULL) < 0) {
        perror(signal_to_string(signal));
        exit(-1);
    }
}

void signal_block(int signal) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
}

void signal_unblock(int signal) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, signal);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

/* process control */
void grab_cont(pid_t pgid) {
    if(tcsetpgrp(STDIN_FILENO, pgid) < 0) {
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
    signal_sethandler(SIGTTOU, SIG_DFL);
    signal_sethandler(SIGTTIN, SIG_DFL);
}
