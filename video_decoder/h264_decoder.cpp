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
            :m_stream_buffer_size(stream_buffer_size),
            m_stream_buffer_data_size(0)
        {
            m_stream_buffer = (unsigned char*)malloc(m_stream_buffer_size);
            memset(m_stream_buffer, 0x00, stream_buffer_size);
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

        bool h264_decoder::decode_h264_to_rgb24(unsigned char* h264_source, unsigned char* rgb24_dest, int width, int hight)
        {

            return true;
        }

        bool h264_decoder::decode_h264_to_yuv420p(unsigned char* h264_source, unsigned char* yuv420p_dest, int width, int hight)
        {
            return true;
        }

        bool h264_decoder::get_rgb24_frame(unsigned char* pframe, int width, int hight)
        {
            AVIOContext *av_io_context_input = NULL;
            AVIOContext *av_io_context_output = NULL;

            AVInputFormat * input_formate = NULL;
            AVOutputFormat *av_output_format = NULL;

            AVFormatContext *av_format_context_input = NULL;
            AVFormatContext *av_formate_context_out_video = NULL;

            AVCodecParserContext *av_codec_praser_context = NULL;
            AVCodecContext	*av_codec_context = NULL;
            AVCodec			*av_codec = NULL;

            AVStream *in_stream_ps = NULL;
            AVStream *out_stream_es;
            AVPacket *av_packet;
            AVFrame	*av_frame;

            int videoindex = -1;

            unsigned char* input_buffer = (unsigned char*)av_malloc(m_stream_buffer_size);

            // input
            av_format_context_input = avformat_alloc_context();
            av_io_context_input = avio_alloc_context(input_buffer, m_stream_buffer_size, 0, NULL, m_callback_pull_h264_stream, NULL, NULL);
            if (av_io_context_input == NULL)
            {
                return false;
            }

            av_format_context_input->pb = av_io_context_input;
            av_format_context_input->flags = AVFMT_FLAG_CUSTOM_IO;

            if (avformat_open_input(&av_format_context_input, NULL, NULL, NULL) < 0)
            {
                LOG("Cannot open input h264 stream.\n");
                return false;
            }

            if (avformat_find_stream_info(av_format_context_input, NULL) < 0)
            {
                LOG("Cannot find stream information\n");
                return false;
            }

            for (int i = 0; i<av_format_context_input->nb_streams; i++)
            {
                if (av_format_context_input->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    videoindex = i;
                    break;
                }
            }

            if (videoindex == -1) {
                LOG("Didn't find a video stream.\n");
                return false;
            }

            av_codec = avcodec_find_decoder(av_format_context_input->streams[videoindex]->codecpar->codec_id);
            if (av_codec == NULL)
            {
                LOG("Codec not found.\n");
                return false;
            }

            //av_codec_praser_context = av_parser_init(av_codec->id);
            //if (!av_codec_praser_context)
            //{
            //    LOG("parser not found\n");
            //    return false;
            //}

            av_codec_context = avcodec_alloc_context3(av_codec);
            if (!av_codec_context) 
            {
                LOG("Could not allocate video codec context\n");
                return false;
            }

            if (avcodec_open2(av_codec_context, av_codec, NULL) <0)
            {
                LOG("Could not open codec.\n");
                return false;
            }


            av_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
            av_frame = av_frame_alloc();

            while (av_read_frame(av_format_context_input, av_packet) >= 0)
            {
                if (av_packet->stream_index == videoindex)
                {
                    if (0 == avcodec_send_packet(av_codec_context, av_packet))
                    {
                        if (0 == avcodec_receive_frame(av_codec_context, av_frame))
                        {
                            LOG("success, get a frame.\n");
                            LOG("frame->linesize[0]: %d, frame->width:%d, frame->height:%d",
                                av_frame->linesize[0], av_frame->width, av_frame->height);
                        }
                        else
                        {
                            LOG("error, when avcodec_receive_frame().\n");
                        }
                    }
                    else
                    {
                        LOG("error, when avcodec_send_packet().\n");
                    }
                }      
            }


            av_free(input_buffer);
            av_packet_unref(av_packet);
            av_frame_free(&av_frame);
            avio_context_free(&av_io_context_input);
            avformat_free_context(av_format_context_input);

            //sws_freeContext(img_convert_ctx);

            return true;
        }

        bool h264_decoder::get_yuv420p_frame(unsigned char* pframe, int width, int hight)
        {
            return true;
        }

        bool h264_decoder::get_one_nalu_packet(unsigned char* nalu_packet)
        {
            int es_packet_start_point;
            int es_packet_length;

            int callback_get_data_length;

            unsigned char *tmp_stream_buffer = (unsigned char*)malloc(m_stream_buffer_size);
            if (tmp_stream_buffer)
            {
                memset(tmp_stream_buffer, 0x00, m_stream_buffer_size);
            }

            do {
                if (find_next_es_packet(m_stream_buffer_data_header, m_stream_buffer_data_size, &es_packet_start_point, &es_packet_length))
                {
                    memcpy(tmp_stream_buffer, m_stream_buffer_data_header + es_packet_start_point, es_packet_length);
                    m_stream_buffer_data_header += es_packet_length;

                    //use ffmpeg deal h264 nalu, which is store in tmp_stream_buffer, now.
                    if (nalu_packet)
                    {
                        memcpy(nalu_packet, tmp_stream_buffer, es_packet_length);
                    }
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
                                callback_get_data_length = m_callback_pull_h264_stream(NULL,
                                    m_stream_buffer + m_stream_buffer_data_size,
                                    m_stream_buffer_size - m_stream_buffer_data_size);

                                if (0 < callback_get_data_length)
                                {
                                    m_stream_buffer_data_size += callback_get_data_length;
                                }

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
                                callback_get_data_length = m_callback_pull_h264_stream(NULL, m_stream_buffer, m_stream_buffer_size);
                                if (0 < callback_get_data_length)
                                {
                                    m_stream_buffer_data_size += callback_get_data_length;
                                }
                            }
                        }

                        m_stream_buffer_data_header = m_stream_buffer;
                    }
                    else if (m_stream_buffer_size == m_stream_buffer_data_size)
                    {
                        LOG("stream buffer in h264 decoder is too small or nalu data is wrong.\n");
                        nalu_packet = NULL;
                        return false;
                    }
                    else
                    {
                        LOG("error, m_stream_buffer_data_size > m_stream_buffer_size.\n");
                        nalu_packet = NULL;
                        return false;
                    }
                }
            } while (true);

            if (tmp_stream_buffer)
            {
                free(tmp_stream_buffer);
                tmp_stream_buffer = NULL;
            }
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
            m_stream_buffer_data_size = 0;
            m_stream_buffer = (unsigned char*)malloc(m_stream_buffer_size);
            memset(m_stream_buffer, 0x00, m_stream_buffer_size);
            m_stream_buffer_data_header = m_stream_buffer;
        }

        int h264_decoder::get_stream_buffer_size()
        {
            return m_stream_buffer_size;
        }

        void h264_decoder::setup_callback_function(callback_pull_h264_stream_h264_decoder callback_func)
        {
            m_callback_pull_h264_stream = callback_func;
        }
}
}