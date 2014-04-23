#include "OTerrain.h"

namespace ohday
{
	OTerrain::OTerrain()
	{
		for(int i = 0; i < c_terrain_size * c_terrain_size; i++)
			height_[i] = 0;

		num_vertex_ = c_terrain_size * c_terrain_size;
		num_index_ = 6 * (c_terrain_size - 1) * (c_terrain_size - 1);


		vertex_buffer_ = NULL;
		index_buffer_ = NULL;
		texture_ = NULL;
	}

	OTerrain::~OTerrain()
	{
		
	}


	bool OTerrain::LoadHeightData(const char* filename)
	{
		return true;
	}

	bool OTerrain::LoadTexture(IDirect3DDevice9 *device, const char* filename)
	{
		HRESULT hr = D3DXCreateTextureFromFile(device, filename, &texture_);
		
		if(hr == S_OK)
			return true;
		return false;
	}


	bool OTerrain::CreateTerrainMesh(IDirect3DDevice9 *device)
	{
		//device->CreateIndexBuffer(
		//	num_face * 3 * sizeof(WORD),
		//	D3DUSAGE_WRITEONLY,
		//	D3DFMT_INDEX16,
		//	D3DPOOL_DEFAULT,
		//	&meshes_[i].index_buffer_,
		//	0);
		//device->CreateVertexBuffer(
		//	num_vertex * sizeof(OVertex),
		//	D3DUSAGE_WRITEONLY,
		//	FVFVER,
		//	D3DPOOL_DEFAULT,
		//	&meshes_[i].vertex_buffer_,
		//	0);
		device->CreateVertexBuffer(
			c_terrain_size * c_terrain_size * sizeof(OVertex),
			D3DUSAGE_WRITEONLY,
			FVFVER,
			D3DPOOL_DEFAULT,
			&vertex_buffer_,
			0);
		OVertex *v = NULL;
		vertex_buffer_->Lock(0, 0, (void**)&v, 0);

		// location
		for(int i = 0; i < c_terrain_size * c_terrain_size; i++)
		{
			int ir = i / c_terrain_size;
			int ic = i % c_terrain_size;

			v[i].x_ = ic - c_terrain_size / 2;
			v[i].y_ = height_[i];
			v[i].z_ = c_terrain_size / 2 - ir;			
		}

		// normal
		for(int i = 0; i < c_terrain_size * c_terrain_size; i++)
		{
			int ir = i / c_terrain_size;
			int ic = i % c_terrain_size;

			if(ir == 0 || ic == 0 || ir == c_terrain_size -1 || ic == c_terrain_size - 1)
			{
				v[i].nx_ = 0;
				v[i].ny_ = 1;
				v[i].nz_ = 0;
			}
			else
			{
				D3DXVECTOR3 v0(v[i].x_, v[i].y_, v[i].z_);

				D3DXVECTOR3 v1(v[i-1].x_, v[i-1].y_, v[i-1].z_);
				D3DXVECTOR3 v2(v[i-c_terrain_size].x_, v[i-c_terrain_size].y_, v[i-c_terrain_size].z_);
				D3DXVECTOR3 v3(v[i-c_terrain_size+1].x_, v[i-c_terrain_size+1].y_, v[i-c_terrain_size+1].z_);
				D3DXVECTOR3 v4(v[i+1].x_, v[i+1].y_, v[i+1].z_);
				D3DXVECTOR3 v5(v[i+c_terrain_size].x_, v[i+c_terrain_size].y_, v[i+c_terrain_size].z_);
				D3DXVECTOR3 v6(v[i+c_terrain_size-1].x_, v[i+c_terrain_size-1].y_, v[i+c_terrain_size-1].z_);

				D3DXVECTOR3 x1 = v1 - v0;
				D3DXVECTOR3 x2 = v2 - v0;
				D3DXVECTOR3 x3 = v3 - v0;
				D3DXVECTOR3 x4 = v4 - v0;
				D3DXVECTOR3 x5 = v5 - v0;
				D3DXVECTOR3 x6 = v6 - v0;

				D3DXVECTOR3 n1, n2, n3, n4, n5, n6;
				D3DXVec3Cross(&n1, &x1, &x2);
				D3DXVec3Cross(&n2, &x2, &x3);
				D3DXVec3Cross(&n3, &x3, &x4);
				D3DXVec3Cross(&n4, &x4, &x5);
				D3DXVec3Cross(&n5, &x5, &x6);
				D3DXVec3Cross(&n6, &x6, &x1);


				D3DXVECTOR3 n = n1 + n2 + n3 + n4 + n5 + n6, retn;
				D3DXVec3Normalize(&retn, &n);

				v[i].nx_ = retn[0];
				v[i].ny_ = retn[1];
				v[i].nz_ = retn[2];
			}
		}

		// uv
		for(int ir = 0; ir < c_terrain_size; ir++)
		{
			int tr = ir % 2;

			for(int ic = 0; ic < c_terrain_size; ic++)
			{
				int tc = ic % 2;
				int i = ir * c_terrain_size + ic;
				v[i].u_ = tc;
				v[i].v_ = tr;				
			}
		}

		vertex_buffer_->Unlock();


		// index buffer
		int num_index = (c_terrain_size-1) * (c_terrain_size-1) * 6;


		device->CreateIndexBuffer(
			num_index * sizeof(WORD),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&index_buffer_,
			0);

		WORD * ins;
		index_buffer_->Lock(0, 0, (void**)&ins, 0);
		int ct = 0;
		for(int ir = 0; ir < c_terrain_size - 1; ir++)
		{
			for(int ic = 0; ic < c_terrain_size - 1; ic++)
			{
				int i = ir * c_terrain_size + ic;

				int v0 = i, v1 = i + 1, v2 = i + c_terrain_size, v3 = i + c_terrain_size + 1;

				ins[ct] = v0;
				ins[ct + 1] = v1;
				ins[ct + 2] = v2;

				ins[ct + 3] = v2;
				ins[ct + 4] = v1;
				ins[ct + 5] = v3;
				ct += 6;
			}
		}
		index_buffer_->Unlock();
		return true;
	}
}