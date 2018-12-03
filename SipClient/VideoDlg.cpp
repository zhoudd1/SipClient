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

int callback_push_ps_stream_rtpreceiver(void *opaque, uint8_t *buf, int data_length)
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
    //LOG("receive data success, receive size = %d.\n", recv_date_length);

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
    //LOG("receive data success, receive size = %d.\n", recv_date_length);

    return recv_date_length;
}


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

// CVideoDlg dialog

IMPLEMENT_DYNAMIC(CVideoDlg, CDialogEx)

CVideoDlg::CVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIDEODLG, pParent)
{
    m_pMediaSession = NULL;
    m_stream_buffer = (unsigned char*)malloc(STREAM_BUFFER_SIZE);
    m_stream_buffer_capacity = STREAM_BUFFER_SIZE;

    m_p_rtp_receiver = new CRtpReceiver();
    m_p_rtp_receiver->setup_callback_function(callback_push_ps_stream_rtpreceiver, NULL, NULL, NULL);
    
    m_pDemux = new bsm_demuxer();
    m_pDemux->setup_callback_function(callback_pull_ps_stream_dexuxer, callback_push_es_video_stream, NULL);

    m_h264_decoder = new h264_decoder();
    m_h264_decoder->setup_callback_function(callback_pull_h264_stream);

    init_steam_fifo();
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

void CVideoDlg::set_windows_size(int width, int height)
{
    RECT old_rect;
    GetWindowRect(&old_rect);

    m_video_dialog_width = width;
    m_video_dialog_height = height;

    RECT new_rect;
    new_rect.top = 0;
    new_rect.left = 0;
    new_rect.right = new_rect.left + m_video_dialog_width;
    new_rect.bottom = new_rect.top + m_video_dialog_height;
    

    MoveWindow(&new_rect);
}

void CVideoDlg::PlayThreadProc(void* pParam)
{
    if(pParam)
    {
        CVideoDlg* p_video_dialog = (CVideoDlg*)pParam;
        p_video_dialog->Play();
    }
}

void CVideoDlg::ps_packet_demuxer_proc(void* pParam)
{
    if (pParam)
    {
        CVideoDlg* p_video_dialog = (CVideoDlg*)pParam;
        p_video_dialog->demux_ps_packet();
    }
}

void CVideoDlg::h264_decode_proc(void* pParam)
{
    if (pParam)
    {
        CVideoDlg* p_video_dialog = (CVideoDlg*)pParam;
        p_video_dialog->decode_h264_data();
    }
}

bool CVideoDlg::StartPlay()
{
    set_windows_size(640, 480);

    m_p_rtp_receiver->set_cleint_ip("192.168.2.102");
    m_p_rtp_receiver->start_proc();

    //start ps demux thread
    m_bps_packet_demuxer_thread_runing = true;
    m_ps_packet_demuxer_handle = (HANDLE)_beginthread(ps_packet_demuxer_proc, 0, (void*)this);
    if (0 == m_ps_packet_demuxer_handle)
    {
        LOG("ps packet demuxer thread start failure.\n");
        return false;
    }
    else
    {
        LOG("ps packet demuxer thread start success.\n");
    }

    //strat h264 decode thread
    m_h264_decoder_thread_runing = true;
    m_h264_decoder_handle = (HANDLE)_beginthread(h264_decode_proc, 0, (void*)this);
    if(0 == m_h264_decoder_handle)
    {
        LOG("h264 decode thread start failure.\n");
        return false;
    }
    else
    {
        LOG("h264 decode thread start success.\n");
    }

    m_bplayThreadRuning = true;
    m_playThreadHandle = (HANDLE)_beginthread(PlayThreadProc, 0, (void*)this);
    if (0 == m_playThreadHandle)
    {
        LOG("render thread start failure.\n");
        return false;
    }
    else
    {
        LOG("render thread start success.\n");
    }

    return true;
}


bool CVideoDlg::StopPlay()
{
    m_bps_packet_demuxer_thread_runing = false;
    m_h264_decoder_thread_runing = false;
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

    while (m_bplayThreadRuning)
    {
        gdi_render();
        //Sleep(40);
    }

    LOG("render thread stop.\n");
    return 0;
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
    LOG("ps packet demuxer thread stop.\n");
}

void CVideoDlg::decode_h264_data()
{
    while (m_h264_decoder_thread_runing)
    {
        if (m_h264_decoder)
        {
            if (m_h264_decoder->get_rgb24_frame(m_stream_buffer, m_stream_buffer_capacity, &m_current_frame_size, m_video_dialog_width, m_video_dialog_height))
            {

            }
        }
    }
    LOG("h264 decode thread stop.\n");
}

void CVideoDlg::gdi_render()
{
    HDC hdc;
    hdc = ::GetDC(this->GetSafeHwnd());

    int pixel_w = m_video_dialog_width, pixel_h = m_video_dialog_height;

    //BMP Header
    BITMAPINFO bmphdr = { 0 };
    DWORD dwBmpHdr = sizeof(BITMAPINFO);
    //24bit
    bmphdr.bmiHeader.biBitCount = 24;
    bmphdr.bmiHeader.biClrImportant = 0;
    bmphdr.bmiHeader.biSize = dwBmpHdr;
    bmphdr.bmiHeader.biSizeImage = 0;
    bmphdr.bmiHeader.biWidth = pixel_w;
    //Notice: BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
    //So we must set reverse biHeight to show image correctly.
    bmphdr.bmiHeader.biHeight = -pixel_h;
    bmphdr.bmiHeader.biXPelsPerMeter = 0;
    bmphdr.bmiHeader.biYPelsPerMeter = 0;
    bmphdr.bmiHeader.biClrUsed = 0;
    bmphdr.bmiHeader.biPlanes = 1;
    bmphdr.bmiHeader.biCompression = BI_RGB;

    //Draw data
    int nResult = StretchDIBits(hdc,
        0, 0,
        pixel_w, pixel_h,
        0, 0,
        pixel_w, pixel_h,
        m_stream_buffer,
        &bmphdr,
        DIB_RGB_COLORS,
        SRCCOPY);

    ::ReleaseDC(m_hWnd, hdc);
}
