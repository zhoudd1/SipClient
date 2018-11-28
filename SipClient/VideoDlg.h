#pragma once

#include "MediaSession\MediaSession.h"
#include "StreamManager\StreamManager.h"
#include "video_decoder\Demuxer.h"
#include "video_decoder\Demuxer2.h"
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
    unsigned char* m_stream_buffer;


public:
    static void PlayThreadProc(void* pParam);   //�̺߳���
    HANDLE m_playThreadHandle;                  //�߳̾��
    bool m_bplayThreadRuning;                   //�߳�����״̬

    bool StartPlay();
    bool StopPlay();
    int Play();

    void gdi_render();
    char* getSdpInfo();
};
