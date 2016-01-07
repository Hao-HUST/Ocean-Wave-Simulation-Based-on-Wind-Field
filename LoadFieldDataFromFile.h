#include <D3DX11.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include "DXUT.h"
#include "mystd.h"



#pragma once

class LoadFieldDataFromFile
{
public:
	LoadFieldDataFromFile();
	~LoadFieldDataFromFile(void);

	void Initialise(char* filename);
	void Release();



	size_t        GetUdiv()           {return u_div;}
	size_t        GetVdiv()           {return v_div;}
	D3DXVECTOR2*  GetDirection()      {
		return m_pdirection;
	}
    

private:
	size_t       u_div;
	size_t       v_div;
	D3DXVECTOR2* m_pdirection;

	void ReadDataRowAndCol(char* filename);
	void LoadDataToArray(char* filename, size_t u,size_t v);
};

