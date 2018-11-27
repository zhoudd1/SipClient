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

bool CVideoDlg::StartPlay()
{
    m_p_rtp_receiver->set_cleint_ip("192.168.2.102");
    m_p_rtp_receiver->start_proc();

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

    if (m_pDemux)
    {
        m_pDemux->demux_ps_to_es_network();
    }

    while (m_bplayThreadRuning)
    {
        
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
