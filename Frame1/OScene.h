#ifndef OHDAY_SCENE
#define OHDAY_SCENE

//#include "OMesh.h"
#include "OUtilities.h"
#include "OTerrain.h"
#include <fbxsdk.h>


namespace ohday
{
	class OScene
	{
	public:
		OScene(void);
		~OScene(void);

		OTerrain * terrain_;

		OSky sky_;
		vector<OMesh> meshes_;
		vector<OMaterial> materials_;
		vector<OTexture> textures_;

		vector<OLeaves> leaves_;

		vector<vector<float>> leaf_path_;
		
		vector<bool> bLeaves_;

		const aiScene * scene_;

		bool ImportScene(const char* filename, IDirect3DDevice9 *device);

		bool ImportTerrain(IDirect3DDevice9* device, const char* heightfile, const char* texturefile);

		bool InitialSky(const char* filename, IDirect3DDevice9 *device);

	private:
	};
}


#endif
