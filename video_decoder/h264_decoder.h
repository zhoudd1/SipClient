#ifndef __H264_DECODER_H__
#define __H264_DECODER_H__

#include "image_decoder\bsm_image_encoder.h"

namespace bsm {
namespace bsm_video_decoder {

typedef enum pixel_format
{
    yuv420p = 0,
    yuv422p,
    rgb24,
    rgbp,
    bgr24,
} pixel_format_e;

typedef int(*callback_pull_h264_stream_h264_decoder)(void *opaque, unsigned char *buf, int buf_size);         //input h264 stream

class h264_decoder
{
public:
    h264_decoder(int stream_buffer_size = 1 * 1024 * 1024);
    ~h264_decoder() {}

private:
    /**
    *   description:
    *       find string-hexadecimal, in source string.
    *
    *   parameter:
    *       source,
    *       source_length,
    *       seed,               // dest string.
    *       seed_length,        // dest string length.
    *       position,           // position of dest string in source string.
    *
    *   return:
    */
    bool find_next_hx_str(unsigned char* source, int source_length, unsigned char* seed, int seed_length, int* position);
    bool find_next_es_packet(unsigned char* source, int source_length, int* es_packet_start_point, int* es_packet_length);

    bool decode_h264_to_rgb24(unsigned char* h264_source, unsigned char* rgb24_dest, int width, int hight);
    bool decode_h264_to_yuv420p(unsigned char* h264_source, unsigned char* yuv420p_dest, int width, int hight);

    void write_media_data_to_file(unsigned char *buf, int wrap, int xsize, int ysize,
        char *filename);

public:
    bool get_rgb24_frame(unsigned char* pframe, int width, int hight);
    bool get_yuv420p_frame(unsigned char* pframe, int width, int hight);

    bool get_one_nalu_packet(unsigned char* nalu_packet);

    void set_stream_buffer_size(int size);
    int get_stream_buffer_size();

    void setup_callback_function(callback_pull_h264_stream_h264_decoder m_callback_func);

private:
    unsigned char* m_stream_buffer;
    int m_stream_buffer_size;
    int m_stream_buffer_data_size;
    unsigned char* m_stream_buffer_data_header;

    bsm::image_encoder::bsm_image_encoder m_image_encoder;

    callback_pull_h264_stream_h264_decoder m_callback_pull_h264_stream;
};

}//namespace bsm_video_decoder
}//namespace bsm

#endif
