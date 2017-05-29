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

/**
 * job getter and setter
 */
char *get_program_name(process *j);
char **get_arg_list(process *j);
int has_next_program(process *j);
char *get_input_redirection(process *j);
char *get_output_redirection(process *j);
write_option get_write_opt(process *j);
void append_job(job *list, job *curr);

 /**
  * signal utility
  */
void sigint_handler(int signal);
void sigtstp_handler(int signal);
void signal_sethandler(int signal, void (*handler)(int));
void signal_block(int signal);
void signal_unblock(int signal);

/**
 * process control
 */
 void grab_cont(pid_t pgid);
 void set_child_pgid(job *j, process *p);
 void new_child(job *j, process *p);
