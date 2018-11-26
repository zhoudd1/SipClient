#include "MediaSession.h"

CMediaSession::CMediaSession()
{
}

CMediaSession::~CMediaSession()
{}


char* CMediaSession::getFrame()
{
    return 0;
}

char* CMediaSession::getSdpInfo()
{
    return m_rtpReceiver.get_sdp_info();
}

int CMediaSession::StartProc()
{
    
    return m_rtpReceiver.start_proc();
}

void CMediaSession::StopProc()
{
    return m_rtpReceiver.stop_proc();
}

unsigned short CMediaSession::getMediaPort()
{
    return m_rtpReceiver.get_media_port();
}
