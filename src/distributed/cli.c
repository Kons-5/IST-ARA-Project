#include "../../include/distributed/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void trim_line(char *s) {
    if (!s) {
        return;
    }

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
    if (!s) {
        return CLI_CMD_ERROR;
    }
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
        a->d = 0.0;
        int n = 0;

        // SimuSimple("path/to/net", t, d)
        if (sscanf(s, "SimuSimple( \"%511[^\"]\" , %hu , %lf ) %n", a->path, &a->t, &a->d, &n) == 3 && s[n] == '\0') {
            if (a->t < 0u || a->t > 65535u) {
                return CLI_CMD_ERROR;
            }
            if (a->d < 0.0) {
                return CLI_CMD_ERROR;
            }
            return CLI_CMD_SIMUSIMPLE;
        }

        // SimuComplete("path/to/net", t, d)
        if (sscanf(s, "SimuComplete( \"%511[^\"]\" , %hu , %lf ) %n", a->path, &a->t, &a->d, &n) == 3 && s[n] == '\0') {
            if (a->t < 0u || a->t > 65535u) {
                return CLI_CMD_ERROR;
            }
            if (a->d < 0.0) {
                return CLI_CMD_ERROR;
            }
            return CLI_CMD_SIMUCOMPLETE;
        }

        // SimuCompleteAll("path/to/net", d)
        if (sscanf(s, "SimuComplete( \"%511[^\"]\" , %lf ) %n", a->path, &a->d, &n) == 2 && s[n] == '\0') {
            if (a->d < 0.0) {
                return CLI_CMD_ERROR;
            }
            return CLI_CMD_SIMUCOMPLETE;
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
    fprintf(stdout, "> SimuSimple(\"path/to/net\", t, d) \n");
    fprintf(stdout, "> SimuComplete(\"path/to/net\", t, d) \n");
    fprintf(stdout, "> SimuCompleteAll(\"path/to/net\") \n");
    fprintf(stdout, "> quit | exit | q \n");
    fprintf(stdout, "> clear | c \n");
    fprintf(stdout, "> help | ? \n\n");
}
