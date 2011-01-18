#include <stdio.h>
#include <string.h>

int play(FILE *file, int debug);

static void display_help(FILE *file)
{
    fprintf(file, "synth [options] <fichier>\n"
        "Synthétise la musique écrite dans le fichier <fichier>\n"
        "Options reconnues :\n"
        "  -h,--help : affiche cette aide et quitte\n"
        "  -d,--debug : affiche des infos de débogage sur la sortie\n");
}

int main(int argc, char **argv)
{
    int debug = 0;
    const char *filename = NULL;

    (void)argc; /* unused parameter */

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
            filename = *++argv;
        else
        {
            fprintf(stderr, "Erreur de syntaxe : plusieurs fichiers "
                "spécifiés\n");
            display_help(stderr);
            return 1;
        }
        argv++;
    }

    if(!filename)
    {
        if(debug)
            fprintf(stderr, "Lecture depuis stdin...\n");
        return play(stdin, debug);
    }
    else
    {
        FILE *file = fopen(filename, "rb");
        if(!file)
        {
            fprintf(stderr, "Impossible d'ouvrir le fichier spécifié\n");
            return 1;
        }
        if(debug)
            fprintf(stderr, "Fichier ouvert\n");
        return play(file, debug);
    }
}
