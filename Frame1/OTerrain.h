#ifndef OHDAY_TERRAIN
#define OHDAY_TERRAIN

#include "OUtilities.h"

namespace ohday
{
	const int c_terrain_size = 100;

	class OTerrain
	{
	public:
		OTerrain();
		~OTerrain();

		bool LoadHeightData(const char* filename);
		bool CreateTerrainMesh(IDirect3DDevice9 *device);
		bool LoadTexture(IDirect3DDevice9 *device, const char* filename);


		float height_[c_terrain_size * c_terrain_size];

		int num_vertex_;
		int num_index_;

		IDirect3DVertexBuffer9 * vertex_buffer_;
		IDirect3DIndexBuffer9 * index_buffer_;
		IDirect3DTexture9* texture_;

	};
}




#endif