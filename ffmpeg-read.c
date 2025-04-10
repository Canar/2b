#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>

void print_error(const char *msg, int err) {
    char errbuf[128];
    av_strerror(err, errbuf, sizeof(errbuf));
    fprintf(stderr, "%s: %s\n", msg, errbuf);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    AVFormatContext *format_ctx = NULL;
    int ret;

    if ((ret = avformat_open_input(&format_ctx, filename, NULL, NULL)) < 0) {
        print_error("Could not open input file", ret);
        return 1;
    }

    if ((ret = avformat_find_stream_info(format_ctx, NULL)) < 0) {
        print_error("Could not find stream information", ret);
        avformat_close_input(&format_ctx);
        return 1;
    }

    AVCodec *codec = NULL;
    AVCodecContext *codec_ctx = NULL;
    int audio_stream_index = -1;

    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }

    if (audio_stream_index == -1) {
        fprintf(stderr, "No audio stream found\n");
        avformat_close_input(&format_ctx);
        return 1;
    }

    AVStream *audio_stream = format_ctx->streams[audio_stream_index];
    codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);

    if (!codec) {
        fprintf(stderr, "Decoder not found\n");
        avformat_close_input(&format_ctx);
        return 1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        avformat_close_input(&format_ctx);
        return 1;
    }

    if ((ret = avcodec_parameters_to_context(codec_ctx, audio_stream->codecpar)) < 0) {
        print_error("Could not copy codec parameters", ret);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return 1;
    }

    if ((ret = avcodec_open2(codec_ctx, codec, NULL)) < 0) {
        print_error("Could not open codec", ret);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return 1;
    }

    AVPacket packet;
    AVFrame *frame = av_frame_alloc();

    while (av_read_frame(format_ctx, &packet) >= 0) {
        if (packet.stream_index == audio_stream_index) {
            ret = avcodec_send_packet(codec_ctx, &packet);
            if (ret < 0) {
                print_error("Error sending a packet for decoding", ret);
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    print_error("Error during decoding", ret);
                    break;
                }

                fwrite(frame->data[0], 1, frame->linesize[0], stdout);
            }
        }

        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}
