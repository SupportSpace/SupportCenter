/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTokenCatcher.h
///
///  Catches tokens within streams
///
///  @author "Archer Software" Sogin M. @date 16.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>

#define START_COMMAND "{A46B7746-DA7F-42cc-85F0-EC412F46A0FA}"

/// Catches tokens within streams
class CTokenCatcher
{
private:
	/// internal stream cache
	std::auto_ptr<char> m_buffer;
	/// internal buffer size;
	int m_bufferLen;
	/// internal buffer current position
	unsigned int m_pos;
	/// token to catch
	std::auto_ptr<char> m_token;
	/// token length
	unsigned int m_tokenLen;
public:
	/// Initializes object instance
	/// @param token token to find in stream
	/// @param tokenLen token length
	CTokenCatcher(const char* token, const unsigned int tokenLen);

	/// dtor
	~CTokenCatcher(void);

	/// Send buffer from stream input
	/// @param buf buffer to put
	/// @param length length of buffer
	/// @return true if token found in overall input sequence
	bool Send(const char* buf, const unsigned int length);

	/// Resets search (before new one)
	void Reset();
private:
	/// compares memory blocks to be idential
	/// @param buf1 memory block1
	/// @param buf2 memory block2
	/// @param len count of bytes to compare
	/// @return true if blocks are idential
	static bool MyMemcmp(char* buf1, char* buf2, unsigned int len);
};
