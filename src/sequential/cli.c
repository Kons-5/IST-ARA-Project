#include "../../include/sequential/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void trim_line(char *s) {
    if (!s)
        return;

    // drop newline(s)
    s[strcspn(s, "\r\n")] = '\0';

    // left trim
    char *p = s + strspn(s, " \t\f\v");
    if (p != s) {
        memmove(s, p, strlen(p) + 1);
    }

    // right trim
    size_t n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\f' || s[n - 1] == '\v')) {
        s[--n] = '\0';
    }
}

cmd_type cli_parse_line(char *s, cli_args *a) {
    if (!s)
        return CLI_CMD_ERROR;
    trim_line(s);

    if (strcmp(s, "help") == 0 || strcmp(s, "?") == 0) {
        return CLI_CMD_HELP;
    }

    if (strcmp(s, "quit") == 0 || strcmp(s, "exit") == 0 || strcmp(s, "q") == 0) {
        return CLI_CMD_QUIT;
    }

    if (strcmp(s, "clear") == 0 || strcmp(s, "c") == 0) {
        return CLI_CMD_CLEAR;
    }

    if (a) {
        a->path[0] = '\0';
        a->t = 0;
        int n = 0;

        // StableTypeLength("path/to/net", t)
        if (sscanf(s, "StableTypeLength( \"%511[^\"]\" , %hu ) %n", a->path, &a->t, &n) == 2 && s[n] == '\0') {
            if (a->t < 0 || a->t > 65535)
                return CLI_CMD_ERROR;
            return CLI_CMD_STABLETYPELENGTH;
        }

        // StableAll("path/to/net")
        if (sscanf(s, "StableAll( \"%511[^\"]\" ) %n", a->path, &n) == 1 && s[n] == '\0') {
            return CLI_CMD_STABLEALL;
        }

        // OptimalTypeLength("path/to/net", t)
        if (sscanf(s, "OptimalTypeLength( \"%511[^\"]\" , %hu ) %n", a->path, &a->t, &n) == 2 && s[n] == '\0') {
            if (a->t < 0 || a->t > 65535)
                return CLI_CMD_ERROR;
            return CLI_CMD_OPTIMALTYPELENGTH;
        }

        // OptimalAll("path/to/net")
        if (sscanf(s, "OptimalAll( \"%511[^\"]\" ) %n", a->path, &n) == 1 && s[n] == '\0') {
            return CLI_CMD_OPTIMALALL;
        }
    }

    return CLI_CMD_UNKNOWN;
}

void cli_print_usage(void) {
    printf("\x1b[2J\x1b[H");
    fflush(stdout);

    char *pwd = getcwd(NULL, 0);
    if (!pwd) {
        pwd = "(unknown)";
    }
    fprintf(stdout, "Working directory:\n%s\n\n", pwd);
    free(pwd);

    fprintf(stdout, "Available commands list: \n");
    fprintf(stdout, "> StableTypeLength(\"path/to/net\", t) \n");
    fprintf(stdout, "> StableAll(\"path/to/net\") \n");
    fprintf(stdout, "> OptimalTypeLength(\"path/to/net\", t) \n");
    fprintf(stdout, "> OptimalAll(\"path/to/net\") \n");
    fprintf(stdout, "> quit  | exit | q \n");
    fprintf(stdout, "> clear | c \n");
    fprintf(stdout, "> help  | ? \n\n");
}
