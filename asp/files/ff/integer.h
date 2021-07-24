/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER

#if 0
#include <windows.h>
#else
#include <itron.h>

#if 0	/* ROI DEBUG */
/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;
#endif	/* ROI DEBUG */

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

#if 0	/* ROI DEBUG */
/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;
#endif	/* ROI DEBUG */

#endif

#define _INTEGER
#endif
