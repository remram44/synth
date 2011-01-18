#include <SDL/SDL.h>
#include <math.h>

static int g_debug = 0;

typedef struct {
    unsigned int nb_channels;
    unsigned char **channels;
} Song;

static void audio_callback(void *udata, Uint8 *stream, int len)
{
    Song *song = (Song*)udata;
    /* TODO : synthétise */
    int i = 0;
    for(; i < len; i++)
        stream[i] = 0;
}

static Song *read_song(FILE *file)
{
    /* TODO : lit une chanson depuis un fichier */
    return NULL;
}

int play(FILE *file, int debug)
{
    SDL_AudioSpec audiospec;
    Song *song = NULL;

    g_debug = debug;

    song = read_song(file);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    audiospec.freq = 44100;
    audiospec.format = AUDIO_S16SYS;
    audiospec.channels = 2;
    audiospec.samples = 1024;
    audiospec.callback = audio_callback;
    audiospec.userdata = (void*)song;
    if(SDL_OpenAudio(&audiospec, NULL) < 0)
    {
        fprintf(stderr, "Impossible d'ouvrir l'audio\n");
        return 1;
    }

    SDL_PauseAudio(0);
    SDL_SetVideoMode(200, 100, 32, 0);

    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}