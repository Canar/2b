#include <stdio.h>
#include <stdlib.h>
#include <aaudio/AAudio.h>

#ifndef RATE
#define RATE 44100
#endif
#ifndef CHANNELS
#define CHANNELS 1
#endif
#define BUFFER_SIZE 1<<12
#define WORD_SIZE 1<<2
#define FRAME_SIZE CHANNELS * WORD_SIZE
#define BUFFER_FRAMES BUFFER_SIZE / FRAME_SIZE

AAudioStreamBuilder *builder;
AAudioStream *stream;

void halt(char ret){
	if(stream)  AAudioStream_close(stream);
	if(builder) AAudioStreamBuilder_delete(builder);
	exit(ret);
}

void check(aaudio_result_t result,const char* action,const char* file,const int line){
	if (result == AAUDIO_OK) return;

	char* err="Failed to %s in file '%s' at line %d:\n\t%s\n";
	if(action) err="Error in file '%2$s' at line %3$d:\n\t%4$s.\n";
	fprintf(stderr,err,action,file,line,AAudio_convertResultToText(result));
	halt(1);
	return;
}

#define AACKM(x,m) check(x,m,__FILE__,__LINE__)
#define AACK(x) AACKM(x,NULL)

void on_error(AAudioStream *stream, void *userData, aaudio_result_t error) {
	fprintf(stderr, "Error: %s\n", AAudio_convertResultToText(error));
}

int on_fill(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames) {
    if (fread(audioData,1,(size_t)(numFrames*FRAME_SIZE),stdin))
		return AAUDIO_CALLBACK_RESULT_CONTINUE;
	halt(0);
	return AAUDIO_CALLBACK_RESULT_STOP;
}

int main() {
//int _start() {
    aaudio_result_t result;

	AACKM(AAudio_createStreamBuilder(&builder),"create stream builder");
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
    //AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, CHANNELS);
    AAudioStreamBuilder_setSampleRate(builder, RATE);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setErrorCallback(builder, on_error, NULL);
    AAudioStreamBuilder_setFramesPerDataCallback(builder, BUFFER_FRAMES);
    AAudioStreamBuilder_setDataCallback(builder, on_fill, NULL);

    AACKM(AAudioStreamBuilder_openStream(builder, &stream),"open stream");
    AACKM(AAudioStream_requestStart(stream),"start stream");

    while (1) {
        // Keep the stream running
    }
	halt(0);
}
