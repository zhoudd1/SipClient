// VideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SipClient.h"
#include "VideoDlg.h"
#include "afxdialogex.h"


// CVideoDlg dialog

int get_network_stream(void *opaque, uint8_t *buf, int buf_size)
{
    CStreamManager* p_stream_manager = CStreamManager::get_instance();
    int length = 0;

    if (p_stream_manager)
    {
        while (1)
        {
            length = p_stream_manager->read_data(NULL, buf, buf_size);
            if (length == buf_size)
            {
                break;
                printf("success");
            }
        }

    }
    return length;
}

int callback_get_ps_stream(void *opaque, uint8_t *buf, int data_length)
{
    int write_data_length = 0;
    int read_data_length = 0;
    unsigned char buffer[100 * 1024] = { 0 };
    write_data_length = CStreamManager::get_instance()->write_data(buf, data_length);
    if (0 < write_data_length)
    {
        LOG("write data, data_length=%d\n", write_data_length);
        //read_data_length = CStreamManager::get_instance()->read_data(NULL, buffer, write_data_length);
        //write_media_data_to_file("E://callback_tmp1.ps", buffer, write_data_length);
    }
    return write_data_length;
}

IMPLEMENT_DYNAMIC(CVideoDlg, CDialogEx)

CVideoDlg::CVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIDEODLG, pParent)
{
    m_pMediaSession = NULL;
    m_stream_buffer = (unsigned char*)malloc(STREAM_BUFFER_SIZE);

    CRtpReceiver::setup_callback_function(callback_get_ps_stream, NULL, NULL, NULL);
    m_p_rtp_receiver = new CRtpReceiver();
    
    CDemuxer::setup_callback_function(get_network_stream);
    m_pDemux = new CDemuxer();

    m_pDemux2 = new CDemuxer2();
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
    //m_pMediaSession = new CMediaSession();
    //m_pMediaSession->StartProc();
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
#if 0
    m_pDemux->set_output_es_video_file("E://dialog_mediaplay.h264");

    while (m_bplayThreadRuning)
    {

        if (m_pDemux)
        {
            m_pDemux->demux_ps_to_es_network();
        }
    }
    return 0;
#endif

    /**
    *   use CDemux2
    */
    unsigned char stream_buffer[100 * 1024];
    int ps_packet_length = 0;
#if 1
    m_pDemux2->setup_dst_es_video_file("E://success_data//tmp1.h264");

    //CStreamManager::get_instance()->get_a_ps_packet(stream_buffer, &ps_packet_length);
    m_pDemux2->deal_ps_packet(stream_buffer, ps_packet_length);
    return 0;
#endif
}
