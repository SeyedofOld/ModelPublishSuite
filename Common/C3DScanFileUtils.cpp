#include "C3DScanFileUtils.h"
#include "3DScanFmt.h"
#include "BaseTypes.h"

uint32_t C3DScanFileUtils::GetVertexSize ( uint32_t uiVertexFmt )
{
	uint32_t uiSize = 0 ;
	if ( uiVertexFmt & VF_POSITIION )
		uiSize += sizeof(float3) ;
	if ( uiVertexFmt & VF_NORMAL )
		uiSize += sizeof(float3) ;
	if ( uiVertexFmt & VF_UV )
		uiSize += sizeof(float2) ;

	return uiSize ;
}
