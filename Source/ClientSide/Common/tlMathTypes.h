/********************************************************************
*	Created:	2010/09/18
*	Filename:	tlTypes.h
*	Author:		Sepehr Taghdissian (sepehr@tochalco.com), modified by
*				Ali Seyedof on 2010/10/01
*	Purpose:	Basic Engine Types
*********************************************************************/

#pragma once

#pragma warning( push )  
#pragma warning(disable:4005)
#include <d3dx9.h>
#pragma warning( pop )  

////////////////////////////////////////////////////////////////////////////////
namespace tochal {

// Math Types
typedef struct {
	float x,y;
} _vector2;

typedef struct {
	float x,y,z;
} _vector3;

typedef struct {
	float x,y,z,w;
} _vector4;

typedef D3DXVECTOR2			vector2;
typedef D3DXVECTOR3			vector3;
typedef D3DXVECTOR4			vector4;
typedef D3DXQUATERNION		quaternion;
typedef D3DXCOLOR			color;
typedef D3DXPLANE			plane;
typedef D3DXMATRIX			matrix;

// 16byte aligned Matrix, overrided from MATRIX
/*
TOCHAL_ALIGN16 struct MATRIX : public D3DXMATRIX
{
public:
	MATRIX()
	{
		_11 = 1.0f;			_12 = 0.0f;			_13 = 0.0f;			_14 = 0.0f;
		_21 = 0.0f;			_22 = 1.0f;			_23 = 0.0f;			_24 = 0.0f;
		_31 = 0.0f;			_32 = 0.0f;			_33 = 1.0f;			_34 = 0.0f;
		_41 = 0.0f;			_42 = 0.0f;			_43 = 0.0f;			_44 = 1.0f;
	}

	MATRIX( const float* f )
	{
		_11 = f[0];			_12 = f[1];			_13 = f[2];			_14 = f[3];
		_21 = f[4];			_22 = f[5];			_23 = f[6];			_24 = f[7];
		_31 = f[8];			_32 = f[9];			_33 = f[10];		_34 = f[11];
		_41 = f[12];		_42 = f[13];		_43 = f[14];		_44 = f[15];
	}

	MATRIX( const MATRIX& m )
	{
		_11 = m._11;		_12 = m._12;		_13 = m._13;		_14 = m._14;
		_21 = m._21;		_22 = m._22;		_23 = m._23;		_24 = m._24;
		_31 = m._31;		_32 = m._32;		_33 = m._33;		_34 = m._34;
		_41 = m._41;		_42 = m._42;		_43 = m._43;		_44 = m._44;
	}

	MATRIX( float _11, float _12, float _13, float _14,
			float _21, float _22, float _23, float _24,
			float _31, float _32, float _33, float _34,
			float _41, float _42, float _43, float _44 )
	{
		_11 = _11;		_12 = _12;		_13 = _13;		_14 = _14;
		_21 = _21;		_22 = _22;		_23 = _23;		_24 = _24;
		_31 = _31;		_32 = _32;		_33 = _33;		_34 = _34;
		_41 = _41;		_42 = _42;		_43 = _43;		_44 = _44;
	}

	MATRIX& operator=( const MATRIX& m )
	{
		_11 = m._11;		_12 = m._12;		_13 = m._13;		_14 = m._14;
		_21 = m._21;		_22 = m._22;		_23 = m._23;		_24 = m._24;
		_31 = m._31;		_32 = m._32;		_33 = m._33;		_34 = m._34;
		_41 = m._41;		_42 = m._42;		_43 = m._43;		_44 = m._44;
		
		return *this;
	}

	MATRIX& operator=( const MATRIX& m )
	{
		_11 = m._11;		_12 = m._12;		_13 = m._13;		_14 = m._14;
		_21 = m._21;		_22 = m._22;		_23 = m._23;		_24 = m._24;
		_31 = m._31;		_32 = m._32;		_33 = m._33;		_34 = m._34;
		_41 = m._41;		_42 = m._42;		_43 = m._43;		_44 = m._44;

		return *this;
	}
};*/

}
