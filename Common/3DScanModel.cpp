/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanModel.h"

void FreeModel ( TD_SCAN_MODEL& model )
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
			subset.iTriCount = 0 ;
		}
		part.sName = "" ;
		part.Subsets.clear() ;
	}
	model.Parts.clear() ;
}