#include <stdio.h>
#include <stdlib.h>
#include <aaudio/AAudio.h>

#define BUFFER_SIZE 2048

AAudioStreamBuilder *builder;
AAudioStream *stream;

void halt(char ret){
	if(stream)  AAudioStream_close(stream);
	if(builder) AAudioStreamBuilder_delete(builder);
	exit(ret);
}

void check(aaudio_result_t result,const char* action,const char* file,const int line){
	if (result != AAUDIO_OK){
		if(action){
			fprintf(stderr,"Error '%s' in file '%s' at line %d.",AAudio_convertResultToText(result),file,line);
		}else{
			fprintf(stderr,"Failed to %s in file '%s' at line %d. %s",action,file,line,AAudio_convertResultToText(result));
		}
		halt(1);
	}
}

#define AACKM(x,m) check(x,m,__FILE__,__LINE__)
#define AACK(x) AACKM(x,NULL)

void errorCallback(AAudioStream *stream, void *userData, aaudio_result_t error) {
    fprintf(stderr, "Error: %s\n", AAudio_convertResultToText(error));
}

int fillBufferWithPCMData(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames) {
    size_t size = numFrames * sizeof(int16_t) * 2; // 2 channels (stereo), 16-bit PCM
    fread(audioData, 1, size, stdin);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

int main() {
    aaudio_result_t result;

	 AACKM(AAudio_createStreamBuilder(&builder),"create stream builder");

    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, 2);
    AAudioStreamBuilder_setSampleRate(builder, 44100);
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setErrorCallback(builder, errorCallback, NULL);
    AAudioStreamBuilder_setFramesPerDataCallback(builder, BUFFER_SIZE / (sizeof(int16_t) * 2));
    AAudioStreamBuilder_setDataCallback(builder, fillBufferWithPCMData, NULL);

    AACKM(AAudioStreamBuilder_openStream(builder, &stream),"open stream");
    AACKM(AAudioStream_requestStart(stream),"start stream");

    while (1) {
        // Keep the stream running
    }
	 halt(0);
}
