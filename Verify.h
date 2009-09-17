

#ifndef __VERIFY_H 
#define __VERIFY_H 1

#include <stdexcept>

namespace HitZy
{
	#ifdef linux
	#define VERIFY_FILE(file_handle)    VERIFY_FILE_TO_CONSOLE(file_handle)
	#define VERIFY_JCFILE(file_handle)    VERIFY_JCFILE_TO_CONSOLE(file_handle)
	#define VERIFY_AUTO_PTR(auto_ptr)   VERIFY_AUTO_PTR_TO_CONSOLE( auto_ptr )
	#define VERIFY_PTR(ptr)             VERIFY_PTR_TO_CONSOLE(ptr)
	#endif 

	#ifdef _CONSOLE 
	#define VERIFY_FILE(file_handle)    VERIFY_FILE_TO_CONSOLE(file_handle)
	#define VERIFY_JCFILE(file_handle)    VERIFY_JCFILE_TO_CONSOLE(file_handle)
	#define VERIFY_AUTO_PTR(auto_ptr)   VERIFY_AUTO_PTR_TO_CONSOLE( auto_ptr )
	#define VERIFY_PTR(ptr)             VERIFY_PTR_TO_CONSOLE(ptr)
	#endif 


	#ifdef _WINDOWS
	#include <windows.h>
	#include <stdlib.h>
	#define VERIFY_FILE(file_handle)    VERIFY_FILE_TO_DEBUGWINDOW(file_handle)
	#define VERIFY_JCFILE(file_handle)    VERIFY_JCFILE_TO_DEBUGWINDOW(file_handle)
	#define VERIFY_AUTO_PTR(auto_ptr)   VERIFY_AUTO_PTR_TO_DEBUGWINDOW( auto_ptr)
	#define VERIFY_PTR(ptr)             VERIFY_PTR_TO_DEBUGWINDOW(ptr)
	#endif
}



#include <iostream>
using namespace std;

/////////////////////////////////////////////////////////////////

#define VERIFY_FILE_TO_CONSOLE( file_handle) \
	if(file_handle.bad()||file_handle.fail()) \
	{ \
		cerr<<"file open failure"<<endl; \
		cerr<<__FILE__<<"  Line:"<<__LINE__<<endl; \
		throw runtime_error("file open failure"); \
	}

/////////////////////////////////////////////////////////////////

#define VERIFY_JCFILE_TO_CONSOLE(file_handle) \
	if(file_handle == NULL) \
	{ \
		cerr<<"file open failure"<<endl; \
		cerr<<__FILE__<<"  Line:"<<__LINE__<<endl; \
		throw runtime_error("file open failure"); \
	}

/////////////////////////////////////////////////////////////////

#define VERIFY_FILE_TO_DEBUGWINDOW( file_handle) \
	if(file_handle.bad()||file_handle.fail()) \
	{ \
		OutputDebugString("file open failure"); \
		OutputDebugString(__FILE__); \
		char buffer[20]; \
		OutputDebugString(_itoa(__LINE__,buffer,10)); \
		throw runtime_error("file open failure"); \
	}


//////////////////////////////////////////////////////////////////
#define VERIFY_JCFILE_TO_DEBUGWINDOW( file_handle) \
	if(file_handle == NULL) \
	{ \
		OutputDebugString("file open failure"); \
		OutputDebugString(__FILE__); \
		char buffer[20]; \
		OutputDebugString(_itoa(__LINE__,buffer,10)); \
		throw runtime_error("file open failure"); \
	}


//////////////////////////////////////////////////////////////////

#define VERIFY_AUTO_PTR_TO_CONSOLE( auto_ptr ) \
	if(auto_ptr.get() == NULL)\
	{ \
		cerr<<"mem alloc failure"<<endl; \
		cerr<<__FILE__<<"  Line:"<<__LINE__<<endl; \
		throw bad_alloc("mem alloc failure"); \
	}

////////////////////////////////////////////////////////////////////

#define VERIFY_AUTO_PTR_TO_DEBUGWINDOW( auto_ptr) \
	if(auto_ptr.get() == NULL) \
	{ \
		OutputDebugString("mem alloc failure"); \
		OutputDebugString(__FILE__); \
		char buffer[20]; \
		OutputDebugString(_itoa(__LINE__,buffer,10)); \
		throw bad_alloc("mem alloc failure"); \
	}


//////////////////////////////////////////////////////////////////

#define VERIFY_PTR_TO_CONSOLE( ptr ) \
	if(ptr == NULL)\
	{ \
		cerr<<"mem alloc failure"<<endl; \
		cerr<<__FILE__<<"  Line:"<<__LINE__<<endl; \
		throw bad_alloc("mem alloc failure"); \
	}

////////////////////////////////////////////////////////////////////

#define VERIFY_PTR_TO_DEBUGWINDOW( ptr) \
	if(ptr == NULL) \
	{ \
		OutputDebugString("mem alloc failure"); \
		OutputDebugString(__FILE__); \
		char buffer[20]; \
		OutputDebugString(_itoa(__LINE__,buffer,10)); \
		throw bad_alloc("mem alloc failure"); \
	}

#endif