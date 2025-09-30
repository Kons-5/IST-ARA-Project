#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#define CLI_LINE_MAX 1024
#define CLI_PATH_MAX 512

typedef enum {
    CLI_CMD_QUIT,
    CLI_CMD_HELP,
    CLI_CMD_CLEAR,
    CLI_CMD_ERROR,
    CLI_CMD_UNKNOWN,
    CLI_CMD_STABLETYPELENGTH,
    CLI_CMD_OPTIMALTYPELENGTH,
    CLI_CMD_STABLEALL,
    CLI_CMD_OPTIMALALL,
} cmd_type;

typedef struct {
    char path[CLI_PATH_MAX];
    unsigned short t;
} cli_args;

cmd_type cli_parse_line(char *s, cli_args *a);
void cli_print_usage();

#endif
