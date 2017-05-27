/*
 * esh - the 'pluggable' shell.
 *
 */
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>
#include "esh-sys-utils.h"
#include "esh.h"

static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
           " -h            print this help\n"
           " -p  plugindir directory from which to load plug-ins\n",
           progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */
static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;
    struct list_elem * e = list_begin(&esh_plugin_list);

    for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL) {
            continue;
        }

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL) {
        prompt = strdup("esh> ");
    }

    return prompt;
}

/*
 * Returns the list of current jobs
 */
static struct list * get_jobs(void) {
    return &current_jobs;
}

/*
 * Searches the pipeline and returns the job corresponding to jid
 * Returns the job, or NULL if not found
 */
static struct esh_pipeline * get_job_from_jid(int jid) {
    struct list_elem *e;
    for (e = list_begin(&current_jobs); e != list_end(&current_jobs); e = list_next(e)) {
        struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
        if (job->jid == jid) {
            return job;
        }
    }

    return NULL;
}

/*
 * Searches the pipeline and return the job corresponding to pgrp
 * Returns the job, or NULL if not found
 */
static struct esh_pipeline * get_job_from_pgrp(pid_t pgrp) {
    struct list_elem *e;
    for (e = list_begin(&current_jobs); e != list_end(&current_jobs); e = list_next(e)) {
        struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
        if (job->pgrp == pgrp) {
            return job;
        }
    }

    return NULL;
}

/* /\* */
/*  * Searches the commands (processes) and returns the process */
/*  * corresponding to pid */
/*  *\/ */
/* static struct esh_command * get_cmd_from_pid(pid_t pid) */
/* { */
/*     // */
/* } */

/*
 * Prints out the commands for a job
 */
static void print_job_commands(struct list jobs)
{
    struct list_elem *e = list_begin(&jobs);
    struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);

    printf("(");
    for (e = list_begin(&pipeline->commands); e != list_end(&pipeline->commands); e = list_next(e)) {

        struct esh_command *command = list_entry(e, struct esh_command, elem);

        char **argv = command->argv;
        while (*argv) {
            printf("%s ", *argv);
            argv++;
        }

        if (list_size(&pipeline->commands) > 1) {
            printf("| ");
        }
    }

    printf(")\n");
}

static void print_single_job(struct esh_pipeline *pipeline)
{
    printf("(");

    struct list_elem *e;
    for (e = list_begin(&pipeline->commands); e != list_end(&pipeline->commands); e = list_next(e)) {

        struct esh_command *command = list_entry(e, struct esh_command, elem);

        char **argv = command->argv;
        while (*argv) {
            printf("%s ", *argv);
            fflush(stdout);
            argv++;
        }

        if (list_size(&pipeline->commands) > 1) {
            printf("| ");
        }
    }

    printf(")\n");
}

/*
 * Change the status of a given job
 */
static void change_job_status(pid_t pid, int status)
{
    if (pid > 0) {

        struct list_elem *e;
        for (e = list_begin(&current_jobs); e != list_end(&current_jobs); e = list_next(e)) {

            struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);

            if (pipeline->pgrp == pid) {

                if (WIFSTOPPED(status)) {

                    if (WSTOPSIG(status) == 22) {
                        pipeline->status = STOPPED;
                    }

                    else {
                        pipeline->status = STOPPED;
                        printf("\n[%d]+ Stopped      ", pipeline->jid);
                        print_job_commands(current_jobs);
                    }
                }

                if (WTERMSIG(status) == 9) {
                    list_remove(e);
                }

                else {
                    // printf("\n");
                }

                // normal termination
                if (WIFEXITED(status)) {
                    list_remove(e);
                }

                else if (WIFSIGNALED(status)) {
                    list_remove(e);
                }

                else if (WIFCONTINUED(status)) {
                    list_remove(e);
                }

                if (list_empty(&current_jobs)) {
                    jid = 0;
                }
            }
        }
    }

    else if (pid < 0) {
        esh_sys_fatal_error("Error in wait ");
    }
}

/*
 * SIGCHLD Handler
 */
static void child_handler(int sig, siginfo_t *info, void *_ctxt)
{
    assert (sig == SIGCHLD);

    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0) {
        change_job_status(pid, status);
    }
}

/**
 * Assign ownership of ther terminal to process group
 * pgrp, restoring its terminal state if provided.
 *
 * Before printing a new prompt, the shell should
 * invoke this function with its own process group
 * id (obtained on startup via getpgrp()) and a
 * sane terminal state (obtained on startup via
 * esh_sys_tty_init()).
 *
 * Taken from Dr. Back's Snippet.
 */
static void
give_terminal_to(pid_t pgrp, struct termios *pg_tty_state)
{
    esh_signal_block(SIGTTOU);
    int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
    if (rc == -1) {
        esh_sys_fatal_error("tcsetpgrp: ");
    }

    if (pg_tty_state) {
        esh_sys_tty_restore(pg_tty_state);
    }
    esh_signal_unblock(SIGTTOU);
}

