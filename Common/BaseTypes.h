/********************************************************************
*	Created:	2010/09/18
*	Filename:	tlTypes.h
*	Author:		Sepehr Taghdissian (sepehr@tochalco.com), modified by
*				Ali Seyedof on 2010/10/01
*	Purpose:	Basic Engine Types
*********************************************************************/

#pragma once

//#include "tlMacros.h"

//////////////////////////////////////////////////////////////////////////////
//Basic Types
// typedef unsigned char		byte;
// typedef unsigned int		dword;
// typedef unsigned int		uint;
// typedef unsigned __int64	qword;
// typedef WORD				word;

#define	FLT_INVALID					1e300				
#define IS_VALID_FLT(f)	(!(f>=FLT_INVALID))

#include <PshPack1.h>

struct float3 {
	float x, y, z;
};

struct float2 {
	float x, y;
};

struct float2_uv {
	float u, v;
};

struct float4_rgba {
	float r, g, b, a;
};

#include <PopPack.h>

////////////////////////////////////////////////////////////////////////////////
