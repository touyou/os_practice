#ifndef __PARSE_H__
#define __PARSE_H__

#define PROMPT "ish$ " /* 入力ライン冒頭の文字列 */
#define NAMELEN 32    /* 各種名前の長さ */
#define ARGLSTLEN 16  /* 1つのプロセスがとる実行時引数の数 */
#define LINELEN 256   /* 入力コマンドの長さ */

typedef enum write_option_ {
    TRUNC,
    APPEND,
} write_option;

typedef struct process_ {
    char*        program_name;
    char**       argument_list;

    char*        input_redirection;

    write_option output_option;
    char*        output_redirection;
    
    int input_fd;
    int output_fd;

    struct process_* next;
} process;

typedef enum job_mode_ {
    FOREGROUND,
    BACKGROUND,
} job_mode;

typedef struct job_ {
    job_mode     mode;
    process*     process_list;
    struct job_* next;
} job;

typedef enum parse_state_ {
    ARGUMENT,
    IN_REDIRCT,
    OUT_REDIRCT_TRUNC,
    OUT_REDIRCT_APPEND,
} parse_state;

char* get_line(char *, int);
job* parse_line(char *);
void free_job(job *);

#endif
