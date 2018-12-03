#pragma once

#include "MediaSession\MediaSession.h"
#include "StreamManager\StreamManager.h"
#include "video_decoder\Demuxer.h"
#include "video_decoder\Demuxer2.h"
#include "video_decoder\h264_decoder.h"
#include "RtpReceiver\RtpReceiver.h"

#define STREAM_BUFFER_SIZE (8 * 1024 * 1024)

// CVideoDlg dialog
using namespace bsm;
using namespace bsm_video_decoder;

class CVideoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoDlg)

public:
	CVideoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEODLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
    unsigned char* m_ScrBuf[4] = { 0 };     //video buffer

    CMediaSession* m_pMediaSession;
    CRtpReceiver* m_p_rtp_receiver;

    bsm_demuxer* m_pDemux;
    bsm_demuxer2* m_pDemux2;
    h264_decoder * m_h264_decoder;
    unsigned char* m_stream_buffer;
    int m_stream_buffer_capacity;
    int m_current_frame_size;

    int m_video_dialog_width;
    int m_video_dialog_height;


public:
    void set_windows_size(int width, int height);

    static void PlayThreadProc(void* pParam);   //thread function
    HANDLE m_playThreadHandle;                  //thread handle
    bool m_bplayThreadRuning;                   //thread running state

    static void ps_packet_demuxer_proc(void* pParam);   //thread function
    HANDLE m_ps_packet_demuxer_handle;                  //thread handle
    bool m_bps_packet_demuxer_thread_runing;            //thread running state

    static void h264_decode_proc(void* pParam);   //thread function
    HANDLE m_h264_decoder_handle;                  //thread handle
    bool m_h264_decoder_thread_runing;            //thread running state

    bool StartPlay();
    bool StopPlay();
    int Play();
    void demux_ps_packet();
    void decode_h264_data();

    void gdi_render();
    char* getSdpInfo();
};
