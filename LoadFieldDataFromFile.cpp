// #include "stdafx.h"
// #include "LoadFieldDataFromFile.h"
// #include <fstream>
// #include <string>
// #include <vector>
// #include <iostream>
#include "DXUT.h"
#include "LoadFieldDataFromFile.h"
using namespace std;
typedef vector<vector<string> > vvs;


template <class T> 
void convertFromString(T &value, const std::string &s) {
	std::stringstream ss(s);
	ss >> value;
}

LoadFieldDataFromFile::LoadFieldDataFromFile(void)
{

	u_div = v_div =0;

}

LoadFieldDataFromFile::~LoadFieldDataFromFile(void)
{
	
}




void LoadFieldDataFromFile::Initialise(char* filename)
{	
	
	ifstream infile(filename);
	infile>>noskipws;
	int row =0;
	int col = 0;

	char chr;
	while(infile>>chr)
	{
		switch(chr)
		{
		case '\n':  
			++ row;  
			col = 0; 
			break;
		case ' ':  
			++ col;  
			break;
		default:;
		}
	}
	infile.close();  
	row ++; col++;  

	u_div = row;
	v_div = col;

	vvs data; 
	//vvs data_spilt;
	data.resize(row);
	for(int k = 0; k < row; k ++)
		data[k].resize(col);

	ifstream infile2(filename);
	size_t trow, tcol;
	trow = tcol = 0;
	string str;
	while(trow < row)
	{
		while(tcol < col-1)
		{
			getline(infile2,str,' '); 
			data[trow][tcol] = str;  
			tcol++;      
		}
		getline(infile2,str,'\n');  
		data[trow][tcol] = str;   
		tcol = 0;      
		trow ++;    
	}
	infile2.close();


	m_pdirection = new D3DXVECTOR2[row*col];
	assert(m_pdirection);
	for (size_t r = 0; r != row; ++r)
	{
		for (size_t c=0; c != col; ++c)
		{
			m_pdirection[r*col+c] = D3DXVECTOR2(data[r][c][0]-'0',data[r][c][2]-'0');
		
		}
	}

}

void LoadFieldDataFromFile::Release()
{

	if (m_pdirection)
	{
		delete [] m_pdirection;
	}
	
}

