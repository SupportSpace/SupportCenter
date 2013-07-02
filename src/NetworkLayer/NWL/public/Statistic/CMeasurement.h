////////////////////////////////////////////////////////////////////////
///
///  Support Space.
///
///  CMeasurement.h
///
///  Measurement service declaration 
///
///  @author Alex Gantman @date 18.6.2008
///
////////////////////////////////////////////////////////////////////////
// CMeasurement.h : Declaration of the CMeasurement
#pragma once
#pragma pack 1
#include <NWL/NetworkLayer.h>
//#define BOOST_THREAD_NO_LIB
#include <list>
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>
//#include <RCEngine/RCEngine.h>
#include <limits.h>
#include <AidLib/CException/CException.h>
#include <string>
#include <boost/format.hpp>

using std::list;
using std::string;

#define PING_PONG_MARK 0x5a5a5a5a

struct MeasurementPoint
{
	MeasurementPoint(DWORD _dTimeStampSend,DWORD _dTimeStampReceived) : dHeader(PING_PONG_MARK)
		{ 
			dTimeStampSend			= _dTimeStampSend;
			dTimeStampReceived	= _dTimeStampReceived;
		}
	const DWORD dHeader;
	BOOLEAN fState;
	DWORD dTimeStampSend;
	DWORD dTimeStampReceived;


};

typedef std::list<MeasurementPoint> SamplesFIFO;
// Mesurament points in msec interval
const unsigned int mesurament_points[2][2] = {
		// 0 to 1min in 10sec interval
		{60*1000,10*1000},
		// from 1 min to FOREVER in 5 min interval
		{ULLONG_MAX,5*60*1000}
	};

class NWL_API CMeasurement 
{
protected:
	SamplesFIFO sampleFIFO;
	DWORD MeasurementDelta;
	float dLaitancy;
	string ppHistory;			//string based data representation

	/// Time based speed calculation algorithm
	unsigned int i_start_time,i_delta,i_index,i_delta_time,i_begin_of_mesurament,i_begin_of_delta,i_mesurament_points;
	float fAvgSpeed;
public:

	CMeasurement() {};
	void SetLifePeriod(DWORD _MeasurementDelta) {MeasurementDelta = _MeasurementDelta; }
	void AddSample(MeasurementPoint &mPoint);
	float GetPerformance();
	const char* data()
			{
				ppHistory.clear();
				i_index=0;
				i_delta_time=0;
				i_begin_of_mesurament = 0;
				i_start_time = mesurament_points[i_index][0];
				i_delta		 = mesurament_points[i_index][1];

				for_each (sampleFIFO.begin(),sampleFIFO.end(), DataToStr(*this));
				return ppHistory.c_str();
			}
	DWORD size()
	{
		return ppHistory.length();
	}

	//std hellper classes
	class Accamulate
	{
		private:
			CMeasurement& cMeasurement;
		public:
			Accamulate(CMeasurement& _cMeasurement): cMeasurement(_cMeasurement) {};
		
			// The function call to process the next elment
 			void operator ( ) ( MeasurementPoint sMeasurementPoint)
			{
				DWORD dDelta;
  
				dDelta = sMeasurementPoint.dTimeStampReceived - sMeasurementPoint.dTimeStampSend;
				if (dDelta < 0)
					throw MCException("Measurement is incorrect");
  

				cMeasurement.dLaitancy += (float)(dDelta);
			}
		};
	class DataToStr
	{
		private:
			CMeasurement& cOwn;
	
			
		public:
			DataToStr(CMeasurement& _cMeasurement): cOwn(_cMeasurement){};
		
			// The function call to process the next elment
 			void operator ( ) ( MeasurementPoint sMeasurementPoint)
			{
				if (cOwn.i_begin_of_mesurament == 0)
				{
					cOwn.i_begin_of_mesurament = sMeasurementPoint.dTimeStampSend;
					cOwn.i_begin_of_delta = sMeasurementPoint.dTimeStampSend;
					cOwn.i_mesurament_points=0;
				} else
					cOwn.i_mesurament_points++;

				cOwn.i_delta_time += (sMeasurementPoint.dTimeStampReceived - sMeasurementPoint.dTimeStampSend);

				if (cOwn.i_start_time < (cOwn.MeasurementDelta*cOwn.i_index*cOwn.i_mesurament_points))
				{
					cOwn.i_begin_of_mesurament=0;
					cOwn.i_index++;
					return;
				}
				if (cOwn.i_delta < (cOwn.MeasurementDelta*cOwn.i_mesurament_points))
				{
					cOwn.i_mesurament_points = max (cOwn.i_mesurament_points,1);
					cOwn.ppHistory += boost::str(boost::format(":%f.2") % (((float)cOwn.i_delta_time)/cOwn.i_mesurament_points ));
					cOwn.i_begin_of_delta = sMeasurementPoint.dTimeStampReceived;
					cOwn.i_mesurament_points=0;
					cOwn.i_delta_time=0;

				}
			}
	};


};

/// Should be used to CStatisticClient as single instance
#define MEASUREMENT_INFO_INSTANCE Loki::SingletonHolder<CMeasurement , Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
