#pragma once
#include "FFmpegAVDecodeUtil.h"
namespace YXLFFmpegStreamUtil
{

	class InputStream
	{
	public:
		InputStream();
		~InputStream();
		int openInputFile(const char*filename, AVPixelFormat dstVideoFormat, int dstWidth, int dstHeight, AVSampleFormat dstAudioFormat, int dstRate);
		//int openOutputFile(const char*filename);
		bool readVideoFrame(vector<char> *pVideoData,vector<char> *pAudioData, AVMediaType *pMediaType);
		int getVideoFrameWidth();
		int getVideoFrameHeight();
	private:
		AVFormatContext *m_pAVFormatContext;
		struct StreamDecoder
		{
			AVMediaType mediaType;
			AVDecodeUtil decodeUtil;
		};
		vector<StreamDecoder*> pStreamDecoders;
		int m_currentVideoIndex;
		int m_currentAudioIndex;
	};
}