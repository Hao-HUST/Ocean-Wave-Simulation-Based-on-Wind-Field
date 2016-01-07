#include "DXUT.h"
#include "Particle.h"


#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
Particle::Particle(void)
{
	ZeroMemory( this, sizeof( Particle ) );
}


Particle::~Particle(void)
{
}

RenderParticle	Particle::toRenderParticle( float timeStamp)
{
	RenderParticle rp;
	float liveTime = timeStamp - m_bornTime ;
	rp.pos = GetPos(timeStamp);
	rp.dir = D3DXVECTOR2( m_dir.x, m_dir.z);

	rp.amp	 = GetAmp(timeStamp);

	rp.size = m_size;

	return rp;
}




ParticleListArray::ParticleListArray( size_t ParticleCapacity )
{
	pParticleVB = new RenderParticle[ ParticleCapacity ];
	assert( pParticleVB );

	pArrayMem = new ArrayItem[ ParticleCapacity ];
	assert( pArrayMem );
	for( size_t i=0; i!= ParticleCapacity; ++i )
	{
		pArrayMem[i].next = i+1;
	}
	pArrayMem[ParticleCapacity-1].next = -1;

	particleCounts = 0;
	maxParticleCounts = ParticleCapacity;

	firstInUseIndex = -1;
	firstUnUseIndex = 0;
}

ParticleListArray::~ParticleListArray()
{
	if( pArrayMem )
	{
		delete [] pArrayMem;
	}

	if( pParticleVB )
	{
		delete [] pParticleVB;
	}
}

bool ParticleListArray::AddParticle( Particle& p )
{
	if( firstUnUseIndex == -1 ) 
	{
		OutputDebugStringA("ListArray over flow!");
		return false;
	}
	//the index of new item
	int newOneIndex = firstUnUseIndex;
	//update firstUnUseIndex
	firstUnUseIndex = pArrayMem[ firstUnUseIndex ].next;

	pArrayMem[newOneIndex].next = firstInUseIndex;
	pArrayMem[newOneIndex].item = p;

	firstInUseIndex = newOneIndex;
	return true;
}

bool ParticleListArray::AppendRenderParticle(RenderParticle rp )
{
	 if( particleCounts+1< maxParticleCounts )
	 {
		 pParticleVB[ particleCounts++ ] = rp;
		 return true;
	 } 
	 return false;
}
void ParticleListArray::IterAllParitcle( float tempTimeStamp , float top, float bottom, float left, float right )
{
	int* fromAccess = &firstInUseIndex;
	int  toIndex = *fromAccess;

	particleCounts = 0;
	while( toIndex != -1 )
	{
		int tempIndex = toIndex;
		float width = right - left;
		float height = top - bottom;

		if( pArrayMem[tempIndex].item.Isalive(tempTimeStamp))
		{
			//do some things, and  add to renderlist
			 Particle& tempParticle =  pArrayMem[tempIndex].item;
			
 			 Particle mirrorParticle;
 			 mirrorParticle = tempParticle;
			 D3DXVECTOR3 offset[8] = { D3DXVECTOR3(width,0,0), D3DXVECTOR3(-width,0,0), D3DXVECTOR3(0,0,height),D3DXVECTOR3(0,0,-height),
									   D3DXVECTOR3(width,0,height), D3DXVECTOR3(-width,0,height), D3DXVECTOR3(width,0,-height),D3DXVECTOR3(-width,0,-height),};
			 for( int i=0; i!=8 ; ++i)
			 {
				 mirrorParticle.m_pos = tempParticle.m_pos + offset[i];
				 if( checkInArea(mirrorParticle, tempTimeStamp, top, bottom, left, right) )
					 AppendRenderParticle(  mirrorParticle.toRenderParticle( tempTimeStamp ) );
			 }

			if( checkInArea(tempParticle,tempTimeStamp, top, bottom, left, right ) )
				AppendRenderParticle(  tempParticle.toRenderParticle( tempTimeStamp ) );
			fromAccess = &pArrayMem[tempIndex].next;
			toIndex = *fromAccess;
		}
		else 
		{
			*fromAccess = pArrayMem[ tempIndex ].next;
			toIndex = *fromAccess;

			pArrayMem[ tempIndex ].next = firstUnUseIndex;
			firstUnUseIndex = tempIndex;
		}
	}
}





