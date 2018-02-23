#ifndef MYWINAPI_DISABLE_STROFINT_BASETSD

typedef signed char INT8;
typedef short INT16;
typedef int INT32;
typedef __int64 INT64;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned __int64 UINT64;

/*
	Disable warning when compile with legacy WinSDK
	as legacy WinSDK wrongly `typedef long INT_PTR`.
*/
#pragma warning(disable : 4142)
#ifdef _WIN64
typedef __int64 INT_PTR;
typedef unsigned __int64 UINT_PTR;
#else /* 32-bit */
typedef int INT_PTR;
typedef unsigned int UINT_PTR;
#endif
#pragma warning(default : 4142)

#endif/*MYWINAPI_DISABLE_STROFINT_BASETSD*/
