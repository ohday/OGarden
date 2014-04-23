#ifndef FBX_IMPORTER
#define FBX_IMPORTER

#include <fbxsdk.h>
#include "OBase.h"

using namespace ohday;

class FBXImporter
{
public:
	FBXImporter(void);
	~FBXImporter(void);

	bool Initialize();
	bool LoadScene(const char* filename);
	bool GetFirstMesh(OBase &mesh);
	bool ProcessMesh(FbxNode *node, OBase &oMesh);

private:

	FbxManager * manager_;
	FbxScene * scene_;


};

#endif