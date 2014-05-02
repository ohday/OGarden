#ifndef OHDAY_UTILITIES
#define OHDAY_UTILITIES

// STD
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define PATH_MIN_LENGTH 0.2f

#define FLOAT_PI 3.141592653f

namespace ohday
{	
	const DWORD FVFVER = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	const DWORD FVFLEAF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX4;

	struct OVertex
	{
		float x_, y_, z_;
		float nx_, ny_, nz_;
		float u_, v_;
		OVertex(){x_ = y_ = z_ = nx_ = ny_ = nz_ = u_ = v_ = 0;}

		OVertex(float x, float y, float z)
		{
			x_ = x;	y_ = y; z_ = z;
			nx_ = ny_ = nz_ = u_ = v_ = 0;
		}

		void SetNormals(float nx, float ny, float nz)
		{
			nx_ = nx;	ny_ = ny; nz_ = nz;
		}

		void SetTextureCoords(float u, float v)
		{
			u_ = u;	v_ = v;
		}
	};

	struct OLeafVertex
	{
		float cx_, cy_, cz_;	// center
		float nx_, ny_, nz_;	// normal
		
		float u_, v_;			// uv
		float alpha_, scalar_;	// rotation angle

		float ovx_, ovy_;		
		float ovz_, beta_;
	};



	struct OMaterial
	{
		float emissive_[4];
		float ambient_[4];
		float diffuse_[4];
		float specular_[4];
		float shinines_;
	};

	struct OTexture
	{
		IDirect3DTexture9* texture0_;
		IDirect3DTexture9* texture1_;

		OTexture()
		{
			texture0_ = NULL;
			texture1_ = NULL;
		}
	};


	struct OMaterialGroup
	{
		int materialId_;
		int textureId_;
		int indexStart_;
		int indexLength_;
	};

	struct OLight
	{
		D3DXVECTOR4 ambient_;
		D3DXVECTOR4 color_;
		D3DXVECTOR4 direction_;
	};

	struct OInstance
	{
		D3DXMATRIX transform_;
	};

	struct OMesh
	{
		IDirect3DVertexBuffer9 * vertexBuffer_;
		IDirect3DIndexBuffer9 * indexBuffer_;

		vector<OMaterial> materials_;
		vector<OTexture> textures_;
		vector<OMaterialGroup> materialGroups_;
		vector<OInstance> instances_;


		int materialIndex_;

		int leafIndex_;


		int numVertex_;
		int numFace_;
	};

	struct OSky
	{
		IDirect3DVertexBuffer9 * vertexBuffer_;
		vector<IDirect3DCubeTexture9*> textures_;
	};

	struct OLeafMotion
	{
//		float pathS_, pathE_;
		float delayTime_;		
		
		float rollW_;

		float rotW_;

		float yV_;

		float xPhi_, xW_, xScaler_;
		float zPhi_, zW_, zScaler_;
	};


	struct OLeaves
	{
		vector<OLeafVertex> originalVertices_;

		IDirect3DVertexBuffer9 * vertexBuffer_;
		IDirect3DIndexBuffer9 * indexBuffer_;
		

		const aiMesh *mesh_;

		int numVertex_;
		int numIndex_;
		int numLeaves_;

		int meshIndex_;


		vector<OLeafMotion> leafMotionParameters_;
	};

	class RandomDevice
	{

	private:
		int gaussPhase_;
		int seed_;
	public:
		enum e_random_type
		{
			_linear = 0,
			_gaussian
		};
		RandomDevice()
		{
			gaussPhase_ = 0;
			ResetSeed();
		}

		void ResetSeed()
		{
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			seed_ = sys.wMilliseconds;

			srand(seed_);
		}

		float GetFloatLine(float f_min, float f_max)
		{
			int ra = rand() % RAND_MAX;
			float rand_res = f_min + (f_max - f_min) * float(rand()) / float(RAND_MAX);
			return rand_res;
		}

		float GetFloatGauss(float f_E, float f_S)
		{
			float u1,u2,v1,v2,s,x;
			do 
			{
				u1 = (float)rand() / RAND_MAX;
				u2 = (float)rand() / RAND_MAX;

				v1 = 2 * u1 - 1;
				v2 = 2 * u2 - 1;

				s = v1 * v1 + v2 * v2;
			} while (s >= 1 || s == 0);

			if(!gaussPhase_)
				x = v1 * sqrt(-2 * log(s) / s);
			else
				x = v2 * sqrt(-2 * log(s) / s);

			gaussPhase_ = 1 - gaussPhase_;

			x = x * f_S + f_E;
			return x;
		}



		

	};

	class OWind
	{
	public:

		float strengthScaler_;
		float timeDelta_;
		float preStrength_;
		float nextStrength_;
		int currentStage_;

		RandomDevice randomDevice_;

		D3DXVECTOR3 dir_;


		OWind()
		{
			strengthScaler_ = 1;
			timeDelta_ = 5;

			preStrength_ = 0;

			nextStrength_ = randomDevice_.GetFloatLine(0, 5);

			dir_ = D3DXVECTOR3(1, 0, 0);
		}
		~OWind()
		{

		}


		float GetStrength(float timeCurrent)
		{
			int c = int(timeCurrent / timeDelta_);
			float lerp = 1 - (timeCurrent - c * timeDelta_) / timeDelta_;

			if(c != currentStage_)
			{
				currentStage_ = c;

				preStrength_ = nextStrength_;
				nextStrength_ = randomDevice_.GetFloatLine(0, 5);
			}

			float ret = preStrength_ * lerp + nextStrength_ * (1-lerp);

			return strengthScaler_ * ret;
		}
		
		

	};


};



#endif