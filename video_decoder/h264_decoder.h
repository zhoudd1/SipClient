#ifndef __H264_DECODER_H__
#define __H264_DECODER_H__

namespace bsm {
namespace bsm_video_decoder {

        typedef int(*callback_pull_h264_stream)(void *opaque, unsigned char *buf, int buf_size);         //input h264 stream

        class h264_decoder
        {
        public:
            h264_decoder(int stream_buffer_size = 4 * 1024 * 1024);
            ~h264_decoder() {}

        private:
            /**
            *   description:
            *       find string-hexadecimal, in soutce string.
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

        public:
            bool get_rgb24_frame(unsigned char* pframe, int width, int hight);
            bool get_yuv420p_frame(unsigned char* pframe, int width, int hight);

            void set_stream_buffer_size(int size);
            int get_stream_buffer_size();

            void setup_callback_function(callback_pull_h264_stream m_callback_func);

        private:
            unsigned char* m_stream_buffer;
            int m_stream_buffer_size;
            int m_stream_buffer_data_size;
            unsigned char* m_stream_buffer_data_header;

            callback_pull_h264_stream m_callback_pull_h264_stream;
        };

}//namespace bsm_video_decoder
}//namespace bsm

#endif
