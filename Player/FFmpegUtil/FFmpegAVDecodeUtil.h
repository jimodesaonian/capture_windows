#pragma once
#include <vector>
using namespace std;
extern "C"
{
# pragma warning (disable:4819)
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

#ifdef WIN32
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
	//#pragma comment(lib,"fpostproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

#endif
}

namespace YXLFFmpegStreamUtil
{
	class AVDecodeUtil
	{
	public:
		AVDecodeUtil();
		~AVDecodeUtil();
		bool setVideoDecodeInfo(AVStream *pSrcStream, AVPixelFormat dstFormat, int width, int height);
		bool setAudioDecodeInfo(AVStream *pSrcStream, AVSampleFormat dstFormat, int bitRates);
		int decodePacket(AVPacket *pPacket, vector<char> *pVideoData, vector<char> *pAudioData);
	private:
		AVFrame *m_pDstFrame;
		AVCodec *m_pCodec;
		AVCodecContext *m_pCodecContext;
		SwsContext *m_pSwsContext;
		SwrContext *m_pSwrContext;
		vector<char> m_dstBuffer;
		AVMediaType m_streamType;
	};
}
