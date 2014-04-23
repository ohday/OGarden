#include "OBase.h"

namespace ohday
{
	OBase::OBase()
	{
		d3d_vertex_buffer_ = NULL;
		d3d_index_buffer_ = NULL;
	}

	OBase::~OBase()
	{

	}

	void OBase::Clear()
	{
		vertex_buffer_.clear();
		index_buffer_.clear();
		instances_.clear();
	}

	void OBase::InitialBuffers(IDirect3DDevice9 * device)
	{
		int num_vertex = vertex_buffer_.size();
		device->CreateVertexBuffer(
			num_vertex * sizeof(OVertex),
			D3DUSAGE_WRITEONLY,
			FVFVER,
			D3DPOOL_DEFAULT,
			&d3d_vertex_buffer_,
			0);

		OVertex *v = NULL;

		d3d_vertex_buffer_->Lock(0, 0, (void**)&v, 0);
		for(int i = 0; i < num_vertex; i++)
			v[i] = vertex_buffer_[i];
		d3d_vertex_buffer_->Unlock();


		int num_index = index_buffer_.size();
		device->CreateIndexBuffer(
			num_index * sizeof(WORD),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&d3d_index_buffer_,
			0);

		WORD * ins = NULL;
		d3d_index_buffer_->Lock(0, 0, (void**)&ins, 0);
		for(int i = 0; i < num_index; i++)
			ins[i] = index_buffer_[i];
		d3d_index_buffer_->Unlock();
	}
}