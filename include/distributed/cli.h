#ifndef CLI_H
#define CLI_H

#define CLI_LINE_MAX 1024
#define CLI_PATH_MAX 512

typedef enum {
    CLI_CMD_QUIT,
    CLI_CMD_HELP,
    CLI_CMD_ERROR,
    CLI_CMD_UNKNOWN,
    CLI_CMD_SIMUSIMPLE,
    CLI_CMD_SIMUCOMPLETE,
    CLI_CMD_SIMUCOMPLETEALL,
} cmd_type;

typedef struct {
    char path[CLI_PATH_MAX];
    unsigned short t;
    double d;
} cli_args;

cmd_type cli_parse_line(char *s, cli_args *a);
void cli_print_usage();

#endif