ParticleGen::ParticleGen( void )
{
	m_pos = D3DXVECTOR3(0,0,0);
	m_period = 1.0;
	m_time_reg = m_period;
	float angle = rand()%180*3.14f/360.0f;
	m_dir = D3DXVECTOR3(0,0,1);


}



void ParticleGen::GenWaveFromWind(float timeStamp, float detaTime , ParticleListArray* pPLA)
{
	assert( pPLA );

	if( m_time_reg < m_period )
	{
		m_time_reg += detaTime;
		return;
	}
	else
	{
		m_time_reg = 0;
	}

	float angle = (rand()%180+1)*3.14f/360.0f;
	m_dir  = D3DXVECTOR3(0, 0,1);

	int sign = rand()&1?-1:1;
	Particle p;
	p.m_size= m_WaveParam.size;
	p.m_pos = m_pos ;
	p.m_bornTime = timeStamp;
	p.m_deadTime = timeStamp+m_WaveParam.liveTime;
	p.m_dir = m_WaveParam.direction;
	p.m_speed = m_WaveParam.speed;
	p.m_amp = 0.6;
	p.m_split_angle = m_dir_angle;
	p.m_period = m_WaveParam.liveTime/4;
	p.m_radius = m_WaveParam.radius; 
	m_period = m_WaveParam.liveTime/2;

    pPLA->AddParticle( p );

	Particle p_bro1, p_bro2;
	p_bro1 = p;
	p_bro2 = p;
	float deta_radius = m_WaveParam.size/12.0f/30;
	int i=0;
	float radius_18 = 3.1416f/10.0f;
	while( i*deta_radius < radius_18*2 )
	{
		++i;

		p.m_amp = p.m_amp/3;
		D3DXMATRIX  transMatrix;
		D3DXMatrixRotationY( &transMatrix, i*deta_radius);
		D3DXVECTOR3 vec3FromDir = p.m_dir;
		D3DXVECTOR4 vec4FromDir( vec3FromDir.x, vec3FromDir.y, vec3FromDir.z, 1 );
		D3DXVECTOR4 vec4ToDir;
		D3DXVec4Transform( &vec4ToDir, &vec4FromDir, &transMatrix );
		D3DXVECTOR3 vec3Dir = D3DXVECTOR3( vec4ToDir.x , vec4ToDir.y, vec4ToDir.z );
		D3DXVECTOR3 n_vecDir;
		D3DXVec3Normalize( &n_vecDir, &vec3Dir );
		p_bro1.m_dir = n_vecDir;
		p_bro1.m_amp = p.m_amp;


		D3DXMatrixRotationY( &transMatrix, -i*deta_radius);
		D3DXVec4Transform( &vec4ToDir, &vec4FromDir, &transMatrix );
		vec3Dir = D3DXVECTOR3( vec4ToDir.x , vec4ToDir.y, vec4ToDir.z );
		D3DXVec3Normalize( &n_vecDir, &vec3Dir );
		p_bro2.m_dir = n_vecDir;
		p_bro2.m_amp = p.m_amp;



	}

}


bool ParticleListArray::checkInArea(const Particle& p, float tempTimeStamp, float top, float bottom, float left, float right)
{
	D3DXVECTOR2 bl,br,tl,tr;
	D3DXVECTOR3 temp_Pos =  p.GetPos(tempTimeStamp);
	tl.x = bl.x = temp_Pos.x - 0.5f*p.m_size;
	tr.x = br.x = temp_Pos.x + 0.5f*p.m_size;

	tl.y = tr.y = temp_Pos.y + 0.5f*p.m_size;
	bl.y = br.y = temp_Pos.y - 0.5f*p.m_size;

	if( tl.x>right && tr.x>right && bl.x>right &&br.x>right)
		return false;

	if( tl.x<left && tr.x<left && bl.x<left &&br.x<left)
		return false;

	if( tl.y>top && tr.y>top && bl.y>top &&br.y>top)
		return false;

	if( tl.y<bottom && tr.y<bottom && bl.y<bottom &&br.y<bottom)
		return false;

	return true;
}