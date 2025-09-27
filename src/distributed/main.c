#include "../../include/distributed/cli.h"
#include "../../include/distributed/sim.h"
#include <stdio.h>

#define forever for(;;)

static inline void cli_prompt(void) {
    printf("\x1b[?25h$ ");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    char buffer[CLI_LINE_MAX];
    cli_args args = {0};
    cli_print_usage();

    forever {
        cli_prompt();
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }

        switch (cli_parse_line(buffer, &args)) {
            case CLI_CMD_QUIT:
                return 0;

            case CLI_CMD_HELP:
                cli_print_usage();
                break;

            case CLI_CMD_ERROR:
                return 1;

            case CLI_CMD_SIMUSIMPLE:
                SimuSimple(args.path, args.t, args.d);
                break;

            case CLI_CMD_SIMUCOMPLETE:
                SimuComplete(args.path, args.t, args.d);
                break;

            case CLI_CMD_SIMUCOMPLETEALL:
                SimuCompleteAll(args.path, args.d);
                break;

            default:
                break;
        }
    }

    return 0;
}
