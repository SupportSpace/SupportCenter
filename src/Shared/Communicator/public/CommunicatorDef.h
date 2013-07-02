#pragma once

#ifdef _DYNAMIC_COMMUNICATOR_
	#ifdef _EXPORT_COMMUNICATOR_
		#define COMMUNICATOR_API __declspec(dllexport)
	#else
		#define COMMUNICATOR_API __declspec(dllimport)
	#endif
#else
	#define COMMUNICATOR_API  
#endif