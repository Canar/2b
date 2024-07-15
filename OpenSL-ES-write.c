#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// OpenSL ES engine and output mix objects
SLObjectItf engineObject = NULL;
SLEngineItf engineEngine = NULL;
SLObjectItf outputMixObject = NULL;

// Player objects
SLObjectItf playerObject = NULL;
SLPlayItf playerPlay = NULL;
SLAndroidSimpleBufferQueueItf playerBufferQueue = NULL;

// Buffer to hold PCM data
#define BUFFER_SIZE 8192
short buffer[BUFFER_SIZE];

// Callback function for buffer queue
void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    size_t bytesRead = fread(buffer, sizeof(short), BUFFER_SIZE, stdin);
    if (bytesRead > 0) {
        (*bq)->Enqueue(bq, buffer, bytesRead * sizeof(short));
    }
}

// Function to initialize OpenSL ES
void initOpenSLES() {
    // Create and realize engine
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    // Create output mix
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // Configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        2, // Channels
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // Configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // Create audio player
    const SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    (*playerObject)->GetInterface(playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerBufferQueue);

    // Register callback function
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, NULL);

    // Set player to playing state
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);

    // Enqueue initial buffer
    playerCallback(playerBufferQueue, NULL);
}

// Function to clean up OpenSL ES
void cleanupOpenSLES() {
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playerPlay = NULL;
        playerBufferQueue = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
}

int main(int argc, char *argv[]) {
    initOpenSLES();

    // Keep the main thread alive to process audio
    while (1) {
        // Sleep to save CPU
        usleep(10000);
    }

    cleanupOpenSLES();
    return 0;
}
