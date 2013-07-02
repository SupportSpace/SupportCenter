/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CShadowedClient.cpp
///
///  Stream, emulating client
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////

#include "CShadowedClient.h"
#include "Streaming/SBlock.h"
#include <memory>
#include <AidLib/Logging/cLog.h>
#include "rfb.h"
#include "CTokenCatcher.h"
#include "PermissibleWarnings.h"
#include <AidLib/CCritSection/CCritSection.h>

const rfbPixelFormat vnc8bitFormat	= {8,	8,	0,	1,	7,	7,	3,	0,	3,	6,	0,	0}; // 256 colors
const rfbPixelFormat vnc16bitFormat	= {16,	16,	0,	1,	63,	31,	31,	0,	6,	11,	0,	0}; // 65000 colors
extern CARD16 screenWidth;
extern CARD16 screenHeight;

CShadowedClient::CShadowedClient(boost::shared_ptr<CAbstractStream> stream)
	: m_stream(stream), m_pos(0), m_inited(false), m_cancelled(false)
{
TRY_CATCH

	InitializeCriticalSection(&m_csRead);
	InitializeCriticalSection(&m_csWrite);

	/// Encodings
	std::vector<CARD32> enc;
	enc.push_back(Swap32IfLE(rfbEncodingZRLE));
	//enc.push_back(Swap32IfLE(rfbEncodingZlib));
	//enc.push_back(Swap32IfLE(rfbEncodingTight));
	//enc.push_back(Swap32IfLE(rfbEncodingZlibHex));
	//enc.push_back(Swap32IfLE(rfbEncodingCompressLevel0 + 6));
	enc.push_back(Swap32IfLE(rfbEncodingXCursor));
	enc.push_back(Swap32IfLE(rfbEncodingRichCursor));
	enc.push_back(Swap32IfLE(rfbEncodingPointerPos));
	/*enc.push_back(Swap32IfLE(rfbEncodingQualityLevel0 + 6));
	enc.push_back(Swap32IfLE(rfbEncodingXOREnable));*/
	enc.push_back(Swap32IfLE(rfbEncodingLastRect));
	enc.push_back(Swap32IfLE(rfbEncodingNewFBSize));
	enc.push_back(Swap32IfLE(rfbEncodingCacheEnable));

	//Initial messages---------------------------------------------------------------------------
	//TODO: add rfbSetPixelFormat & rfbSetEncodings messages
	m_initialMessageSize = strlen(START_COMMAND)
				+
				sz_rfbClientInitMsg
				+
				sz_rfbSetPixelFormatMsg
				+
				sz_rfbSetEncodingsMsg
				+ 
				enc.size()*sizeof(CARD32);
	m_initialMessage.reset(new char[m_initialMessageSize]);

	/// SART_COMMAND
	int pos = 0;
	memcpy(m_initialMessage.get() + pos, START_COMMAND, strlen(START_COMMAND));
	pos+=strlen(START_COMMAND);

	/// rfbClientInitMsg
	rfbClientInitMsg initMsg;
	initMsg.shared = false;
	memcpy(m_initialMessage.get() + pos, &initMsg, sz_rfbClientInitMsg);
	pos+=sz_rfbClientInitMsg;

	/// rfbSetPixelFormat
	rfbSetPixelFormatMsg pixelFormatMsg;
	pixelFormatMsg.type = rfbSetPixelFormat;
	pixelFormatMsg.format = vnc16bitFormat; //vnc8bitFormat;
    pixelFormatMsg.format.redMax = Swap16IfLE(pixelFormatMsg.format.redMax);
    pixelFormatMsg.format.greenMax = Swap16IfLE(pixelFormatMsg.format.greenMax);
    pixelFormatMsg.format.blueMax = Swap16IfLE(pixelFormatMsg.format.blueMax);
	memcpy(m_initialMessage.get() + pos, &pixelFormatMsg, sz_rfbSetPixelFormatMsg);
	pos+=sz_rfbSetPixelFormatMsg;

	/// rfbSetEncodings
	rfbSetEncodingsMsg setEncodingsMsg;
	setEncodingsMsg.type = rfbSetEncodings;
	setEncodingsMsg.nEncodings = Swap16IfLE(enc.size());
	setEncodingsMsg.pad = 0;
	memcpy(m_initialMessage.get() + pos, &setEncodingsMsg, sz_rfbSetEncodingsMsg);
	pos+=sz_rfbSetEncodingsMsg;

	for(std::vector<CARD32>::iterator i=enc.begin();
		i != enc.end();
		++i)
	{
		memcpy(m_initialMessage.get() + pos, &(*i), sizeof(CARD32));
		pos+=sizeof(CARD32);
	}


	//Framebuffer updates------------------------------------------------------------------------
	m_updateRequest.reset(new char[sz_rfbFramebufferUpdateRequestMsg]);
	rfbFramebufferUpdateRequestMsg updateRequest;
	memset(&updateRequest,0,sz_rfbFramebufferUpdateRequestMsg);
	updateRequest.type = rfbFramebufferUpdateRequest;
	updateRequest.incremental = 1;
	memcpy(m_updateRequest.get(), &updateRequest, sz_rfbFramebufferUpdateRequestMsg);
	m_height = reinterpret_cast<CARD16*>(m_updateRequest.get() + (reinterpret_cast<char*>(&updateRequest.h) - reinterpret_cast<char*>(&updateRequest)));
	m_width = reinterpret_cast<CARD16*>(m_updateRequest.get() + (reinterpret_cast<char*>(&updateRequest.w) - reinterpret_cast<char*>(&updateRequest)));

CATCH_THROW("CShadowedClient::CShadowedClient")
}

