/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  TLSStructs.h
///
///  Declares constants, structures for using GnuTLS library
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>
#include "TLSWrapper.h"

#define DEFAULT_TLS_BUFFER_SIZE 4096

/// Key exchange algorithm
enum ETLSKeyExchange
{
	KX_PSK		= GNUTLS_KX_PSK,		// Pre-Shared Keys
	KX_DHE_PSK	= GNUTLS_KX_DHE_PSK		// Pre-Shared Keys with Diffie-Hellman exchange
};

/// Compression type
enum ETLSCompression
{
	PRS_NULL	= GNUTLS_COMP_NULL,		// None
	PRS_ZLIB	= GNUTLS_COMP_ZLIB,		// ZLIB algorithm
	PRS_LZO		= GNUTLS_COMP_LZO		// LZO algorithm
};

/// Cipher type
enum ETLSCipher
{
	CPH_NULL	= GNUTLS_CIPHER_NULL,
	CPH_AES_128	= GNUTLS_CIPHER_AES_128_CBC,
	CPH_AES_256	= GNUTLS_CIPHER_AES_256_CBC,
	CPH_3DES	= GNUTLS_CIPHER_3DES_CBC,
	CPH_RC4_128	= GNUTLS_CIPHER_ARCFOUR_128
};

/// Prime bits
enum ETLSPrimeBits
{
	PB_768		= 768,
	PB_1024		= 1024,
	PB_2048		= 2048,
	PB_3072		= 3072,
	PB_4096		= 4096
};

/// Cipher suite
struct NWL_API STLSSuite
{
	ETLSKeyExchange		KeyExchange;	// Key Exchange algorithm
	ETLSCipher			Cipher;			// Cipher algorithm
	ETLSCompression		Compression;	// Compression algorithm
	ETLSPrimeBits		PrimeBits;		// needed for Diffie-Hellman exchange

	// Default constructor
	STLSSuite() : KeyExchange( KX_PSK ), Cipher( CPH_AES_256 ), Compression( PRS_ZLIB ), PrimeBits( PB_1024 ) {};
};

/// Credentials structure
struct NWL_API STLSCredentials
{
	tstring	UserID;
	tstring	Key;

	// Default constructor
	STLSCredentials() : UserID( _T("") ), Key( _T("") ) {};
};