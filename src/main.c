#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

/*--------------------------------*/
/*  HARDCODED DISPLAY RESOLUTION  */
/*--------------------------------*/
#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)
#define SCREEN_BPP (32)
#define SCREEN_BYTES ((SCREEN_WIDTH)*(SCREEN_HEIGHT)*(SCREEN_BPP)/8)

/*-----------------*/
/*  TIME HELPERS   */
/*-----------------*/
static double now_secs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void sleep_until(double target_sec) {
    double remaining = target_sec - now_secs();
    if (remaining <= 0.0)
        return;

    struct timespec ts;
    ts.tv_sec = (time_t)remaining;
    ts.tv_nsec = (long)((remaining-ts.tv_sec) * 1e9);
    nanosleep(&ts, NULL);
}

/*-----------------*/
/*  FRAMEBUFFER    */
/*-----------------*/
static int fb_fd = -1;
static uint8_t* fb_mem = NULL;

static int fb_open(void) {
    // Open framebuffer
    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("open /dev/fb0");
        return -1;
    }
 
    // (OPTIONAL) Verify the kernel agrees we have 1920x1080
    struct fb_var_screeninfo var_info;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info) == 0) {
        if (var_info.xres != SCREEN_WIDTH || var_info.yres != SCREEN_HEIGHT) {
            fprintf(stderr,
                "Warning: framebuffer reports %ux%u, "
                "but we are hardcoded to %ux%u\n",
                var_info.xres, var_info.yres, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
        if (var_info.bits_per_pixel != SCREEN_BPP) {
            fprintf(stderr,
                "Warning: framebuffer reports %u bpp, "
                "but we expect %u bpp (RGB565)\n",
                var_info.bits_per_pixel, SCREEN_BPP);
        }
    }
 
    // Map framebuffer's physical memory into process address space
    fb_mem = mmap(
        NULL, 
        SCREEN_BYTES,
        PROT_READ | PROT_WRITE, 
        MAP_SHARED, 
        fb_fd, 
        0
    );
    if (fb_mem == MAP_FAILED) {
        perror("mmap /dev/fb0");
        return -1;
    }
 
    return 0;
}

static void fb_close(void)
{
    if (fb_mem && fb_mem != MAP_FAILED) {
        memset(fb_mem, 0, SCREEN_BYTES);
        munmap(fb_mem, SCREEN_BYTES);
    }

    if (fb_fd >= 0) {
        close(fb_fd);
    }
}

/*-------------------*/
/*  DECODER CONTEXT  */
/*-------------------*/
typedef struct {
    AVFormatContext*   fmt_ctx;
    AVCodecContext*    codec_ctx;
    struct SwsContext* sws;
    AVFrame*           frame_yuv; /* decoded frame from libavcodec (YUV)  */
    AVFrame*           frame_rgb; /* converted frame ready for framebuffer */
    AVPacket*          pkt;
    int                video_stream_idx;
    double             time_base; /* stream time_base as a double (seconds/tick) */
} PlayerCtx;
 
static int player_open(PlayerCtx *ctx, const char *filepath) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->video_stream_idx = -1;
 
    // Open container
    if (avformat_open_input(&ctx->fmt_ctx, filepath, NULL, NULL) < 0) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return -1;
    }

    if (avformat_find_stream_info(ctx->fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Cannot find stream info\n");
        return -1;
    }
 
    // Find video stream
    for (unsigned i = 0; i < ctx->fmt_ctx->nb_streams; i++) {
        if (ctx->fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ctx->video_stream_idx = (int)i;
            break;
        }
    }

    if (ctx->video_stream_idx < 0) {
        fprintf(stderr, "No video stream found\n");
        return -1;
    }
 
    AVStream* stream = ctx->fmt_ctx->streams[ctx->video_stream_idx];
    AVCodecParameters* codecpar = stream->codecpar;
    ctx->time_base = av_q2d(stream->time_base);
 
    // Open decoder
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec\n");
        return -1;
    }

    ctx->codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->codec_ctx, codecpar);
    if (avcodec_open2(ctx->codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Cannot open codec\n");
        return -1;
    }
 
    // Pixel-format converter: YUV420p → RGB565, scaled to 1920x1080
    ctx->sws = sws_getContext(
        codecpar->width, 
        codecpar->height, 
        ctx->codec_ctx->pix_fmt,
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        AV_PIX_FMT_RGB32,
        SWS_FAST_BILINEAR, 
        NULL, NULL, NULL
    );
    if (!ctx->sws) {
        fprintf(stderr, "Cannot create swscale context\n");
        return -1;
    }
 
    // Allocate frame buffers
    ctx->frame_yuv = av_frame_alloc();
    ctx->frame_rgb = av_frame_alloc();
    if (!ctx->frame_yuv || !ctx->frame_rgb) {
        fprintf(stderr, "Cannot allocate frames\n");
        return -1;
    }
 
    // Back the RGB frame with a real pixel buffer
    int buf_size = av_image_get_buffer_size(
        AV_PIX_FMT_RGB32,
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        1
    );
    uint8_t* buf = av_malloc(buf_size);
    if (!buf) {
        fprintf(stderr, "Cannot allocate RGB buffer\n");
        return -1;
    }
    av_image_fill_arrays(
        ctx->frame_rgb->data, ctx->frame_rgb->linesize,
        buf, 
        AV_PIX_FMT_RGB32,
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        1
    );
 
    ctx->pkt = av_packet_alloc();
    if (!ctx->pkt) {
        fprintf(stderr, "Cannot allocate packet\n");
        return -1;
    }
 
    return 0;
}
 
