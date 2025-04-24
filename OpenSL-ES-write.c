#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

SLObjectItf sles=NULL, outmix=NULL, playerObject=NULL;
SLEngineItf engine = NULL;
SLPlayItf playerPlay = NULL;
SLAndroidSimpleBufferQueueItf playerBufferQueue = NULL;

#define BUFFER_SIZE 8192
char buffer[BUFFER_SIZE];

volatile int eof=0;
#define CALL(object,method,...) (*(object))->method((object),##__VA_ARGS__)
#define DESTROY(x) if(x!=NULL) CALL(x,Destroy)

void fill_buffer(SLAndroidSimpleBufferQueueItf bq, void *context) {
	size_t bytes_read = fread(buffer,1,BUFFER_SIZE,stdin);
	eof=bytes_read<=0;
	CALL(bq,Enqueue,buffer,bytes_read);
}

int main(int argc, char *argv[]) {
	slCreateEngine(&sles, 0, NULL, 0, NULL, NULL);
	CALL(sles,Realize,SL_BOOLEAN_FALSE);
	CALL(sles,GetInterface,SL_IID_ENGINE,&engine);
	CALL(engine,CreateOutputMix,&outmix,0,NULL,NULL);
	CALL(outmix,Realize,SL_BOOLEAN_FALSE);
	/*
	SLDataFormat_PCM format_pcm = {
		SL_DATAFORMAT_PCM,
		2, // Channels
		SL_SAMPLINGRATE_44_1,
		SL_PCMSAMPLEFORMAT_FIXED_16,
		SL_PCMSAMPLEFORMAT_FIXED_16,
		SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
		SL_BYTEORDER_LITTLEENDIAN
	};
	*/
	SLAndroidDataFormat_PCM_EX format_pcm = {
		SL_ANDROID_DATAFORMAT_PCM_EX, 1, SL_SAMPLINGRATE_44_1, 32, 32,
		SL_SPEAKER_FRONT_LEFT, SL_BYTEORDER_LITTLEENDIAN, SL_ANDROID_PCM_REPRESENTATION_FLOAT
	};
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};
	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outmix};
	SLDataSink audioSnk = {&loc_outmix, NULL};
	const SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req[] = {SL_BOOLEAN_TRUE};
	CALL(engine,CreateAudioPlayer, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
	CALL(playerObject,Realize,SL_BOOLEAN_FALSE);
	CALL(playerObject,GetInterface, SL_IID_PLAY, &playerPlay);
	CALL(playerObject,GetInterface, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerBufferQueue);
	CALL(playerBufferQueue,RegisterCallback,fill_buffer, NULL);
	CALL(playerPlay,SetPlayState, SL_PLAYSTATE_PLAYING);
	fill_buffer(playerBufferQueue, NULL);
	
	while (!eof) usleep(1000 * 1000 / 24); //24Hz sleep. Cinema!
	
	DESTROY(playerObject);
	DESTROY(outmix);
	DESTROY(sles);

	return 0;
}
