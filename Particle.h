#pragma once
#include <D3DX11.h>
#include <MsXml.h>
#include "LoadFieldDataFromFile.h"
class ParticleListArray;

struct RenderParticle
{
	D3DXVECTOR3  pos;
	D3DXVECTOR2  dir;
	float        size;
	float        amp;
};


struct WaveParam
{
	float size;
	float liveTime;
	D3DXVECTOR3 direction;
	float speed;
	float amp;
	float period;
	float radius;
};

struct WindField
{
	D3DXVECTOR3 direction;
	D3DXVECTOR3 position;
};


class Particle
{
public:
	Particle(void);
	~Particle(void);
	void        	Update( float timeStamp, ParticleListArray* pPLA  );
	inline bool     Isalive( float timeStamp ){ return timeStamp<m_deadTime ? true : false ;}
	inline float    GetAmp( float timeStamp ){ return m_amp*sin(3.14159f*((timeStamp - m_bornTime)/(m_deadTime - m_bornTime)/*+0.5*/)); }
	inline float    GetSize(float timeStamp ) {return m_size*sin(3.14159f*((timeStamp - m_bornTime)/(m_deadTime - m_bornTime)));}

	RenderParticle  toRenderParticle( float timeStamp );
	inline D3DXVECTOR2 GetDir(float timeStamp)  const{return D3DXVECTOR2(-sin(m_split_angle+2*3.1415926*timeStamp/m_period),cos(m_split_angle+2*3.1415926*timeStamp/m_period));}
	inline D3DXVECTOR3 GetPos(float timeStamp)  const { return m_pos + m_dir*(m_speed*(timeStamp - m_bornTime)); }


public:
	D3DXVECTOR3  m_pos;
	D3DXVECTOR3  m_dir; 
	float        m_speed;
	float        m_amp; 
	float        m_size;
	float        m_bornTime;
	float        m_deadTime;
	float		 m_split_angle;

	float        m_period;
	float        m_radius;
};

class ParticleListArray
{
public:
	struct ArrayItem 
	{
		Particle  item;
		int       next;
	};

	ParticleListArray( size_t ParticleCapacity );
	~ParticleListArray();

	bool  AddParticle( Particle& p );
	bool  AppendRenderParticle( RenderParticle rp );
	void  IterAllParitcle( float tempTimeStamp ,float top, float bottom, float left, float right );

	inline size_t GetCapacity() const { return maxParticleCounts; }
	inline size_t GetParticleCounts() const { return particleCounts ; }
	inline RenderParticle* GetRenderParticleBuf() const { return pParticleVB; }

private:
	int firstInUseIndex;
	int firstUnUseIndex;
	size_t maxParticleCounts;
	size_t particleCounts;

	ArrayItem*          pArrayMem;
	RenderParticle*     pParticleVB;

	bool checkInArea( const Particle& p,float tempTimeStamp, float top, float bottom, float left, float right);
};



class ParticleGen 
{
public:



	 ParticleGen( );
	//~ParticleGen( );
	inline void SetPosition( D3DXVECTOR3 pos ){ m_pos = pos; }

	void        GenWaveFromWind(float timeStamp, float detaTime , ParticleListArray* pPLA);

	void        loadDataFromFile(WaveParam* m_WaveParam, const char* filename,size_t u_div, size_t v_div);
public:
	D3DXVECTOR3  m_pos;
	float        m_period;
	float        m_time_reg;
	D3DXVECTOR3  m_dir;
    WaveParam    m_WaveParam;

	float        m_dir_angle;

};


class ParticleSourceMan
{
public:
	ParticleSourceMan(void){ m_pPLA = NULL ;};
	virtual ~ParticleSourceMan(void){};


	int m_udiv;
	int m_vdiv;


protected:
	ParticleListArray* m_pPLA;
	ParticleGen*   m_pParticleGens;
	int            m_ParticleSourceCounts;
	D3DXVECTOR3    m_pos;
	D3DXVECTOR3    m_scale;

	

	virtual void UpdateParticle( float timeStamp, float detaTime ) = 0;	



public:
	void InitParticleGens( size_t maxParticleNum, size_t u_div, size_t v_div, D3DXVECTOR2 pos, D3DXVECTOR2 size, D3DXVECTOR2* WindField_direction)
	{
		assert( u_div );
		assert( v_div );
		m_pPLA = new ParticleListArray(maxParticleNum);
		assert( m_pPLA );

		m_ParticleSourceCounts = u_div*v_div;

		m_udiv = u_div;
		m_vdiv = v_div;
		

		m_pParticleGens = new ParticleGen[m_ParticleSourceCounts];
		assert( m_pParticleGens );


 		int index = 0;
		float* amp = new float[u_div];
		D3DXVECTOR3* direction = new D3DXVECTOR3[u_div];
		float angle = rand()%180*3.14f/360.0f;
		for (int i = 0;i<u_div;i++)
		{
			amp[i] = 1.0f;

		}



 		m_pos = D3DXVECTOR3( pos.x , 0.0 , pos.y ) ;
 		m_scale = D3DXVECTOR3( size.x, 1.0, size.y);
 
 		float u_step = 1.0f/(u_div+1);
 		float v_step = 1.0f/(v_div+1);
 		for( size_t r = 0; r != v_div; ++r )
 		{
 			for( size_t c=0; c != u_div; ++c )
 			{
 				D3DXVECTOR2 p( -0.5f+(c+1)*u_step, 0.5f-(r+1)*v_step );
 				m_pParticleGens[ r*u_div+c].m_pos = m_pos + D3DXVECTOR3(p.x*size.x, 0, p.y*size.y );


				float angle = rand()%90*3.14159/360;
                m_pParticleGens[ r*u_div+c ].m_WaveParam.direction = D3DXVECTOR3(WindField_direction[r*u_div+c].y,0,WindField_direction[r*u_div+c].x);				
				m_pParticleGens[r*u_div+c].m_WaveParam.size = 200.0f+rand()%30;
				m_pParticleGens[r*u_div+c].m_WaveParam.liveTime = 20.0f+rand()%5;
				m_pParticleGens[r*u_div+c].m_WaveParam.speed = 25+rand()%10;

 			}
 		}

	}
	
	inline D3DXVECTOR3 GetPos() { return m_pos; }
	inline D3DXVECTOR3 GetSize() {return m_scale; }
	inline size_t      GetMaxParticleNum() { return m_pPLA->GetCapacity(); }
	inline RenderParticle* GetRenderParticleBuf() {return m_pPLA->GetRenderParticleBuf();}
	inline size_t      GetParticleCounts(){ return m_pPLA->GetParticleCounts(); }

	

	void Release()
	{
		if( m_pParticleGens )
			delete [] m_pParticleGens;

		if( m_pPLA )
		{
			delete m_pPLA;
			m_pPLA = NULL;
		}

		m_pParticleGens = NULL;
		m_ParticleSourceCounts = 0;
	};

	void Update( float timeStamp, float detaTime )
	{
		UpdateParticle( timeStamp, detaTime );
		m_pPLA->IterAllParitcle( timeStamp, m_pos.z + 0.5f*m_scale.z, m_pos.z - 0.5f*m_scale.z, m_pos.x - 0.5f*m_scale.x, m_pos.x + 0.5f*m_scale.x );
	}
};


class  TestCase2:public ParticleSourceMan
{
	void UpdateParticle( float timeStamp, float detaTime )
	{

		for( int i=0; i<m_ParticleSourceCounts; ++i)
		{

			m_pParticleGens[i].GenWaveFromWind(timeStamp,detaTime,m_pPLA);
			
		}
	}
};