CShadowedClient::~CShadowedClient(void)
{
TRY_CATCH
	DeleteCriticalSection(&m_csRead);
	DeleteCriticalSection(&m_csWrite);
CATCH_LOG("CShadowedClient::~CShadowedClient")
}

unsigned int CShadowedClient::SendInternal( const char*buf, const unsigned int& len )
{
TRY_CATCH
	CCritSection cs(&m_csWrite);
	if (m_stream.get())
		m_stream->SendInternal(buf,len);
	return len;
CATCH_THROW("CShadowedClient::SendInternal")
}

unsigned int CShadowedClient::ReceiveInternal( char* buf, const unsigned int& len)
{
TRY_CATCH
	CCritSection cs(&m_csRead);
	if (m_cancelled) throw MCException("Cancelled");
	if (!m_inited)
	{
		///Not yet inited
		if (m_pos + len > m_initialMessageSize)
			throw MCException("m_pos + len > m_initialMessageSize");
		memcpy(buf, m_initialMessage.get() + m_pos, len); 
		m_pos+=len;
		if (m_pos >= m_initialMessageSize)
		{
			m_inited = true;
			m_pos = 0;
		}
	}
	else
	{
		Sleep(150);	//TODO: tune this value
		*m_height = screenHeight;
		*m_width = screenWidth;

		//Sending update command
		if (m_pos + len > sz_rfbFramebufferUpdateRequestMsg)
			throw MCException("m_pos + len > sz_rfbFramebufferUpdateRequestMsg");
		memcpy(buf, m_updateRequest.get() + m_pos, len);
		m_pos+=len;
		if (m_pos >= sz_rfbFramebufferUpdateRequestMsg)
			m_pos=0;
		return len;
	}
	return len;
CATCH_THROW("CShadowedClient::ReceiveInternal")
}

bool CShadowedClient::HasInData()
{
TRY_CATCH
	throw MCException("Method not supported");
CATCH_THROW("CShadowedClient::HasInData")
}

///  Cancel reading from the stream
void CShadowedClient::CancelReceiveOperation()
{
TRY_CATCH
	m_cancelled = true;
CATCH_THROW("CShadowedClient::CancelReceiveOperation")
}