#include <stdio.h>
#include <stdlib.h>
#include <aaudio/AAudio.h>

#define BUFFER_SIZE 1<<12
#define WORD_SIZE 1<<2 //TODO: variable format?
#define TICK_TIME 1000 * 1000 * 1000 /* 1 second, 10^9ps */
#define API "AAudio"

#include "args.c"

AAudioStreamBuilder *builder;
AAudioStream *stream;

void halt(char ret){
	if(stream)  AAudioStream_close(stream);
	if(builder) AAudioStreamBuilder_delete(builder);
	exit(ret);
}

void check(aaudio_result_t result,const char* action,const char* file,const int line){
	if (result == AAUDIO_OK) return;
	char* err="Error in file '%2$s' at line %3$d:\n\t%4$s.\n";
	if(action) err="Failed to %s in file '%s' at line %d:\n\t%s\n";
	fprintf(stderr,err,action,file,line,AAudio_convertResultToText(result));
	halt(1); /*implies*/ return;
}

#define AACKM(x,m) check(x,m,__FILE__,__LINE__)
#define AACK(x) AACKM(x,NULL)

void on_error(AAudioStream *stream, void *userData, aaudio_result_t error) {
	fprintf(stderr, "Error: %s\n", AAudio_convertResultToText(error));
}

int on_fill(AAudioStream *stream, void *user_data, void *audio_data, int32_t frame_count) {
	if (fread(audio_data,1,(size_t)((*(int*)user_data)*frame_count),stdin))
		return AAUDIO_CALLBACK_RESULT_CONTINUE;
	halt(0); /*implies*/ return AAUDIO_CALLBACK_RESULT_STOP;
}

int main(int argc,char**argv) {
	aaudio_result_t result;

	AACKM(AAudio_createStreamBuilder(&builder),"create stream builder");
	AAudioStreamBuilder_setDirection(builder,AAUDIO_DIRECTION_OUTPUT);
	AAudioStreamBuilder_setErrorCallback(builder,on_error,NULL);
	AAudioStreamBuilder_setFormat(builder,AAUDIO_FORMAT_PCM_FLOAT);
	//AAudioStreamBuilder_setFormat(builder,AAUDIO_FORMAT_PCM_I16); 

	int rate,channels;
	handle_args(argc,argv,&channels,&rate);
	AAudioStreamBuilder_setChannelCount(builder,channels);
	AAudioStreamBuilder_setSampleRate(builder,rate);

	int frame_size = channels * WORD_SIZE;
	AAudioStreamBuilder_setFramesPerDataCallback(builder,BUFFER_SIZE / frame_size);
	AAudioStreamBuilder_setDataCallback(builder,on_fill,&frame_size);

	AACKM(AAudioStreamBuilder_openStream(builder, &stream),"open stream");
	AACKM(AAudioStream_requestStart(stream),"start stream");

	aaudio_stream_state_t state = AAUDIO_STREAM_STATE_STARTING;
	while (state != AAUDIO_STREAM_STATE_STOPPED && state != AAUDIO_STREAM_STATE_CLOSED)
		AAudioStream_waitForStateChange(stream,state,0,TICK_TIME);

	halt(0);
}
