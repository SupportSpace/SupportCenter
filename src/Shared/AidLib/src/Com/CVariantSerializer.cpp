/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVariantSerializer.cpp
///
///  Implements CVariantSerializer class, responsible for serialization of variants
///
///  @author Dmitry Netrebenko @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/Com/CVariantSerializer.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Strings/tstring.h>
#include <atlbase.h>

CVariantSerializer::SPCharBuf CVariantSerializer::VariantToBuf(CVariantSerializer::SPVariant var, unsigned int* size)
{
TRY_CATCH
	/// Check up pointers
	if(!var.get() || (NULL == size))
		MCException(_T("Invalid pointer."));

	/// Init local variables
	SPCharBuf buf;
	IStream* stream;
	HRESULT result = S_OK;
	ULARGE_INTEGER count;
	LARGE_INTEGER pos;
	pos.QuadPart = 0;
	ULONG read;

	/// Create stream in the memory
	result = CreateStreamOnHGlobal(NULL, TRUE, &stream);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to CreateStreamOnHGlobal (%X)."), result));

	/// Write variant to stream
	result = var->WriteToStream(stream);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to WriteToStream (%X)."), result));

	/// Calculate size of data
	result = stream->Seek(pos, STREAM_SEEK_END, &count);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to IStream->Seek (%X)."), result));

	/// Check up size
	if(count.HighPart > 0)
		throw MCException(_T("Variant too big."));

	/// Jump to begin
	result = stream->Seek(pos, STREAM_SEEK_SET, NULL);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to IStream->Seek (%X)."), result));

	/// Allocate memory
	unsigned int sz = count.LowPart;
	buf.reset(new char[sz]);

	/// Copy data from stream to buffer
	result = stream->Read(buf.get(), sz, &read);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to IStream->Read (%X)."), result));

	/// Return buffer and size
	*size = sz;
	return buf;
CATCH_THROW()
}

CVariantSerializer::SPVariant CVariantSerializer::BufToVariant(CVariantSerializer::SPCharBuf buf, const unsigned int size)
{
TRY_CATCH
	/// Check up pointer
	if(!buf.get())
		MCException(_T("Invalid pointer."));

	/// Init local variables
	SPVariant var;
	IStream* stream;
	HRESULT result = S_OK;
	ULONG write;
	LARGE_INTEGER pos;
	pos.QuadPart = 0;

	/// Create stream in the memory
	result = CreateStreamOnHGlobal(NULL, TRUE, &stream);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to CreateStreamOnHGlobal (%X)."), result));

	/// Write buffer into stream
	result = stream->Write(buf.get(), size, &write);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to IStream->Write (%X)."), result));

	/// Jump to beginning
	result = stream->Seek(pos, STREAM_SEEK_SET, NULL);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to IStream->Seek (%X)."), result));

	/// Create variant
	var.reset(new CComVariantEx);

	/// Read variant's data from stream
	result = var->ReadFromStream(stream);
	if(S_OK != result)
		throw MCException(Format(_T("Failed to ReadFromStream (%X)."), result));

	/// Return variant
	return var;
CATCH_THROW()
}
