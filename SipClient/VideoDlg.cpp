// VideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SipClient.h"
#include "VideoDlg.h"
#include "afxdialogex.h"
#include "StreamManager\StreamManager.h"

using namespace bsm;
using namespace bsm_utils;

stream_manager* g_ps_stream_fifo;
stream_manager* g_h264_es_stream_fifo;
stream_manager* g_rgb24_stram_fifo;

void init_steam_fifo()
{

    g_ps_stream_fifo = new stream_manager();
    g_h264_es_stream_fifo = new stream_manager();
    g_rgb24_stram_fifo = new stream_manager();

    if(g_ps_stream_fifo)
    { 
        g_ps_stream_fifo->set_capacity_size(2 * 1024 * 1024);
    }

    if (g_h264_es_stream_fifo)
    {
        g_h264_es_stream_fifo->set_capacity_size(2 * 1024 * 1024);
    }
    
    if (g_rgb24_stram_fifo)
    {
        g_rgb24_stram_fifo->set_capacity_size(2 * 1024 * 1024);
    }
}


//callback functions. 

/**
*   callback function, used for rtpreceiver.
*/

int callback_push_ps_stream(void *opaque, uint8_t *buf, int data_length)
{
    int write_data_length = 0;
    int read_data_length = 0;
    unsigned char buffer[100 * 1024] = { 0 };

    //write_data_length = stream_manager::get_instance()->push_data(buf, data_length);
    write_data_length = g_ps_stream_fifo->push_data(buf, data_length);
    if (0 < write_data_length)
    {
        //LOG("write data, data_length=%d\n", write_data_length);
        //read_data_length = stream_manager::get_instance()->pull_data(NULL, buffer, write_data_length);
        //write_media_data_to_file("E://callback_tmp1.ps", buffer, read_data_length);
    }
    return write_data_length;
}

/**
*    callback function, used for bsm_demuxer, which use ffmpeg.
*/
int callback_pull_ps_stream_dexuxer(void *opaque, uint8_t *buf, int buf_size)
{
    int recv_date_length = 0;
    while (recv_date_length != buf_size)
    {
        recv_date_length = g_ps_stream_fifo->pull_data(NULL, buf, buf_size);
        //LOG("src stream_manager don't have enought data, callback will waite a moment.\n");
    }
    LOG("receive data success, receive size = %d.\n", recv_date_length);

    return recv_date_length;
}

int callback_push_es_video_stream(void *opaque, uint8_t *data, int data_length)
{
#if 1
    int write_data_length = 0;
    int read_data_length = 0;

    write_data_length = g_h264_es_stream_fifo->push_data(data, data_length);
    if (0 < write_data_length)
    {
        //LOG("write data, data_length=%d\n", write_data_length);
    }
    return write_data_length;
#endif
#if 0
    char* file_name = "E://28181_demuxer_callback_stream.h264";
    FILE* p_file = NULL;
    int write_data_size = 0;
    if (data != NULL && data_length > 0)
    {
        if (NULL == p_file && strlen(file_name) > 0)
        {
            ::fopen_s(&p_file, file_name, "ab+");
        }

        if (p_file != NULL)
        {
            write_data_size = ::fwrite(data, data_length, 1, p_file);
            ::fclose(p_file);
            p_file = NULL;
        }
    }
    return write_data_size;
#endif
}

/**
*    callback function, used for h264_decoder, which use ffmpeg.
*/
int callback_pull_h264_stream(void *opaque, unsigned char *buf, int buf_size)
{
    int recv_date_length = 0;
    while (recv_date_length != buf_size)
    {
        recv_date_length = g_h264_es_stream_fifo->pull_data(NULL, buf, buf_size);
        //LOG("src stream_manager don't have enought data, callback will waite a moment.\n");
    }
    LOG("receive data success, receive size = %d.\n", recv_date_length);

    return recv_date_length;
}

// CVideoDlg dialog

IMPLEMENT_DYNAMIC(CVideoDlg, CDialogEx)

CVideoDlg::CVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIDEODLG, pParent)
{
    m_pMediaSession = NULL;
    m_stream_buffer = (unsigned char*)malloc(STREAM_BUFFER_SIZE);

    CRtpReceiver::setup_callback_function(callback_push_ps_stream, NULL, NULL, NULL);
    m_p_rtp_receiver = new CRtpReceiver();
    
    bsm_demuxer::setup_callback_function(callback_pull_ps_stream_dexuxer, callback_push_es_video_stream, NULL);
    m_pDemux = new bsm_demuxer();

    m_h264_decoder = new h264_decoder();
    m_h264_decoder->setup_callback_function(callback_pull_h264_stream);

    init_steam_fifo();

    //m_pDemux2 = new bsm_demuxer2();
}

CVideoDlg::~CVideoDlg()
{
    if (m_stream_buffer)
    {
        free(m_stream_buffer);
    }

    if (m_pDemux)
    {
        delete m_pDemux;
    }
}

void CVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoDlg message handlers

