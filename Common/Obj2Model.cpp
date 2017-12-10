/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Convert Obj data structure to 3d scan model structure
*********************************************************************/

#pragma once

#include <stdint.h>
#include "Obj2Model.h"

bool ConvertObjTo3DModel ( MY_OBJ& obj, TDSCAN_MODEL& model )
{
	for ( uint32_t i = 0 ; i < obj.subParts.size() ; i++ ) {
		MY_OBJ_PART& objpart = obj.subParts [ i ] ;

		for ( uint32_t j = 0 ; j < objpart.subSets.size() ; j++ ) {
			MY_DRAW_BATCH& subset = objpart.subSets [ j ] ;

		}
		
	}

	return true ;
}
