/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Convert Obj data structure to 3d scan model structure
*********************************************************************/

#pragma once

#include "3DScanModel.h"
#include "ObjLoader.h"

bool ConvertObjTo3DModel ( MY_OBJ& obj, TDSCAN_MODEL& model ) ;