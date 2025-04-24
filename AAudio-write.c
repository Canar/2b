#include <stdio.h>
#include <stdlib.h>
#include <aaudio/AAudio.h>

#define API "AAudio"
#include "args.c"

#define BUFFER_SIZE 1<<12
#define WORD_SIZE 1<<2 //TODO: variable format?
#define TICK_TIME 1000 * 1000 * 1000 / 24 /* 24 ticks/s: cinema! */
#define CK(x,m) check(x,m,__FILE__,__LINE__)

AAudioStreamBuilder *builder;
AAudioStream *stream;

void halt(char ret){
	if (stream)  AAudioStream_close(stream);
	if (builder) AAudioStreamBuilder_delete(builder);
	exit(ret);
}

void check(aaudio_result_t result,const char* action,const char* file,const int line){
	if (result == AAUDIO_OK) return;
	char* err="Error in file '%2$s' at line %3$d:\n\t%4$s.\n";
	if(action) err="Failed to %s in file '%s' at line %d:\n\t%s\n";
	fprintf(stderr,err,action,file,line,AAudio_convertResultToText(result));
	halt(1); /*implies*/ return;
}

int on_fill(AAudioStream *stream, void *user_data, void *audio_data, int32_t frame_count) {
	if(fread(audio_data,1,(size_t)((*(int*)user_data)*frame_count),stdin))
		return AAUDIO_CALLBACK_RESULT_CONTINUE;
	halt(0); /*implies*/ return AAUDIO_CALLBACK_RESULT_STOP;
}

void init_builder(AAudioStreamBuilder*builder,int rate,int channels){
	int frame_size = channels * WORD_SIZE;
	AAudioStreamBuilder_setDirection(builder,AAUDIO_DIRECTION_OUTPUT);
	AAudioStreamBuilder_setFormat(builder,AAUDIO_FORMAT_PCM_FLOAT);
	//AAudioStreamBuilder_setFormat(builder,AAUDIO_FORMAT_PCM_I16); 
	AAudioStreamBuilder_setChannelCount(builder,channels);
	AAudioStreamBuilder_setSampleRate(builder,rate);
	AAudioStreamBuilder_setFramesPerDataCallback(builder,BUFFER_SIZE / frame_size);
	AAudioStreamBuilder_setDataCallback(builder,on_fill,&frame_size);
}

int main(int argc,char**argv) {
	aaudio_result_t result;
	int rate,channels;
	aaudio_stream_state_t state = AAUDIO_STREAM_STATE_STARTING;

	handle_args(argc,argv,&channels,&rate);
	CK(AAudio_createStreamBuilder(&builder),"create stream builder");
	CK(AAudioStreamBuilder_openStream(builder, &stream),"open stream");
	CK(AAudioStream_requestStart(stream),"start stream");

	while (state != AAUDIO_STREAM_STATE_STOPPED && state != AAUDIO_STREAM_STATE_CLOSED)
		AAudioStream_waitForStateChange(stream,state,0,TICK_TIME);
}
