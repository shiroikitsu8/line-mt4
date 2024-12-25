#include "audio.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <string.h>
#include <unordered_map>

bool audioInitialized = false;
SDL_AudioDeviceID audioDevice;
SDL_AudioStream *audioStream;

void specCallback(void *a1, uint8_t *data, int len)
{
    if (SDL_AudioStreamAvailable(audioStream) < len)
        return;

    int total = 0;
    while (true)
    {
        int gotten = SDL_AudioStreamGet(audioStream, data + total, len - total);
        if (gotten < 0)
            break;
        total += gotten;
        if (total >= len)
            break;
    }
}

int jmp_nsAdrv_init(const char *card_id, const char *device_name, int channels, int bits, int rate, int period_size, void *hwmp)
{
    if (audioInitialized)
    {
        printf("nsAdrv already initialized\n");
        return 0;
    }
    SDL_AudioSpec playbackSpec;
    SDL_zero(playbackSpec);
    playbackSpec.freq = rate;
    playbackSpec.format = AUDIO_S32;
    playbackSpec.channels = channels;
    playbackSpec.samples = 256;
    playbackSpec.callback = specCallback;

    SDL_AudioSpec receivedSpec;
    audioDevice = SDL_OpenAudioDevice(NULL, SDL_FALSE, &playbackSpec, &receivedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (!audioDevice)
    {
        printf("Can't find a audio device\n");
        return 0;
    }

    printf("Device: %s\nChannels %d\nFrequency: %d\n", SDL_GetAudioDeviceName(audioDevice, SDL_FALSE), receivedSpec.channels, receivedSpec.freq);

    audioStream = SDL_NewAudioStream(AUDIO_S32, channels, rate, receivedSpec.format, receivedSpec.channels, receivedSpec.freq);
    if (!audioStream)
    {
        printf("Can't create a audio stream\n");

        SDL_CloseAudioDevice(audioDevice);
        return 0;
    }

    audioInitialized = true;

    SDL_PauseAudioDevice(audioDevice, 0);
    return 0;
}

int jmp_nsAdrv_term(const char *card_id)
{
    if (!audioInitialized)
    {
        printf("nsAdrv not initialized yet\n");
        return 0;
    }
    SDL_FreeAudioStream(audioStream);
    SDL_CloseAudioDevice(audioDevice);
    audioInitialized = false;
    return 0;
}

int jmp_nsAdrv_wait(int ms)
{
    if (!audioInitialized)
        return 256;
    if (SDL_AudioStreamAvailable(audioStream) >= 6144)
        return 0;
    return 256;
}

int jmp_nsAdrv_start()
{
    return 0;
}

void jmp_nsAdrv_mixup(void *hwmp)
{
}

int jmp_nsAdrv_mixsts(int channel)
{
    return 0;
}

void jmp_nsAdrv_write(uint8_t *data, int size)
{
    if (!audioInitialized)
        return;
    SDL_AudioStreamPut(audioStream, data, 6144);
}

std::unordered_map<const char *, void *> AUDIO_FUNC_STUBS = {
    {"nsAdrv_init", (void *)jmp_nsAdrv_init},
    {"nsAdrv_term", (void *)jmp_nsAdrv_term},
    {"nsAdrv_wait", (void *)jmp_nsAdrv_wait},
    {"nsAdrv_start", (void *)jmp_nsAdrv_start},
    {"nsAdrv_mixup", (void *)jmp_nsAdrv_mixup},
    {"nsAdrv_mixsts", (void *)jmp_nsAdrv_mixsts},
    {"nsAdrv_write", (void *)jmp_nsAdrv_write}};

#define AUDIO_DL_ID 192352

bool audio_dlopen(const char *lib_name, void **result)
{
    if (strcmp(lib_name, "./nsAdrv.dll") == 0)
    {
        *result = (void *)AUDIO_DL_ID;
        return true;
    }
    return false;
}
bool audio_dlsym(void *handle, const char *symbol, void **result)
{
    if ((int)handle != AUDIO_DL_ID)
        return false;
    for (auto &it : AUDIO_FUNC_STUBS) {
        if (strcmp(it.first, symbol) == 0)
        {
            *result = it.second;
            return true;
        }
    }
    *result = NULL;
    return true;
}