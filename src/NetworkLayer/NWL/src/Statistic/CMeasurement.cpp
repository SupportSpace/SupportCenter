/////////////////////////////////////////////////////////////////////////
///
///  Support Space Ltd.
///
///  CMeasurement.cpp
///
///  CMeasurement object implementation. 
///
///  @author Alex Gantman @date 18.06.2008
///
////////////////////////////////////////////////////////////////////////
#include "NWL\Statistic\CMeasurement.h"

void CMeasurement::AddSample(MeasurementPoint &mPoint) 
{
	sampleFIFO.insert(sampleFIFO.end(), mPoint);
}
 

float CMeasurement::GetPerformance()
{
 TRY_CATCH

	 _ASSERT (MeasurementDelta =! 0);

	 dLaitancy = 0; 
	 for_each (sampleFIFO.begin(),sampleFIFO.end(), Accamulate(*this ));
	 dLaitancy = dLaitancy / sampleFIFO.size() - MeasurementDelta ;
 
 CATCH_LOG()
	return dLaitancy;


}