void write_media_data_to_file(char* file_name, void* pLog, int nLen)
{
    FILE* m_pLogFile = NULL;
    if (pLog != NULL && nLen > 0)
    {
        if (NULL == m_pLogFile && strlen(file_name) > 0)
        {
            ::fopen_s(&m_pLogFile, file_name, "ab+");
        }

        if (m_pLogFile != NULL)
        {
            ::fwrite(pLog, nLen, 1, m_pLogFile);
            ::fclose(m_pLogFile);
            m_pLogFile = NULL;
        }
    }
}

void CVideoDlg::PlayThreadProc(void* pParam)
{
    if(pParam)
    {
        CVideoDlg* pThis = (CVideoDlg*)pParam;
        pThis->Play();
    }
}

void CVideoDlg::ps_packet_demuxer_proc(void* pParam)
{
    if (pParam)
    {
        CVideoDlg* pThis = (CVideoDlg*)pParam;
        pThis->demux_ps_packet();
    }
}

bool CVideoDlg::StartPlay()
{
    m_p_rtp_receiver->set_cleint_ip("192.168.2.102");
    m_p_rtp_receiver->start_proc();

    m_bps_packet_demuxer_thread_runing = true;
    m_ps_packet_demuxer_handle = (HANDLE)_beginthread(ps_packet_demuxer_proc, 0, (void*)this);
    if (0 == m_ps_packet_demuxer_handle)
    {
        LOG("ps packet demuxer thread start failure.\n");
        return false;
    }

    m_playThreadHandle = (HANDLE)_beginthread(PlayThreadProc, 0, (void*)this);
    if (0 == m_playThreadHandle)
    {
        //Ïß³ÌÆô¶¯Ê§°Ü
        return false;
    }
    m_bplayThreadRuning = true;
    return true;
}


bool CVideoDlg::StopPlay()
{
    m_bplayThreadRuning = false;
    return false;
}


char* CVideoDlg::getSdpInfo()
{
    if (NULL != m_p_rtp_receiver)
    {
        return m_p_rtp_receiver->get_sdp_info();
    }
    else
    {
        return nullptr;
    }
}



int CVideoDlg::Play()
{
    /**
    *   use CDemux
    */
#if 1
    //m_pDemux->set_output_es_video_file("E://dialog_mediaplay.h264");

    unsigned char* media_stream_buffer = (unsigned char*)malloc(4 * 1024 * 1024);

    while (m_bplayThreadRuning)
    {
        if (m_h264_decoder)
        {
            if (m_h264_decoder->get_rgb24_frame(media_stream_buffer, 480, 320))
            {

            }
        }
    }
    return 0;
#endif

    /**
    *   use CDemux2
    */

#if 0
    unsigned char stream_buffer[100 * 1024];
    int ps_packet_length = 0;
    //m_pDemux2->setup_dst_es_video_file("E://success_data//tmp1.h264");

    //CStreamManager::get_instance()->get_a_ps_packet(stream_buffer, &ps_packet_length);
    m_pDemux2->deal_ps_packet(stream_buffer, ps_packet_length);
    return 0;
#endif
}

void CVideoDlg::demux_ps_packet()
{
    while (m_bps_packet_demuxer_thread_runing)
    {
        if (m_pDemux)
        {
            m_pDemux->demux_ps_to_es_network();
        }
    }
}

void CVideoDlg::gdi_render()
{
    //HDC hdc = GetDC(hwnd);

    //RECT rect;
    //GetWindowRect(hwnd, &rect);
    //int screen_w = rect.right - rect.left;
    //int screen_h = rect.bottom - rect.top;

    //int pixel_w = 1920, pixel_h = 1080;

    ////BMP Header
    //BITMAPINFO bmphdr = { 0 };
    //DWORD dwBmpHdr = sizeof(BITMAPINFO);
    ////24bit
    //bmphdr.bmiHeader.biBitCount = 24;
    //bmphdr.bmiHeader.biClrImportant = 0;
    //bmphdr.bmiHeader.biSize = dwBmpHdr;
    //bmphdr.bmiHeader.biSizeImage = 0;
    //bmphdr.bmiHeader.biWidth = pixel_w;
    ////Notice: BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
    ////So we must set reverse biHeight to show image correctly.
    //bmphdr.bmiHeader.biHeight = -pixel_h;
    //bmphdr.bmiHeader.biXPelsPerMeter = 0;
    //bmphdr.bmiHeader.biYPelsPerMeter = 0;
    //bmphdr.bmiHeader.biClrUsed = 0;
    //bmphdr.bmiHeader.biPlanes = 1;
    //bmphdr.bmiHeader.biCompression = BI_RGB;

    ////Draw data
    //int nResult = StretchDIBits(hdc,
    //    0, 0,
    //    screen_w, screen_h,
    //    0, 0,
    //    pixel_w, pixel_h,
    //    buffer,
    //    &bmphdr,
    //    DIB_RGB_COLORS,
    //    SRCCOPY);

    //ReleaseDC(hwnd, hdc);
}
