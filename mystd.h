#pragma once

inline float frac( float n){ return n - floor(n); }

template<class T>
void SafeReleaseDelete( T* pSomething )
{
	if(pSomething)
	{
		pSomething->Release();
		delete pSomething;
		pSomething = NULL;
	}
}


template<class T>
void SafeDelete( T* pSomething )
{
	if( pSomething )
	{
		delete pSomething;
		pSomething = NULL;
	}
}

template<class T>
void SafeDeleteArray( T* pSomething )
{
	if( pSomething )
	{
		delete [] pSomething;
		pSomething = NULL;
	}
}
