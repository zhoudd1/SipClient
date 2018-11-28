#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/mathematics.h>
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/mathematics.h>
#ifdef __cplusplus
};
#endif
#endif

#include "h264_decoder.h"
#include "utils/logger.h"

namespace bsm {
namespace bsm_video_decoder {

        h264_decoder::h264_decoder(int stream_buffer_size)
        {
            m_stream_buffer = (unsigned char*)malloc(stream_buffer_size);
            m_stream_buffer_data_header = m_stream_buffer;
        }

        bool h264_decoder::find_next_hx_str(unsigned char* source, int source_length, unsigned char* seed, int seed_length, int* position)
        {
            if (!source || !seed)
            {
                return false;
            }

            unsigned char* pHeader = source;
            int total_length = source_length;
            int processed_length = 0;

            while (total_length - processed_length >= seed_length)
            {
                for (int i = 0; i < seed_length && (pHeader[i] == seed[i]); i++)
                {
                    if (seed_length - 1 == i)
                    {
                        *position = processed_length;
                        return true;
                    }
                }

                processed_length++;
                pHeader = source + processed_length;
            }

            return false;
        }

        bool h264_decoder::find_next_es_packet(unsigned char* source, int source_length, int* es_packet_start_point, int* es_packet_length)
        {
            if (!source)
            {
                return false;
            }

            int _h264_packet_start_point = 0;
            int _h264_packet_end_point = 0;

            unsigned char h264_packet_start_code_32bit[4];
            h264_packet_start_code_32bit[0] = 0x00;
            h264_packet_start_code_32bit[1] = 0x00;
            h264_packet_start_code_32bit[2] = 0x00;
            h264_packet_start_code_32bit[3] = 0x01;

            unsigned char h264_packet_start_code_24bit[4];
            h264_packet_start_code_24bit[0] = 0x00;
            h264_packet_start_code_24bit[1] = 0x00;
            h264_packet_start_code_24bit[2] = 0x01;

            // find nalu packet, when prefix is '00 00 00 01'.
            if (find_next_hx_str(source, source_length, h264_packet_start_code_32bit, 4, &_h264_packet_start_point))
            {
                if (find_next_hx_str(source + 4, source_length - 4, h264_packet_start_code_32bit, 4, &_h264_packet_end_point))
                {
                    *es_packet_start_point = _h264_packet_start_point;
                    *es_packet_length = (_h264_packet_end_point - _h264_packet_start_point) + 4;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        bool h264_decoder::get_rgb24_frame(unsigned char* pframe, int width, int hight)
        {
            int es_packet_start_point;
            int es_packet_length;

            unsigned char *tmp_stream_buffer = (unsigned char*)malloc(m_stream_buffer_size);
            if (tmp_stream_buffer)
            {
                memset(tmp_stream_buffer, 0x00, m_stream_buffer_size);
            }

            do{
                if (find_next_es_packet(m_stream_buffer_data_header, m_stream_buffer_data_size, &es_packet_start_point, &es_packet_length))
                {
                    memcpy(tmp_stream_buffer, m_stream_buffer_data_header + es_packet_start_point, es_packet_length);
                    m_stream_buffer_data_header += es_packet_length;

                    //use ffmpeg deal h264 nalu, which is store in tmp_stream_buffer, now.
                }
                else
                {
                    if (0 <= m_stream_buffer_data_size)
                    {
                        //fill m_stream_buffer.
                        if (0 < m_stream_buffer_data_size)
                        {
                            memcpy(tmp_stream_buffer, m_stream_buffer_data_header, m_stream_buffer_data_size);
                            memset(m_stream_buffer, 0x00, m_stream_buffer_size);

                            memcpy(m_stream_buffer, tmp_stream_buffer, m_stream_buffer_data_size);

                            //use callback function get data to fill m_stream_buffer.
                            if (NULL != m_callback_pull_h264_stream)
                            {
                                m_callback_pull_h264_stream(NULL, 
                                    m_stream_buffer+ m_stream_buffer_data_size,
                                    m_stream_buffer_size - m_stream_buffer_data_size);
                            }

                            m_stream_buffer_data_header = m_stream_buffer;
                            m_stream_buffer_data_size = m_stream_buffer_size;
                        }
                        else
                        {
                            memset(m_stream_buffer, 0x00, m_stream_buffer_size);
                            //use callback function get data to fill m_stream_buffer.
                            if (NULL != m_callback_pull_h264_stream)
                            {
                                m_callback_pull_h264_stream(NULL, m_stream_buffer, m_stream_buffer_size);
                            }
                        }

                        m_stream_buffer_data_header = m_stream_buffer;
                    }
                    else if (m_stream_buffer_size == m_stream_buffer_data_size)
                    {
                        LOG("stream buffer in h264 decoder is too small or nalu data is wrong.\n");
                        pframe = NULL;
                        return false;
                    }
                    else
                    {
                        LOG("error, m_stream_buffer_data_size > m_stream_buffer_size.\n");
                        pframe = NULL;
                        return false;
                    }
                }
            }while (true);

            if (tmp_stream_buffer)
            {
                free(tmp_stream_buffer);
                tmp_stream_buffer = NULL;
            }
            return true;
        }

        bool h264_decoder::get_yuv420p_frame(unsigned char* pframe, int width, int hight)
        {
            return true;
        }


        void h264_decoder::set_stream_buffer_size(int size)
        {
            if (m_stream_buffer)
            {
                free(m_stream_buffer);
                m_stream_buffer = NULL;
            }

            m_stream_buffer_size = size;
            m_stream_buffer = (unsigned char*)malloc(m_stream_buffer_size);
            m_stream_buffer_data_header = m_stream_buffer;
        }

        int h264_decoder::get_stream_buffer_size()
        {
            return m_stream_buffer_size;
        }

        void h264_decoder::setup_callback_function(callback_pull_h264_stream callback_func)
        {
            m_callback_pull_h264_stream = callback_func;
        }
}
}