/*
 * Wait
 */
static void wait_for_job(struct esh_command_line *cline, struct esh_pipeline *pipeline, struct termios *shell_tty)
{
    int status;
    pid_t pid;

    if ((pid = waitpid(-1, &status, WUNTRACED)) > 0) {
        give_terminal_to(getpgrp(), shell_tty);
        change_job_status(pid, status);
    }
}

/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell = {
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */
    .parse_command_line = esh_parse_command_line, /* Default parser */
    .get_jobs = get_jobs,
    .get_job_from_jid = get_job_from_jid,
    .get_job_from_pgrp = get_job_from_pgrp
    /* .get_cmd_from_pid = get_cmd_from_pid */
};

int
main(int ac, char *av[])
{
    int opt;
    jid = 0;
    list_init(&esh_plugin_list);
    list_init(&current_jobs);

    /* Process command-line arguments. See getopt(3) */
    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }

    esh_plugin_load_from_directory("plugins/");
    esh_plugin_initialize(&shell);
    setpgid(0, 0);
    struct termios *shell_tty = esh_sys_tty_init();
    give_terminal_to(getpgrp(), shell_tty);

    /* Read/eval loop. */
    for (;;) {

        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
        char * cmdline = shell.readline(prompt);
        free (prompt);

        if (cmdline == NULL) { /* User typed EOF */
            break;
        }

        struct esh_command_line * cline = shell.parse_command_line(cmdline);
        free (cmdline);
        if (cline == NULL) {                /* Error in command line */
            continue;
        }

        if (list_empty(&cline->pipes)) {    /* User hit enter */
            esh_command_line_free(cline);
            continue;
        }

        struct esh_pipeline *pipeline;
        pipeline = list_entry(list_begin(&cline->pipes), struct esh_pipeline, elem);

        struct esh_command *commands;
        commands = list_entry(list_begin(&pipeline->commands), struct esh_command, elem);
        //esh_command_print(command);

	int command_type = process_type(commands->argv[0]);

	//PLUGIN CHECK
	struct list_elem * e = list_begin(&esh_plugin_list);
	for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
	    struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

	    if (plugin->process_builtin(commands)) {
		command_type = -1;
		continue;
	    }
	}

        if (command_type == 1) {
            exit(EXIT_SUCCESS);
        }

        // jobs
        else if (command_type == 2) {

            char *statusStrings[] = {"Foreground","Running","Stopped", "Needs Terminal"};
            struct list_elem *e;
            for (e = list_begin(&current_jobs); e != list_end(&current_jobs); e = list_next(e)) {
                struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);
                printf("[%d] %s ", pipeline->jid, statusStrings[pipeline->status]);
                print_single_job(pipeline);
            }
        }

        // fg, bg, kill, stop
        else if (command_type == 3 || command_type == 4 || command_type == 5 || command_type == 6) {

            if (!list_empty(&current_jobs)) {

                int jobid_arg = -1;

                if (commands->argv[1] == NULL) {
                    struct list_elem *e = list_back(&current_jobs);
                    struct esh_pipeline *pipeline = list_entry(e, struct esh_pipeline, elem);
                    jobid_arg = pipeline->jid;
                }

                else {

                    if (strncmp(commands->argv[1], "%", 1) == 0) {
                        char *temp = (char*) malloc(5);
                        strcpy(temp, commands->argv[1]+1);
                        jobid_arg = atoi(temp);
                        free(temp);
                    }

                    else {
                        jobid_arg = atoi(commands->argv[1]);
                    }
                }

                struct esh_pipeline *pipeline;
                pipeline = get_job_from_jid(jobid_arg);

                // fg
                if (command_type == 3) {

                    esh_signal_block(SIGCHLD);
                    pipeline->status = FOREGROUND;
                    print_single_job(pipeline);
                    give_terminal_to(pipeline->pgrp, shell_tty);

                    if (kill (-pipeline->pgrp, SIGCONT) < 0) {
                        esh_sys_fatal_error("fg error: kill SIGCONT ");
                    }

                    wait_for_job(cline, pipeline, shell_tty);
                    esh_signal_unblock(SIGCHLD);
                }

                // bg
                if (command_type == 4) {

                    pipeline->status = BACKGROUND;

                    if (kill(-pipeline->pgrp, SIGCONT) < 0) {
                        esh_sys_fatal_error("SIGCONT Error ");
                    }

                    print_job_commands(current_jobs);
                    printf("\n");
                }

                // kill
                else if (command_type == 5) {
                    if (kill(-pipeline->pgrp, SIGKILL) < 0) {
                        esh_sys_fatal_error("SIGKILL Error ");
                    }
                }

                // stop
                else if (command_type == 6) {
                    if (kill(-pipeline->pgrp, SIGSTOP) < 0) {
                        esh_sys_fatal_error("SIGSTOP Error ");
                    }
                }
            }
        }

        else if (command_type == 0) {

            /*
             * Don't think this is pipeline friendly.
             */

            esh_signal_sethandler(SIGCHLD, child_handler);

            jid++;
            if (list_empty(&current_jobs)) {
                jid = 1;
            }

            pipeline->jid = jid;
            pipeline->pgrp = -1;
            pid_t pid;

            // pipes
            int oldPipe[2], newPipe[2];
            bool isPiped = false;

            if (list_size(&pipeline->commands) > 1) {
                isPiped = true;
            }

            struct list_elem *e;
            for (e = list_begin(&pipeline->commands); e != list_end(&pipeline->commands); e = list_next(e)) {

                struct esh_command *command = list_entry(e, struct esh_command, elem);

                //esh_command_print(command);

                if(isPiped && list_next(e) != list_tail(&pipeline->commands)) {
                    pipe(newPipe);
                }

                esh_signal_block(SIGCHLD);
                pid = fork();

                // child
                if (pid == 0) {

                    pid = getpid();
                    command->pid = pid;

                    if (pipeline->pgrp == -1) {
                        pipeline->pgrp = pid;
                    }

                    if (setpgid(pid, pipeline->pgrp) < 0) {
                        esh_sys_fatal_error("Error Setting Process Group ");
                    }

                    if (!pipeline->bg_job) {
                        give_terminal_to(pipeline->pgrp, shell_tty);
                        pipeline->status = FOREGROUND;
                    }

                    else {
                        pipeline->status = BACKGROUND;
                    }

                    if (isPiped && e != list_begin(&pipeline->commands)) {
                        // closing and duplicating old pipe
                        close(oldPipe[1]);
                        dup2(oldPipe[0], 0);
                        close(oldPipe[0]);
                    }

                    if (isPiped && list_next(e) != list_tail(&pipeline->commands)) {
                        // closing and duplicating new pipe
                        close(newPipe[0]);
                        dup2(newPipe[1], 1);
                        close(newPipe[1]);
                    }

                    if (command->iored_input != NULL) {
                        int in_fd = open(command->iored_input, O_RDONLY);
                        if (dup2(in_fd, 0) < 0) {
                            esh_sys_fatal_error("dup2 error ");
                        }
                        close(in_fd);
                    }

                    if (command->iored_output != NULL) {
                        int out_fd;
                        if (command->append_to_output) {
                            out_fd = open(command->iored_output, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        }

                        else {
                            out_fd = open(command->iored_output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        }

                        if (dup2(out_fd, 1) < 0) {
                            esh_sys_fatal_error("dup2 error ");
                        }

                        close(out_fd);
                    }

                    if (execvp(command->argv[0], command->argv) < 0) {
                        esh_sys_fatal_error("Exec Error ");
                    }
                }

                else if (pid < 0) {
                    esh_sys_fatal_error("Fork Error ");
                }

                // parent
                else {

                    if (pipeline->pgrp == -1) {
                        pipeline->pgrp = pid;
                    }

                    if (setpgid(pid, pipeline->pgrp) < 0) {
                        esh_sys_fatal_error("Error Setting Process Group ");
                    }

                    if (isPiped && e != list_begin(&pipeline->commands)) {
                        // closing old pipes
                        close(oldPipe[0]);
                        close(oldPipe[1]);
                    }

                    if (isPiped && list_next(e) != list_tail(&pipeline->commands)) {
                        // setting oldPipe to newPipe
                        oldPipe[0] = newPipe[0];
                        oldPipe[1] = newPipe[1];
                    }

                    if (isPiped && list_next(e) == list_tail(&pipeline->commands)) {
                        close(oldPipe[0]);
                        close(oldPipe[1]);
                        close(newPipe[0]);
                        close(newPipe[1]);
                    }
                }
            }

            if (pipeline->bg_job) {
                pipeline->status = BACKGROUND;
                printf("[%d] %d\n", pipeline->jid, pipeline->pgrp);
            }

            e = list_pop_front(&cline->pipes);
            list_push_back(&current_jobs, e);

            if (!pipeline->bg_job) {
                wait_for_job(cline, pipeline, shell_tty);
            }

            esh_signal_unblock(SIGCHLD);
        }

        esh_command_line_free(cline);
    }

    return 0;

}

/*
 * Checks if the command passed can be handled by the esh shell.
 * If it is not a builtin command, returns 0.
 */
int process_type(char *command)
{
    if (!strcmp(command, "exit")) {
        return 1;
    }

    else if (!strcmp(command, "jobs")) {
        return 2;
    }

    else if (!strcmp(command, "fg")) {
        return 3;
    }

    else if (!strcmp(command, "bg")) {
        return 4;
    }

    else if (!strcmp(command, "kill")) {
        return 5;
    }

    else if (!strcmp(command, "stop")) {
        return 6;
    }

    return 0;
}
