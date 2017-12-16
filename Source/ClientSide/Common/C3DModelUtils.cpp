/********************************************************************
*	Created:	2017/12/13
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3d model utility class
*********************************************************************/

#pragma once

#include "C3DModelUtils.h"
#include "C3DScanFileUtils.h"

void C3DModelUtils::FreeModel ( TD_SCAN_MODEL& model )
{
	for ( uint32_t i = 0 ; i < model.Parts.size () ; i++ ) {
		TD_MODEL_PART& part = model.Parts [ i ] ;

		for ( uint32_t j = 0 ; j < part.Subsets.size () ; j++ ) {
			TD_MODEL_SUBSET& subset = part.Subsets [ j ] ;

			if ( subset.pVB )
				delete subset.pVB ;
			subset.pVB = NULL ;

			if ( subset.pIB )
				delete subset.pIB ;
			subset.pIB = NULL ;

			subset.sMatName = "" ;
			subset.uiVertexFmt = 0 ;
			subset.uiTriCount = 0 ;
		}
		part.sName = "" ;
		part.Subsets.clear() ;
	}
	model.Parts.clear() ;
}

