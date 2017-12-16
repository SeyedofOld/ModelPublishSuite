#pragma once

// De-Allocation Macros
#define TOCHAL_RELEASE( pPtr )			if( pPtr != NULL )		{	pPtr->Release();		pPtr = NULL;	}
#define TOCHAL_DELETE( pPtr )			if( pPtr != NULL )		{	delete pPtr;			pPtr = NULL;	}
#define TOCHAL_DELETE_ARRAY( pPtr )		if( pPtr != NULL )		{	delete[] pPtr;			pPtr = NULL;	}

#define VALIDATE_RANGE( value, minValue, maxValue ) \
	if ( value < (minValue) ) \
		value = (minValue); \
	else if ( value > (maxValue) ) \
		value = (maxValue);

#define TOCHAL_INLINE __forceinline

#define TOCHAL_RGBA(r, g, b, a)			((DWORD)((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff)))
#define TOCHAL_COLOR(r, g, b, a)		TOCHAL_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#ifdef TOCHAL_IMPORT
	#define TOCHAL_DLL_IMPORT_EXPORT	__declspec( dllimport )
#else
	#define TOCHAL_DLL_IMPORT_EXPORT	__declspec( dllexport )
#endif

