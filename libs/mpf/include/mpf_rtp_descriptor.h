/*
 * Copyright 2008 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MPF_RTP_DESCRIPTOR_H__
#define __MPF_RTP_DESCRIPTOR_H__

/**
 * @file mpf_rtp_descriptor.h
 * @brief MPF RTP Stream Descriptor
 */ 

#include "mpf_stream_mode.h"
#include "mpf_media_descriptor.h"
#include "mpf_codec_descriptor.h"

APT_BEGIN_EXTERN_C

/** RTP media descriptor declaration */
typedef struct mpf_rtp_media_descriptor_t mpf_rtp_media_descriptor_t;
/** RTP stream descriptor declaration */
typedef struct mpf_rtp_stream_descriptor_t mpf_rtp_stream_descriptor_t;
/** RTP termination descriptor declaration */
typedef struct mpf_rtp_termination_descriptor_t mpf_rtp_termination_descriptor_t;
/** RTP config */
typedef struct mpf_rtp_config_t mpf_rtp_config_t;


/** RTP media (local/remote) descriptor */
struct mpf_rtp_media_descriptor_t {
	/** Media descriptor base */
	mpf_media_descriptor_t base;
	/** Stream mode (send/receive) */
	mpf_stream_mode_e      mode;
	/** Packetization time */
	apr_uint16_t           ptime;
	/** Codec list */
	mpf_codec_list_t       codec_list;
	/** Media identifier */
	apr_size_t             mid;
};

/** RTP stream descriptor */
struct mpf_rtp_stream_descriptor_t {
	/** Local media descriptor */
	mpf_rtp_media_descriptor_t *local;
	/** Remote media descriptor */
	mpf_rtp_media_descriptor_t *remote;
};

/** RTP termination descriptor */
struct mpf_rtp_termination_descriptor_t {
	/** Audio stream descriptor */
	mpf_rtp_stream_descriptor_t audio;
	/** Video stream descriptor */
	mpf_rtp_stream_descriptor_t video;
};

/** RTP config */
struct mpf_rtp_config_t {
	/** Local IP address to bind to */
	apt_str_t ip;
	/** RTP port range (min) */
	apr_port_t rtp_port_min;
	/** RTP port range (max) */
	apr_port_t rtp_port_max;
	/** RTP port range (cur) */
	apr_port_t rtp_port_cur;
};

/** Initialize media descriptor */
static APR_INLINE void mpf_rtp_media_descriptor_init(mpf_rtp_media_descriptor_t *media)
{
	mpf_media_descriptor_init(&media->base);
	media->mode = STREAM_MODE_NONE;
	media->ptime = 0;
	mpf_codec_list_reset(&media->codec_list);
	media->mid = 0;
}

/** Initialize stream descriptor */
static APR_INLINE void mpf_rtp_stream_descriptor_init(mpf_rtp_stream_descriptor_t *stream)
{
	stream->local = NULL;
	stream->remote = NULL;
}

/** Initialize RTP termination descriptor */
static APR_INLINE void mpf_rtp_termination_descriptor_init(mpf_rtp_termination_descriptor_t *rtp_descriptor)
{
	mpf_rtp_stream_descriptor_init(&rtp_descriptor->audio);
	mpf_rtp_stream_descriptor_init(&rtp_descriptor->video);
}

APT_END_EXTERN_C

#endif /*__MPF_RTP_DESCRIPTOR_H__*/
