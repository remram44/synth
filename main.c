#include <stdio.h>
#include <string.h>

int play(FILE *file, int debug);

static void display_help(FILE *file)
{
    fprintf(file, "synth [options] <file>\n"
        "Synthesize music from <file>\n"
        "Available options:\n"
        "  -h,--help : displays this help message and exits\n"
        "  -d,--debug : enables debug output on stderr\n");
}

int main(int argc, char **argv)
{
    int debug = 0;
    const char *filename = NULL;

    (void)argc; /* unused parameter */
    argv++;

    while(*argv)
    {
        if(!strcmp(*argv, "--help") || !strcmp(*argv, "-h"))
        {
            display_help(stdout);
            return 0;
        }
        else if(!strcmp(*argv, "--debug") || !strcmp(*argv, "-d")
         || !strcmp(*argv, "-v"))
            debug = 1;
        else if(!filename)
            filename = *argv;
        else
        {
            fprintf(stderr, "Syntax error: multiple files given\n");
            display_help(stderr);
            return 1;
        }
        argv++;
    }

    if(!filename)
    {
        if(debug)
            fprintf(stderr, "Reading from stdin...\n");
        return play(stdin, debug);
    }
    else
    {
        FILE *file = fopen(filename, "rb");
        if(!file)
        {
            fprintf(stderr, "Unable to open the file\n");
            return 1;
        }
        if(debug)
            fprintf(stderr, "File opened\n");
        return play(file, debug);
    }
}
