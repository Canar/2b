#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

SLObjectItf engineObject=NULL, outputMixObject=NULL, playerObject=NULL;
SLEngineItf engineEngine = NULL;
SLPlayItf playerPlay = NULL;
SLAndroidSimpleBufferQueueItf playerBufferQueue = NULL;

#define BUFFER_SIZE 8192
short buffer[BUFFER_SIZE];

volatile int eof=0;
#define CALL(object,method,...) (*(object))->method((object),##__VA_ARGS__)
#define DESTROY(x) if(x!=NULL) CALL(x,Destroy)
void cleanupOpenSLES() {
	DESTROY(playerObject);
	DESTROY(outputMixObject);
	DESTROY(engineObject);
}

void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    size_t bytes_read = fread(buffer, sizeof(short), BUFFER_SIZE, stdin);
	eof=bytes_read<=0;
	CALL(bq,Enqueue,buffer,bytes_read * sizeof(short));
}

void initOpenSLES() {
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	CALL(engineObject,Realize,SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    
	(*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);

    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
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
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    (*playerObject)->GetInterface(playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &playerBufferQueue);

    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, NULL);
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    playerCallback(playerBufferQueue, NULL);
}
int main(int argc, char *argv[]) {
    initOpenSLES();
    while(!eof)usleep(1000 * 1000 / 24);//24Hz sleep. Cinema!
    cleanupOpenSLES();
    return 0;
}
