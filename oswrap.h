// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef _OSWRAP_H
#define _OSWRAP_H 

// 1) Define a macro to Get Environmental variables
//  char* getenv(const char* name);
#define OS_GETENV  getenv


// 2) Define a macro to log messages

#define DVB_INFO   1
#define DVB_DEBUG  2
#define DVB_ERROR  3
#define DVB_WARN   4
#define DVB_TRACE  5
#define DVB_TRACE1 6
#define DVB_TRACE2 7
#define DVB_TRACE3 8

#define OS_LOG(level,format,...) \
{ \
    fprintf(stderr, "%d:%s:",level,"INBSI"); \
    fprintf(stderr, format,__VA_ARGS__); \
}  

#endif // _OSWRAP_H