static void player_close(PlayerCtx *ctx)
{
    if (ctx->frame_rgb) {
        av_free(ctx->frame_rgb->data[0]);
        av_frame_free(&ctx->frame_rgb);
    }
    av_frame_free(&ctx->frame_yuv);
    av_packet_free(&ctx->pkt);
    sws_freeContext(ctx->sws);
    avcodec_free_context(&ctx->codec_ctx);
    avformat_close_input(&ctx->fmt_ctx);
}
 
/* ------------------------------------------------------------------ */
/*  Playback loop                                                       */
/* ------------------------------------------------------------------ */
static void player_run(PlayerCtx *ctx)
{
    double playback_start_wall = -1.0; /* set on first frame */
    double first_pts_sec       =  0.0;
 
    while (av_read_frame(ctx->fmt_ctx, ctx->pkt) >= 0) {
 
        // Skip non-video packets (audio, subtitles, ...)
        if (ctx->pkt->stream_index != ctx->video_stream_idx) {
            av_packet_unref(ctx->pkt);
            continue;
        }
 
        // Send compressed packet to decoder
        if (avcodec_send_packet(ctx->codec_ctx, ctx->pkt) < 0) {
            av_packet_unref(ctx->pkt);
            continue;
        }
 
        // Pull decoded frames (may be 0, 1, or several per packet)
        while (avcodec_receive_frame(ctx->codec_ctx, ctx->frame_yuv) == 0) {
            // Frame timing 
            double pts_sec = ctx->frame_yuv->pts * ctx->time_base;
 
            if (playback_start_wall < 0.0) {
                // Anchor wall-clock to the first PTS
                playback_start_wall = now_secs();
                first_pts_sec       = pts_sec;
            }
 
            double target_wall = playback_start_wall + (pts_sec - first_pts_sec);
            sleep_until(target_wall);
 
            // Convert YUV → RGB565, scale to 1920x1080
            sws_scale(
                ctx->sws,
                (const uint8_t *const *)ctx->frame_yuv->data,
                ctx->frame_yuv->linesize,
                0, 
                ctx->codec_ctx->height,
                ctx->frame_rgb->data,
                ctx->frame_rgb->linesize
            );
 
            // Copy to framebuffer
            memcpy(fb_mem, ctx->frame_rgb->data[0], SCREEN_BYTES);
        }
 
        av_packet_unref(ctx->pkt);
    }
 
    // Flush decoder 
    avcodec_send_packet(ctx->codec_ctx, NULL);
    while (avcodec_receive_frame(ctx->codec_ctx, ctx->frame_yuv) == 0) {
        sws_scale(
            ctx->sws,
            (const uint8_t *const *)ctx->frame_yuv->data,
            ctx->frame_yuv->linesize,
            0, 
            ctx->codec_ctx->height,
            ctx->frame_rgb->data,
            ctx->frame_rgb->linesize
        );
        memcpy(fb_mem, ctx->frame_rgb->data[0], SCREEN_BYTES);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <video.mp4>\n", argv[0]);
        return 1;
    }

    if (fb_open() < 0) {
        fprintf(stderr, "Could not locate framebuffer\n");
        return 1;
    }

    PlayerCtx ctx;
    if (player_open(&ctx, argv[1]) < 0) {
        fb_close();
        return 1;
    }

    player_run(&ctx);

    player_close(&ctx);
    fb_close();

    return 0;
}