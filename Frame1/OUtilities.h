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

#define PATH_MIN_LENGTH 0.2


namespace ohday
{	
	const DWORD FVFVER = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	const DWORD FVFLEAF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX4;

	const float c_leaf_v = 1.4f;
	const float c_leaf_roll = 0.5f;
	const float c_leaf_falling_time = 7.0f;

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
		int material_id_;
		int texture_id_;
		int index_start_;
		int index_length_;
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
		IDirect3DVertexBuffer9 * vertex_buffer_;
		IDirect3DIndexBuffer9 * index_buffer_;

		vector<OMaterial> materials_;
		vector<OTexture> textures_;
		vector<OMaterialGroup> material_groups_;
		vector<OInstance> instances_;


		int material_index_;

		int leaf_index_;


		int num_vertex_;
		int num_index_;
	};

	struct OSky
	{
		IDirect3DVertexBuffer9 * vertex_buffer_;
		vector<IDirect3DCubeTexture9*> textures_;
	};

	struct OLeafMotion
	{
		float path_s_, path_e_;
		float scalar1_, scalar2_,  scalar_roll_;
		float falling_v_;
		float rollingW_;
		float delayTime_;
	};


	struct OLeaves
	{
		vector<OLeafVertex> original_vertices_;

		IDirect3DVertexBuffer9 * vertex_buffer_;
		IDirect3DIndexBuffer9 * index_buffer_;
		

		const aiMesh *mesh_;

		int num_vertex_;
		int num_index_;
		int num_leaves_;

		vector<OLeafMotion> leaf_motion_parameters_;
	};

	class RandomDevice
	{

	private:
		int gauss_phase;
		int seed;
	public:
		enum e_random_type
		{
			_linear = 0,
			_gaussian
		};
		RandomDevice()
		{
			gauss_phase = 0;
			ResetSeed();
		}

		void ResetSeed()
		{
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			seed = sys.wMilliseconds;

			srand(seed);
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

			if(!gauss_phase)
				x = v1 * sqrt(-2 * log(s) / s);
			else
				x = v2 * sqrt(-2 * log(s) / s);

			gauss_phase = 1 - gauss_phase;

			x = x * f_S + f_E;
			return x;
		}



		

	};


};



#endif