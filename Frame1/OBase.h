#ifndef OHDAY_BASE
#define OHDAY_BASE

#include "OUtilities.h"

namespace ohday
{
	class OBase
	{
	public:
		OBase();
		~OBase();
		void Clear();
		void InitialBuffers(IDirect3DDevice9 * device);

		vector<OVertex> vertex_buffer_;
		vector<int> index_buffer_;

		vector<OMaterial> materials_;
		vector<int> material_indices_;

		vector<D3DXMATRIX> instances_;

		IDirect3DVertexBuffer9* d3d_vertex_buffer_;
		IDirect3DIndexBuffer9* d3d_index_buffer_;

	};
}

#endif