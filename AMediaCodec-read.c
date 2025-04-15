#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jni.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>

#define AUDIO_INBUF_SIZE 2048

void decode(const char *input_filename) {
    // Initialize media extractor
    AMediaExtractor *extractor = AMediaExtractor_new();
    int fd = open(input_filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return;
    }
    
    AMediaExtractor_setDataSourceFd(extractor, fd, 0, LONG_MAX);
    close(fd);

    // Find the audio track
    size_t num_tracks = AMediaExtractor_getTrackCount(extractor);
    AMediaFormat *format = NULL;
    for (size_t i = 0; i < num_tracks; ++i) {
        format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *mime;
        if (AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime) &&
            strncmp(mime, "audio/", 6) == 0) {
            AMediaExtractor_selectTrack(extractor, i);
            break;
        }
        AMediaFormat_delete(format);
        format = NULL;
    }

    if (!format) {
        fprintf(stderr, "No audio track found in file '%s'\n", input_filename);
        AMediaExtractor_delete(extractor);
        return;
    }

    // Create codec
    const char *mime;
    AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime);
    AMediaCodec *codec = AMediaCodec_createDecoderByType(mime);
    AMediaCodec_configure(codec, format, NULL, NULL, 0);
    AMediaCodec_start(codec);
    AMediaFormat_delete(format);

    // Decode
    ssize_t bufferIndex = -1;
    while (1) {
        // Read data from extractor
        bufferIndex = AMediaCodec_dequeueInputBuffer(codec, 2000);
        if (bufferIndex >= 0) {
            size_t bufferSize;
            uint8_t *buffer = AMediaCodec_getInputBuffer(codec, bufferIndex, &bufferSize);
            ssize_t sampleSize = AMediaExtractor_readSampleData(extractor, buffer, bufferSize);

            if (sampleSize < 0) {
                // End of stream
                AMediaCodec_queueInputBuffer(codec, bufferIndex, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
                break;
            }

            int64_t presentationTimeUs = AMediaExtractor_getSampleTime(extractor);
            AMediaCodec_queueInputBuffer(codec, bufferIndex, 0, sampleSize, presentationTimeUs, 0);
            AMediaExtractor_advance(extractor);
        }

        // Get output from decoder
        AMediaCodecBufferInfo info;
        bufferIndex = AMediaCodec_dequeueOutputBuffer(codec, &info, 0);
        if (bufferIndex >= 0) {
            size_t bufferSize;
            uint8_t *buffer = AMediaCodec_getOutputBuffer(codec, bufferIndex, &bufferSize);
            fwrite(buffer + info.offset, 1, info.size, stdout);
            AMediaCodec_releaseOutputBuffer(codec, bufferIndex, 0);
        }
    }

    // Cleanup
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    AMediaExtractor_delete(extractor);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }
    decode(argv[1]);
    return 0;
}
