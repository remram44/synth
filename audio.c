#include <SDL/SDL.h>
#include <math.h>

static int g_debug = 0;

typedef struct {
    unsigned int instrument;
    float volume;
    float *notes;
} Channel;

typedef struct {
    unsigned int nb_channels;
    unsigned int length;
    unsigned int pos;
    unsigned char **channels;
} Chunk;

typedef struct {
    unsigned int nb_chunks;
    float *chunks;
} Song;

static void audio_callback(void *udata, Uint8 *stream, int len)
{
    Song *song = (Song*)udata;
    /* TODO : synthesize */
    int i = 0;
    for(; i < len; i++)
        stream[i] = 0;
}

typedef union {
    char command;
    struct {
        char command;
        char *param;
    } string;
    struct {
        char command;
        int param;
    } number;
} Command;

static int read_line(FILE *file, Command *ret)
{
    static char buffer[1024];

    ret->command = '\0';

    if(!fgets(buffer, 1024, file))
    {
        if(g_debug)
            fprintf(stderr, "End of file reached\n");
        return -1;
    }

    /* reads the command (one character) */
    {
        char *com = strtok(buffer, " \t");
        if(!com || com[0] == '\0' || com[0] == '#' || com[1] != '\0')
            /* Blank or commented line */
            return 0;
        ret->command = com[0];
    }

    /* reads the parameter */
    {
        char *param = strtok(NULL, " \t");
        if(!param)
        {
            if(g_debug)
                fprintf(stderr, "No parameter given to %c command\n",
                        ret->command);
            return -1;
        }
        switch(ret->command)
        {
        case 'l': /* label */
        case 'j': /* goto label */
        case 'n': /* notes */
            ret->string.param = malloc(sizeof(char) * (strlen(param) + 1));
            strcpy(ret->string.param, param);
            break;
        case 'p': /* start of chunk (number of channels) */
        case 'v': /* volume for channel */
        case 'i': /* instrument for channel */
            {
                char *endptr;
                ret->number.param = strtol(param, &endptr, 10);
                if(*endptr != '\0')
                {
                    if(g_debug)
                        fprintf(stderr, "Non-numeric parameter to %c command\n",
                                ret->command);
                    return -1;
                }
            }
            break;
        default:
            if(g_debug)
                fprintf(stderr, "Unknown command %c\n", ret->command);
            return -1;
        }
    }

    return 0;
}

static Song *read_song(FILE *file)
{
    Command command;
    if(g_debug)
        fprintf(stderr, "Starting parsing of song file...\n");
    while(read_line(file, &command) != -1)
    {
        switch(command.command)
        {
        case '\0':
            break;
        case 'l':
        case 'j':
        case 'n':
        case 'p':
        case 'v':
        case 'i':
            fprintf(stderr, "Warning: '%c' is currently unimplemented\n",
                command.command);
            break;
        }
    }
    if(g_debug)
        fprintf(stderr, "Song file parsed\n");
    return NULL;
}

int play(FILE *file, int debug)
{
    SDL_AudioSpec audiospec;
    Song *song = NULL;

    g_debug = debug;

    song = read_song(file);

    if(g_debug)
        fprintf(stderr, "Initializing SDL...\n");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    audiospec.freq = 44100;
    audiospec.format = AUDIO_S16SYS;
    audiospec.channels = 2;
    audiospec.samples = 1024;
    audiospec.callback = audio_callback;
    audiospec.userdata = (void*)song;
    if(SDL_OpenAudio(&audiospec, NULL) < 0)
    {
        fprintf(stderr, "Unable to open audio context\n");
        return 1;
    }

    if(g_debug)
        fprintf(stderr, "Starting audio...\n");

    SDL_PauseAudio(0);
    SDL_SetVideoMode(200, 100, 32, 0);

    if(g_debug)
        fprintf(stderr, "Stopping audio and exiting gracefully\n");

    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}