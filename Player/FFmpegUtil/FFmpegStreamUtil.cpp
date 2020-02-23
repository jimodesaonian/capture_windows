
#include "FFmpegStreamUtil.h"

namespace YXLFFmpegStreamUtil
{

	InputStream::InputStream()
	{
		m_pAVFormatContext = NULL;
		m_currentAudioIndex = -1;
		m_currentVideoIndex = -1;
	}

	InputStream::~InputStream()
	{
		if (m_pAVFormatContext)
		{
			av_free(m_pAVFormatContext);
		}
		for (int i = 0; i < (int)pStreamDecoders.size(); i++)
		{
			if (pStreamDecoders[i])
			{
				delete pStreamDecoders[i];
			}
		}
		pStreamDecoders.clear();
	}

	int InputStream::openInputFile(const char*filename, AVPixelFormat dstVideoFormat, int dstWidth, int dstHeight, AVSampleFormat dstAudioFormat, int dstRate)
	{
		for (int i = 0; i < (int)pStreamDecoders.size(); i++)
		{
			if (pStreamDecoders[i])
			{
				delete pStreamDecoders[i];
			}
		}
		pStreamDecoders.clear();

		int ret;
		if ((ret = avformat_open_input(&m_pAVFormatContext, filename, NULL, NULL)) < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
			return ret;
		}

		if ((ret = avformat_find_stream_info(m_pAVFormatContext, NULL)) < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
			return ret;
		}
		for (unsigned int i = 0; i < m_pAVFormatContext->nb_streams; i++) {
		
			AVStream* stream = m_pAVFormatContext->streams[i];
			//stream->codecpar
			AVCodecParameters *pParameters = stream->codecpar;

			if (pParameters->codec_type == AVMEDIA_TYPE_VIDEO) 
			{
				StreamDecoder *pStreamDecoder = new StreamDecoder;
				pStreamDecoder->mediaType = AVMEDIA_TYPE_VIDEO;
				pStreamDecoder->decodeUtil.setVideoDecodeInfo(stream, dstVideoFormat, dstWidth, dstHeight);
				pStreamDecoders.push_back(pStreamDecoder);
				m_currentVideoIndex = i;
			}
			else if (pParameters->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				StreamDecoder *pStreamDecoder = new StreamDecoder;
				pStreamDecoder->mediaType = AVMEDIA_TYPE_AUDIO;
				pStreamDecoder->decodeUtil.setAudioDecodeInfo(stream, dstAudioFormat, dstRate);
				pStreamDecoders.push_back(pStreamDecoder);
				m_currentAudioIndex = i;
			}
			else
			{
				pStreamDecoders.push_back(NULL);
			}
		}
	
		av_dump_format(m_pAVFormatContext, 0, filename, 0);
		return 0;
	}
	/*int InputStream::openOutputFile(const char*filename)
	{
		avcodec_register_all();
		AVCodec *pCodec = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_H264);
		avformat_alloc_output_context2(&m_pAVFormatContext, NULL, NULL, filename);
		AVStream *pStream = avformat_new_stream(m_pAVFormatContext, NULL);
		
		AVCodecParameters *pCodecParameters = pStream->codecpar;
		pCodecParameters->codec_id = AVCodecID::AV_CODEC_ID_H264;

		pCodecParameters->codec_type = AVMediaType::AVMEDIA_TYPE_VIDEO;
		pCodecParameters->bit_rate = 1024 * 1024 * 12;
		// resolution must be a multiple of two 
		pCodecParameters->width = 480;
		pCodecParameters->height = 320;
		// frames per second 
		pCodecParameters->sample_aspect_ratio.den = 25;
		pCodecParameters->sample_aspect_ratio.num = 1;
		pCodecParameters->format = AVPixelFormat::AV_PIX_FMT_YUV420P;
		//pCodecParameters-> = 12;

	//	AVCodecParameters *pCodecParameters = 
		AVRational rational;
		rational.den = 1;
		rational.num = 0;
	//	av_stream_set_recommended_encoder_configuration(pStream, "");
	//	av_stream_set_r_frame_rate(pStream, rational);
		if (!m_pAVFormatContext) {
			av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
			return AVERROR_UNKNOWN;
		}

	

		if (!(m_pAVFormatContext->oformat->flags & AVFMT_NOFILE)) {
			int ret = avio_open(&m_pAVFormatContext->pb, filename, AVIO_FLAG_WRITE);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
				return ret;
			}
		}
		// init muxer, write output file header 
		int ret = avformat_write_header(m_pAVFormatContext, NULL);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
			return ret;
		}

		return 0;
		
	}*/
	bool InputStream::readVideoFrame(vector<char> *pVideoData, vector<char> *pAudioData, AVMediaType *pMediaType)
	{
		int s = 1;
		AVPacket pkt;
		//		AVFrame frm;
		int ret = av_read_frame(m_pAVFormatContext, &pkt);
		if (ret < 0)
		{
			return false;
		}
		if (pkt.stream_index != m_currentAudioIndex&&pkt.stream_index != m_currentVideoIndex)
		{
			return false;
		}

		if (this->pStreamDecoders[pkt.stream_index])
		{
			
			if (pMediaType)
			{
				*pMediaType = this->pStreamDecoders[pkt.stream_index]->mediaType;
			}
			return this->pStreamDecoders[pkt.stream_index]->decodeUtil.decodePacket(&pkt, pVideoData, pAudioData) == 0;
		}
		return false;
	}

	int InputStream::getVideoFrameWidth()
	{
		for (int i = 0; i < (int)m_pAVFormatContext->nb_streams; i++)
		{
			AVStream* stream = m_pAVFormatContext->streams[i];
			//stream->codecpar
			AVCodecParameters *pParameters = stream->codecpar;

			if (pParameters->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				return pParameters->width;
			}
		}
		return -1;
	}
	int InputStream::getVideoFrameHeight()
	{
		for (int i = 0; i < (int)m_pAVFormatContext->nb_streams; i++)
		{
			AVStream* stream = m_pAVFormatContext->streams[i];
			//stream->codecpar
			AVCodecParameters *pParameters = stream->codecpar;

			if (pParameters->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				return pParameters->height;
			}
		}
		return -1;
	}
}

