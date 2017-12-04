/********************************************************************
*	Created:	2010/09/18
*	Filename:	tlTypes.h
*	Author:		Sepehr Taghdissian (sepehr@tochalco.com), modified by
*				Ali Seyedof on 2010/10/01
*	Purpose:	Basic Engine Types
*********************************************************************/

#pragma once

#include "tlMacros.h"

////////////////////////////////////////////////////////////////////////////////
// Basic Types
typedef unsigned char		byte;
typedef unsigned int		dword;
typedef unsigned int		uint;
typedef unsigned __int64	qword;
typedef WORD				word;

#define	FLT_INVALID					1e300				
#define IS_VALID_FLT(f)	(!(f>=FLT_INVALID))

#include <PshPack1.h>

struct TOCHAL_VERSION {
	union {
		struct {
			byte byMajor ;
			byte byMinor ;
			byte byBuild ;
			byte byRevision ;
		} ;
		dword dwVersion ;
	};
	TOCHAL_VERSION() {
		dwVersion = 0 ;
	};
};

struct TOCHAL_DATE {
	union {
		struct {
			byte byYear ;
			byte byMonth ;
			byte byDay ;
			byte byNone ;
		} ;
		dword dwDate ;
	};
	TOCHAL_DATE() {
		dwDate = 0 ;
	};
};

#include <PopPack.h>

////////////////////////////////////////////////////////////////////////////////
