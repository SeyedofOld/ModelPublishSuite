/********************************************************************
*	Created:	2010/09/18
*	Filename:	tlCSingleton.h
*	Author:		Sepehr Taghdissian (sepehr@tochalco.com)
*	Usage:		Inherit any Singleton class from this Template (with _T=class name)
*				add 'friend class Singleton<_ClassName>;' to the newly created class
*				Define Constructor and Destructor in 'private' section
*	
*	Purpose:	Singleton Class
*********************************************************************/

#pragma once

//#include "tlCMem.h"

template <typename _T>
class CSingleton
{
public:
	static _T* GetInstance();
	static void DestroyInstance();
	static void CreateInstance();

protected:
	CSingleton();
	virtual ~CSingleton();

private:
	CSingleton( const CSingleton<_T>& );

private:
	static _T*		s_pInstance;
};

////////////////////////////////////////////////////////////////////////////////
template <typename _T> _T* CSingleton<_T>::s_pInstance = NULL;

////////////////////////////////////////////////////////////////////////////////
template <typename _T>
_T* CSingleton<_T>::GetInstance()
{
// 	if( s_pInstance == NULL )		{
// 		s_pInstance = new _T;
// 	}

	return s_pInstance;
}

template <typename _T>
void CSingleton<_T>::DestroyInstance()
{
	if( s_pInstance != NULL )	{
		delete s_pInstance;
		s_pInstance = NULL;
	}
}

template <typename _T>
void CSingleton<_T>::CreateInstance()
{
	if( s_pInstance == NULL )		{
		s_pInstance = new _T;
	}
}

template <typename _T>
CSingleton<_T>::CSingleton()
{

}

template <typename _T>
CSingleton<_T>::~CSingleton()
{

}
