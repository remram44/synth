#include <SDL/SDL.h>
#include <math.h>

static int g_debug = 0;

typedef struct {
    unsigned int instrument;
    float volume;
    unsigned int length;
    float *notes;
} Channel;

typedef struct Chunk_s {
    unsigned int nb_channels;
    unsigned int pos;
    Channel **channels;
    struct Chunk_s *next;
} Chunk;

typedef struct {
    unsigned int nb_chunks;
    Chunk *chunks_f;
    Chunk *chunks_l;
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
    const char *const token_delim = " \t\r\n";

    ret->command = '\0';

    if(!fgets(buffer, 1024, file))
    {
        if(g_debug)
            fprintf(stderr, "End of file reached\n");
        return -1;
    }

    /* reads the command (one character) */
    {
        char *com = strtok(buffer, token_delim);
        if(!com || com[0] == '\0' || com[0] == '#' || com[1] != '\0')
            /* Blank or commented line */
            return 0;
        ret->command = com[0];
    }

    /* reads the parameter */
    {
        char *param = strtok(NULL, token_delim);
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

static Channel *new_channel(void)
{
    Channel *channel = malloc(sizeof(Channel));
    channel->instrument = 0;
    channel->volume = 100;
    channel->length = 0;
    channel->notes = NULL;
    return channel;
}

static Chunk *new_chunk(Song *song, size_t nb_channels)
{
    Chunk *chunk;
    song->nb_chunks++;
    if(song->chunks_l)
    {
        song->chunks_l->next = malloc(sizeof(Chunk));
        song->chunks_l = song->chunks_l->next;
    }
    else
        song->chunks_f = song->chunks_l = malloc(sizeof(Chunk));
    chunk = song->chunks_l;
    chunk->next = NULL;

    chunk->nb_channels = nb_channels;
    chunk->pos = 0;
    chunk->channels = malloc(sizeof(Channel*) * nb_channels);

    if(g_debug)
        fprintf(stderr, "Creating channel 0\n");
    chunk->channels[0] = new_channel();
    return chunk;
}

static float note2freq(char note)
{
    static float freqs[12] = {32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25,
        49., 51.91, 55., 58.27, 61.74};
    float freq;
    size_t index;
    if(note >= 'A' && note <= 'Z')
        index = note - 'A';
    else if(note >= 'a' && note <= 'z')
        index = note - 'a' + 26;
    else if(note >= '0' && note <= '9')
        index = note - '0' + 26 + 26;
    else
        index = 0;
    freq = freqs[index%12];
    index /= 12;
    while(index--)
        freq *= 2;
    return freq;
}

static Channel *find_channel(Song *song, Chunk **chunk)
{
    /* last chunk is finished */
    if((*chunk)->pos == (*chunk)->nb_channels)
    {
        (*chunk)->pos = 0;
        *chunk = NULL;
    }
    /* no current chunk ; auto-allocate one */
    if(!*chunk)
    {
        if(g_debug)
            fprintf(stderr, "Autocreating chunk (1 channel)\n");
        *chunk = new_chunk(song, 1);
    }
    return (*chunk)->channels[(*chunk)->pos];
}

static Song *read_song(FILE *file)
{
    Command command;
    Song *song = malloc(sizeof(Song));
    Chunk *chunk = NULL;
    song->nb_chunks = 0;
    song->chunks_f = song->chunks_l = NULL;
    if(g_debug)
        fprintf(stderr, "Starting parsing of song file...\n");
    while(read_line(file, &command) != -1)
    {
        switch(command.command)
        {
        case '\0':
            break;
        case 'p':
            {
                if(g_debug)
                    fprintf(stderr, "New chunk (%d channels)\n",
                            command.number.param);
                if(command.number.param < 1)
                    return NULL;
                chunk = new_chunk(song, command.number.param);
            }
            break;
        case 'n':
            {
                size_t i;
                /* select channel from chunk
                 * a new chunk may be allocated */
                Channel *channel = find_channel(song, &chunk);

                /* fill it with notes! */
                channel->length = strlen(command.string.param + 1);
                channel->notes = malloc(sizeof(float) * channel->length);
                for(i = 0; i < channel->length; i++)
                    channel->notes[i] = note2freq(command.string.param[i]);
                if(g_debug)
                    fprintf(stderr, "%u notes read into channel %u\n",
                        channel->length, chunk->pos);

                /* to next channel */
                chunk->pos++;
                if(chunk->pos < chunk->nb_channels)
                    chunk->channels[chunk->pos] = new_channel();
            }
            break;
        case 'l':
        case 'j':
        case 'v':
        case 'i':
            fprintf(stderr, "Warning: '%c' is currently unimplemented\n",
                command.command);
            break;
        }
    }
    if(chunk->pos != chunk->nb_channels)
    {
        if(g_debug)
            fprintf(stderr, "Missing %d channels in chunk\n",
                chunk->nb_channels - chunk->pos);
        return NULL;
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