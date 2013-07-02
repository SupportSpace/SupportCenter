/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameSequence.cpp
///
///  Implements CFrameSequence class, responsible for management of sequence of frames
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFrameSequence.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/scoped_ptr.hpp>
#include <AidLib/CException/CException.h>
#include "CBlockBitmap.h"
#include <numeric>

CFrameSequence::CFrameSequence()
	:	m_currentFrame(-1)
	,	m_framesCount(0)
	,	m_repeats(0)
	,	m_lastFoundFrame(-1)
{
TRY_CATCH
CATCH_THROW()
}

CFrameSequence::~CFrameSequence()
{
TRY_CATCH
CATCH_LOG()
}

void CFrameSequence::LoadFrames()
{
TRY_CATCH

	/// Open file with frames data
	HANDLE hFile = CreateFile(
		SETTINGS_INSTANCE.GetFramesFileName().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(INVALID_HANDLE_VALUE == hFile)
		throw MCException_Win(_T("Can not open file with frames"));

	/// Create shared pointer to file's handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spFile(hFile,CloseHandle);

	/// Read number of columns
	int cols = 0;
	DWORD read = 0;
	ReadFile(hFile, &cols, 4, &read, NULL);

	if(4 != read)
		throw MCException_Win(_T("Can not read number of columns"));

	/// Read number of rows
	int rows = 0;
	ReadFile(hFile, &rows, 4, &read, NULL);

	if(4 != read)
		throw MCException_Win(_T("Can not read number of rows"));

	/// Calculate number of bytes per frame
	int bytesPerFrame = cols * rows;

	/// Set grid settings
	SETTINGS_INSTANCE.SetGrid(cols,rows);

	/// Read number of frames
	int framesCount = 0;
	ReadFile(hFile, &framesCount, 4, &read, NULL);

	if(4 != read)
		throw MCException_Win(_T("Can not read number of frames"));

	m_framesCount = 0;

	m_framesMap.clear();

	SPRect prevFrameRect(new RECT());
	ControlPoints firstFrameControlPoints;
	Rects firstFrameRects;

	/// Read frames
	for(int i = 0; i < framesCount; ++i)
	{
		boost::scoped_ptr<char> buf(new char[bytesPerFrame]);
		ReadFile(hFile, buf.get(), bytesPerFrame, &read, NULL);

		if(read != bytesPerFrame)
			throw MCException_Win(_T("Error at loading frame"));

		/// Create and init new frame
		SPFrame frame(new CFrame());
		/// Create control points
		ControlPoints points = GetControlPointsByMask(buf.get(), bytesPerFrame);
		/// Create vector of rectangles
		Rects rects = GetRectsByMask(buf.get(), bytesPerFrame);
		if(0 == i)
		{
			/// Remember control points and rectangles for first frame
			firstFrameControlPoints = points;
			firstFrameRects = rects;
		}
		else
		{
			/// Init frame
			frame->Init(rects, points, prevFrameRect);
		}

		prevFrameRect.reset(new RECT());
		prevFrameRect = std::accumulate(rects.begin(), rects.end(), prevFrameRect, RectComposition());

		/// Get hash by control points
		tstring hash = GetHashByControlPoints(points);
		/// Add entry to map
		m_framesMap.insert(FramesMapEntry(hash, m_framesCount));
		m_frames.push_back(frame);
		m_framesCount++;
	}

	/// Init first frame
	if(framesCount > 0)
		m_frames[0]->Init(firstFrameRects, firstFrameControlPoints, prevFrameRect);

	/// Initialization of control points storage
	InitControlPoints();

CATCH_THROW()
}

SPFrame CFrameSequence::GetNextFrame()
{
TRY_CATCH

	m_currentFrame++;
	if(m_currentFrame >= m_framesCount)
	{
		m_currentFrame = 0;
		m_repeats++;
		if(m_repeats >= SETTINGS_INSTANCE.GetRepeats())
		{
			m_currentFrame = m_framesCount;
			m_repeats--;
			/// Return empty frame
			return SPFrame(reinterpret_cast<CFrame*>(NULL));
		}
	}
	/// Get next frame
	return m_frames[m_currentFrame];

CATCH_THROW()
}

Rects CFrameSequence::GetRectsByMask(const char* buf, const int len)
{
TRY_CATCH

	/// Create vector of rectangles by frame data
	Rects rects;
	char* buffer = const_cast<char*>(buf);
	int cols = SETTINGS_INSTANCE.GetColCount();
	int rows = SETTINGS_INSTANCE.GetRowCount();
	int width = SETTINGS_INSTANCE.GetBlockWidth();
	int height = SETTINGS_INSTANCE.GetBlockHeight();

	for(int i = 0; i < rows; ++i)
	{
		for(int j = 0; j < cols; ++j)
		{
			byte value = *buffer;
			if(value)
			{
				/// Create rectangle
				SPRect rect(new RECT());
				rect->left = width * j;
				rect->right = rect->left + width;
				rect->top = height * i;
				rect->bottom = rect->top + height;

				/// Add rectangle to vector
				rects.push_back(rect);
			}
			buffer++;
		}
	}
	return rects;

CATCH_THROW()
}

bool CFrameSequence::FindFrame(HDC hdc, int* skipped)
{
TRY_CATCH

	ControlPoints::iterator index;
	tstring hash(_T(""));

	/// Get colors of control points and create hash
	for(index = m_controlPoints.begin(); index != m_controlPoints.end(); ++index)
	{
		SPControlPoint point = *index;
		hash += i2tstring(::GetPixel(hdc, point->m_point.x, point->m_point.y));
		hash += _T(";");
	}

	/// Search frame by hash
	FramesMap::iterator foundFrame = m_framesMap.find(hash);
	if(foundFrame == m_framesMap.end())
		return false;
	int frame = foundFrame->second;

	/// Calculating number of skipped frames
	if(m_lastFoundFrame < frame)
	{
		*skipped = frame - m_lastFoundFrame - 1;
		m_lastFoundFrame = frame;
		return true;
	}
	else
	{
		if(m_lastFoundFrame > frame)
		{
			*skipped = frame + m_framesCount - m_lastFoundFrame - 1;
			m_lastFoundFrame = frame;
			return true;
		}
		else
			return false;
	}

CATCH_THROW()
}

void CFrameSequence::Reset()
{
TRY_CATCH

	m_currentFrame = -1;
	m_lastFoundFrame = -1;

CATCH_THROW()
}

ControlPoints CFrameSequence::GetControlPointsByMask(const char* buf, const int len)
{
TRY_CATCH

	/// Create vector of control points by frame data
	ControlPoints points;
	char* buffer = const_cast<char*>(buf);
	int cols = SETTINGS_INSTANCE.GetColCount();
	int rows = SETTINGS_INSTANCE.GetRowCount();
	int width = SETTINGS_INSTANCE.GetBlockWidth();
	int height = SETTINGS_INSTANCE.GetBlockHeight();
	HDC dc = BLOCKBITMAP_INSTANCE.GetDC();
	bool analyzeEmpty = SETTINGS_INSTANCE.GetAnalyzeEmptySpace();

	int blockX = SETTINGS_INSTANCE.GetBlockWidth() / 2;
	int blockY = SETTINGS_INSTANCE.GetBlockHeight() / 2;

	COLORREF blockColor = GetPixel(dc, blockX, blockY);
	COLORREF clearColor = RGB(255,255,255);

	for(int i = 0; i < rows; ++i)
	{
		for(int j = 0; j < cols; ++j)
		{
			SPControlPoint point(new SControlPoint());
			point->m_point.x = width * j + blockX;
			point->m_point.y = height * i + blockY;

			byte value = *buffer;
			if(value)
				point->m_color = blockColor;
			else
				point->m_color = clearColor;

			points.push_back(point);
			buffer++;
		}
	}
	return points;

CATCH_THROW()
}

tstring CFrameSequence::GetHashByControlPoints(const ControlPoints& points)
{
TRY_CATCH

	/// Calculating string hash by control points
	tstring result(_T(""));
	ControlPoints::const_iterator index;

	for(index = points.begin(); index != points.end(); ++index)
	{
		SPControlPoint point = *index;
		result += i2tstring(point->m_color);
		result += _T(";");
	}

	return result;

CATCH_THROW()
}

void CFrameSequence::InitControlPoints()
{
TRY_CATCH

	/// Initialization of internal vector with control points
	int cols = SETTINGS_INSTANCE.GetColCount();
	int rows = SETTINGS_INSTANCE.GetRowCount();
	int width = SETTINGS_INSTANCE.GetBlockWidth();
	int height = SETTINGS_INSTANCE.GetBlockHeight();
	HDC dc = BLOCKBITMAP_INSTANCE.GetDC();

	int blockX = SETTINGS_INSTANCE.GetBlockWidth() / 2;
	int blockY = SETTINGS_INSTANCE.GetBlockHeight() / 2;

	COLORREF blockColor = GetPixel(dc, blockX, blockY);

	for(int i = 0; i < rows; ++i)
	{
		for(int j = 0; j < cols; ++j)
		{
			SPControlPoint point(new SControlPoint());
			point->m_point.x = width * j + blockX;
			point->m_point.y = height * i + blockY;
			point->m_color = blockColor;
			m_controlPoints.push_back(point);
		}
	}

CATCH_THROW()
}
