#include "FFmpegAVDecodeUtil.h"

namespace YXLFFmpegStreamUtil
{
	AVDecodeUtil::AVDecodeUtil()
	{
		m_pDstFrame = av_frame_alloc();
		m_pCodec = NULL;
		m_pCodecContext = NULL;
		m_pSwsContext = NULL;
		m_pSwrContext = NULL;
	}

	AVDecodeUtil::~AVDecodeUtil()
	{
		av_frame_free(&m_pDstFrame);
		avcodec_free_context(&m_pCodecContext);
		if (m_pSwsContext)
		{
			sws_freeContext(m_pSwsContext);
		}
		if (m_pSwrContext)
		{
			swr_free(&m_pSwrContext);
		}

	}


	bool AVDecodeUtil::setVideoDecodeInfo(AVStream *pSrcStream, AVPixelFormat dstFormat, int width, int height)
	{
		int ret;
		avcodec_free_context(&m_pCodecContext);

		AVCodecParameters *pCodecParameters = pSrcStream->codecpar;

		m_pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
		m_pCodecContext = avcodec_alloc_context3(m_pCodec);

		this->m_streamType = pSrcStream->codecpar->codec_type;
		if (this->m_streamType != AVMEDIA_TYPE_VIDEO)
		{
			return false;
		}

		m_pCodecContext->pix_fmt = (AVPixelFormat)pCodecParameters->format;
		m_pCodecContext->width = pCodecParameters->width;
		m_pCodecContext->height = pCodecParameters->height;
	

		m_pDstFrame->format = dstFormat;
		m_pDstFrame->width = width > 0 ? width : pCodecParameters->width;
		m_pDstFrame->height = height > 0 ? height : pCodecParameters->height;

		int dataSize = av_image_get_buffer_size(dstFormat, m_pDstFrame->width, m_pDstFrame->height, 1);
		m_dstBuffer.resize(dataSize);
		
		av_image_fill_arrays(m_pDstFrame->data, m_pDstFrame->linesize, (uint8_t*)&m_dstBuffer[0], (AVPixelFormat)m_pDstFrame->format, m_pDstFrame->width, m_pDstFrame->height, 1);

		if (m_pSwsContext)
		{
			sws_freeContext(m_pSwsContext);
		}
		m_pSwsContext = sws_getContext(m_pCodecContext->width, m_pCodecContext->height, m_pCodecContext->pix_fmt, m_pDstFrame->width, m_pDstFrame->height, (AVPixelFormat)m_pDstFrame->format, 0, NULL, NULL, NULL);


		ret = avcodec_open2(m_pCodecContext, m_pCodec, NULL);
		return ret == 0;
	}

	bool AVDecodeUtil::setAudioDecodeInfo(AVStream *pSrcStream, AVSampleFormat dstFormat, int bitRates)
	{
		int ret;
		avcodec_free_context(&m_pCodecContext);

		AVCodecParameters *pCodecParameters = pSrcStream->codecpar;

		m_pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
		m_pCodecContext = avcodec_alloc_context3(m_pCodec);

		this->m_streamType = pSrcStream->codecpar->codec_type;
		if (this->m_streamType != AVMEDIA_TYPE_AUDIO)
		{
			return false;
		}


		m_pCodecContext->sample_fmt = (AVSampleFormat)pCodecParameters->format;
		m_pCodecContext->sample_rate = pCodecParameters->sample_rate;
		m_pCodecContext->sample_aspect_ratio = pCodecParameters->sample_aspect_ratio;
		m_pCodecContext->channels = pCodecParameters->channels;
		m_pCodecContext->channel_layout = pCodecParameters->channel_layout;
		m_pCodecContext->bits_per_coded_sample = pCodecParameters->bits_per_raw_sample;


		m_pDstFrame->format = dstFormat;
		m_pDstFrame->channels = 2;
		m_pDstFrame->sample_rate = bitRates;
		if (m_pSwrContext)
		{
			swr_free(&m_pSwrContext);
		}
		int64_t defaultChannelLayout = av_get_default_channel_layout(pCodecParameters->channels);
		m_pSwrContext = swr_alloc_set_opts(NULL, av_get_default_channel_layout(m_pDstFrame->channels), (AVSampleFormat)dstFormat, bitRates, pCodecParameters->channel_layout == 0 ? defaultChannelLayout : pCodecParameters->channel_layout, (AVSampleFormat)pCodecParameters->format, pCodecParameters->sample_rate, 0, NULL);
		ret = swr_init(m_pSwrContext);
		if (ret != 0)
		{
			return false;
		}
		ret = swr_is_initialized(m_pSwrContext);
		if (ret == 0)
		{
			return false;
		}



		ret = avcodec_open2(m_pCodecContext, m_pCodec, NULL);

		return ret == 0;
	}

	int AVDecodeUtil::decodePacket(AVPacket *pPacket, vector<char> *pVideoData, vector<char> *pAudioData)
	{
		AVFrame *pCurrentFrame = av_frame_alloc();
		//	int gop;

		int ret = avcodec_send_packet(m_pCodecContext, pPacket);
		ret = avcodec_receive_frame(m_pCodecContext, pCurrentFrame);

		if (ret != 0)
		{
			av_frame_unref(pCurrentFrame);

			int a = AVERROR_EOF;
			a = AVERROR(EAGAIN);
			a = AVERROR(EINVAL);
			av_frame_free(&pCurrentFrame);
			return 1;
		}
		if (this->m_streamType == AVMEDIA_TYPE_VIDEO)
		{
			int dataSize = (int)m_dstBuffer.size();

			sws_scale(m_pSwsContext, pCurrentFrame->data, pCurrentFrame->linesize, 0, pCurrentFrame->height, m_pDstFrame->data, m_pDstFrame->linesize);
			*pVideoData = m_dstBuffer;
		}
		else if (this->m_streamType == AVMEDIA_TYPE_AUDIO)
		{
			
		
			
			int outSampleCount = swr_get_out_samples(m_pSwrContext, pCurrentFrame->nb_samples);

			int dataSize = av_samples_get_buffer_size(NULL, m_pDstFrame->channels, outSampleCount, (AVSampleFormat)m_pDstFrame->format, 0);
			m_dstBuffer.resize(dataSize);
			int ret = av_samples_fill_arrays(m_pDstFrame->data, m_pDstFrame->linesize, (uint8_t*)&m_dstBuffer[0], m_pDstFrame->channels, pCurrentFrame->nb_samples, (AVSampleFormat)m_pDstFrame->format, 0);

			ret = swr_convert(m_pSwrContext, m_pDstFrame->data, outSampleCount, (const uint8_t **)pCurrentFrame->extended_data, pCurrentFrame->nb_samples);
	//		memcpy(&m_dstBuffer[0], &pCurrentFrame->data[0][0], 4096);
		
		
			*pAudioData = m_dstBuffer;
	/*		vector<char> t(2048);
			memcpy(&t[0], &pCurrentFrame->extended_data[0][2048], 2048);*/
			//pCurrentFrame->data
			printf("t");
			
		}


		av_frame_unref(pCurrentFrame);
		av_frame_free(&pCurrentFrame);
		//	avcodec_free_context(&pCodecContex);
		return 0;
	}

}