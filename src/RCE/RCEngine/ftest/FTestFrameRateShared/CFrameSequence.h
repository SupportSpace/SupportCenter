/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameSequence.h
///
///  Declares CFrameSequence class, responsible for management of sequence of frames
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include "CFrame.h"
#include "Rects.h"
#include "SControlPoint.h"
#include <AidLib/Strings/tstring.h>
#include <map>

///  Map to store referenses to frames
typedef std::map<tstring,int> FramesMap;
///  Map's entry
typedef std::pair<tstring, int> FramesMapEntry;

///  CFrameSequence class, responsible for management of sequence of frames
class CFrameSequence
{
private:
///  Prevents making copies of CFrameSequence objects.
	CFrameSequence( const CFrameSequence& );
	CFrameSequence& operator=( const CFrameSequence& );

public:
///  Constructor
	CFrameSequence();
///  Destructor
	~CFrameSequence();

private:
///  Vector of frames
	Frames			m_frames;
///  Current frame index
	int				m_currentFrame;
///  Number of frames
	int				m_framesCount;
///  Number of repeats
	int				m_repeats;
///  last found frame
	int				m_lastFoundFrame;
///  Storage to control points
	ControlPoints	m_controlPoints;
///  Storage to fast search frames by hash
	FramesMap		m_framesMap;

public:
///  Loads frames
	void LoadFrames();

///  Gets next frame to show
///  @return shared pointer to next frame
	SPFrame GetNextFrame();

///  Finds frame by DC
///  @param hdc - viewer's bitmap dc
///  @param skipped - pointer to number of skipped frames
///  @return true if frame found
	bool FindFrame( HDC hdc, int* skipped );

///  Resets internal data
	void Reset();

private:
///  Converts mask to vector of rectangles
///  @param buf - buffer with mask data
///  @param len - buffer size
///  @return vector of rectangles
	Rects GetRectsByMask( const char* buf, const int len );

///  Converts mask to vector of control points
///  @param buf - buffer with mask data
///  @param len - buffer size
///  @return vector of control points
	ControlPoints GetControlPointsByMask( const char* buf, const int len );

///  Creates string hash by control points
///  @param points - vector with control points
///  @return string hash
	tstring GetHashByControlPoints( const ControlPoints& points );

///  Initializes internal vector with control points
	void InitControlPoints();

};

///  Shared pointer to CFrameSequence class
typedef boost::shared_ptr<CFrameSequence> SPFrameSequence;
