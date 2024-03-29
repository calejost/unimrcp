/*
 * Copyright 2008-2014 Arsen Chaloyan
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
 * 
 * $Id$
 */

#ifndef RECOG_SESSION_H
#define RECOG_SESSION_H

/**
 * @file recogsession.h
 * @brief Recognizer Session
 */ 

#include "umcsession.h"

class RecogScenario;
struct RecogChannel;

class RecogSession : public UmcSession
{
public:
/* ============================ CREATORS =================================== */
	RecogSession(const RecogScenario* pScenario);
	virtual ~RecogSession();

protected:
/* ============================ MANIPULATORS =============================== */
	virtual bool Start();
	virtual bool Stop();

	RecogChannel* CreateRecogChannel();
	bool StartRecognition(mrcp_channel_t* pMrcpChannel);
	bool OnDefineGrammar(mrcp_channel_t* pMrcpChannel);

	mrcp_message_t* CreateDefineGrammarRequest(mrcp_channel_t* pMrcpChannel);
	mrcp_message_t* CreateRecognizeRequest(mrcp_channel_t* pMrcpChannel);

	static bool ParseNLSMLResult(mrcp_message_t* pMrcpMessage);
	FILE* GetAudioIn(const mpf_codec_descriptor_t* pDescriptor, apr_pool_t* pool) const;

/* ============================ HANDLERS =================================== */
	virtual bool OnSessionTerminate(mrcp_sig_status_code_e status);
	virtual bool OnChannelAdd(mrcp_channel_t* pMrcpChannel, mrcp_sig_status_code_e status);
	virtual bool OnMessageReceive(mrcp_channel_t* pMrcpChannel, mrcp_message_t* pMrcpMessage);

/* ============================ ACCESSORS ================================== */
	const RecogScenario* GetScenario() const;

private:
/* ============================ DATA ======================================= */
	RecogChannel* m_pRecogChannel;
	const char*   m_ContentId;
};


/* ============================ INLINE METHODS ============================= */
inline const RecogScenario* RecogSession::GetScenario() const
{
	return (RecogScenario*)m_pScenario;
}

#endif /* RECOG_SESSION_H */
