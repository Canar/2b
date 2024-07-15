#include <stdio.h>
#include <stdlib.h>
#include <aaudio/AAudio.h>
#include <android/api-level.h>

// #define __ANDROID_API__ 30

#define BUFFER_SIZE 2048

void errorCallback(AAudioStream *stream, void *userData, aaudio_result_t error) {
    fprintf(stderr, "Error: %s\n", AAudio_convertResultToText(error));
}

int fillBufferWithPCMData(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames) {
    size_t size = numFrames * sizeof(int16_t) * 2; // 2 channels (stereo), 16-bit PCM
    fread(audioData, 1, size, stdin);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

int main() {
    AAudioStreamBuilder *builder;
    AAudioStream *stream;
    aaudio_result_t result;

    result = AAudio_createStreamBuilder(&builder);
    if (result != AAUDIO_OK) {
        fprintf(stderr, "Failed to create stream builder: %s\n", AAudio_convertResultToText(result));
        return EXIT_FAILURE;
    }

    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, 2);
    AAudioStreamBuilder_setSampleRate(builder, 44100);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setErrorCallback(builder, errorCallback, NULL);
    AAudioStreamBuilder_setFramesPerDataCallback(builder, BUFFER_SIZE / (sizeof(int16_t) * 2));
    AAudioStreamBuilder_setDataCallback(builder, fillBufferWithPCMData, NULL);

    result = AAudioStreamBuilder_openStream(builder, &stream);
    if (result != AAUDIO_OK) {
        fprintf(stderr, "Failed to open stream: %s\n", AAudio_convertResultToText(result));
        AAudioStreamBuilder_delete(builder);
        return EXIT_FAILURE;
    }

    result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) {
        fprintf(stderr, "Failed to start stream: %s\n", AAudio_convertResultToText(result));
        AAudioStream_close(stream);
        AAudioStreamBuilder_delete(builder);
        return EXIT_FAILURE;
    }

    while (1) {
        // Keep the stream running
    }

    AAudioStream_close(stream);
    AAudioStreamBuilder_delete(builder);

    return EXIT_SUCCESS;
}


//AAudioStream_dataCallback (AAudioStream *, void *, void *, int32_t)' 
//(aka 'int (*(struct AAudioStreamStruct *, void *, void *, int))(struct AAudioStreamStruct *, void *, void *, int)') to parameter of type 
//'AAudioStream_dataCallback' (aka 'int (*)(struct AAudioStreamStruct *, void *, void *, int)'
