#include "FBXImporter.h"


FBXImporter::FBXImporter(void)
{
	manager_ = NULL;
	scene_ = NULL;
}


FBXImporter::~FBXImporter(void)
{
}

bool FBXImporter::Initialize()
{
	if(manager_)
		manager_->Destroy();

	manager_ = FbxManager::Create();

	if(manager_ == NULL)
		return false;

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(manager_, IOSROOT);
	manager_->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	manager_->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	scene_ = FbxScene::Create(manager_, "My Scene");
	if(!scene_)
		return false;

	return true;
}

bool FBXImporter::LoadScene(const char* filename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor,  lSDKMinor,  lSDKRevision;

	int i, lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(manager_,"");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(filename, -1, manager_->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if(!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
			printf("FBX file format version for file '%s' is %d.%d.%d\n\n", filename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		printf("FBX file format version for file '%s' is %d.%d.%d\n\n", filename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		printf("Animation Stack Information\n");

		lAnimStackCount = lImporter->GetAnimStackCount();

		printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
		printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
		printf("\n");

		for(i = 0; i < lAnimStackCount; i++)
		{
			FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			printf("    Animation Stack %d\n", i);
			printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
			printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		
		
	}

	// Import the scene.
	lStatus = lImporter->Import(scene_);

	if(lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
	{
		printf("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		manager_->GetIOSettings()->SetStringProp(IMP_FBX_PASSWORD,      lString);
		manager_->GetIOSettings()->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(scene_);

		if(lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			printf("\nPassword is wrong, import aborted.\n");
		}
	}

	// Destroy the importer.
	lImporter->Destroy();


	return lStatus;
}

bool FBXImporter::GetFirstMesh(OBase &mesh)
{
	if(!scene_)
		return false;

	FbxNode * root_node = scene_->GetRootNode();

//	if(root_node->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eMesh)
//		return false;

	FbxNode * node = root_node->GetChild(0);
	return ProcessMesh(node, mesh);

}

bool FBXImporter::ProcessMesh(FbxNode *node, OBase &oMesh)
{
	FbxMesh* fMesh = node->GetMesh();
	if(!fMesh)
		return false;

	oMesh.Clear();

	int num_ctrl_pt = fMesh->GetControlPointsCount();
	FbxVector4* ctrl_pts =  fMesh->GetControlPoints();

	oMesh.vertex_buffer_.resize(num_ctrl_pt);
	// vertex location
	for(int i = 0; i < num_ctrl_pt; i++)
	{
		oMesh.vertex_buffer_[i].x_ = ctrl_pts[i][0];
		oMesh.vertex_buffer_[i].y_ = ctrl_pts[i][1];
		oMesh.vertex_buffer_[i].z_ = ctrl_pts[i][2];
	}

	// index buffer
	int num_polygon = fMesh->GetPolygonCount();
	for(int i = 0; i < num_polygon; i++)
	{
		int size_polygon = fMesh->GetPolygonSize(i);
		for(int j = 0; j < size_polygon; j++)
		{
			int k = fMesh->GetPolygonVertex(i, j);
			oMesh.index_buffer_.push_back(k);
		}
	}

	// uv buffer
	int uvlayer = 0;
	FbxGeometryElementUV * pVertexUV = fMesh->GetElementUV(uvlayer);

	FbxLayerElement::EMappingMode mapping_mode =  pVertexUV->GetMappingMode();
	FbxLayerElement::EReferenceMode ref_mode = pVertexUV->GetReferenceMode();

	if(mapping_mode == FbxLayerElement::eByControlPoint)
	{
		for(int i = 0; i < num_ctrl_pt; i++)
		{
			if(ref_mode == FbxLayerElement::eDirect)
			{
				oMesh.vertex_buffer_[i].u_ = pVertexUV->GetDirectArray().GetAt(i)[0];
				oMesh.vertex_buffer_[i].v_ = pVertexUV->GetDirectArray().GetAt(i)[1];
			}
			else if(ref_mode == FbxLayerElement::eIndexToDirect)
			{
				int id = pVertexUV->GetIndexArray().GetAt(i);
				oMesh.vertex_buffer_[i].u_ = pVertexUV->GetDirectArray().GetAt(id)[0];
				oMesh.vertex_buffer_[i].v_ = pVertexUV->GetDirectArray().GetAt(id)[1];
			}			
		}
	}
	else if(mapping_mode == FbxLayerElement::eByPolygonVertex)
	{
		for(int i = 0; i < num_polygon; i++)
		{
			int size_polygon = fMesh->GetPolygonSize(i);
			for(int j = 0; j < size_polygon; j++)
			{
				int ctrl_index = fMesh->GetPolygonVertex(i, j);
				int uv_index = fMesh->GetTextureUVIndex(i, j);
				oMesh.vertex_buffer_[ctrl_index].u_ = pVertexUV->GetDirectArray().GetAt(uv_index)[0];
				oMesh.vertex_buffer_[ctrl_index].v_ = pVertexUV->GetDirectArray().GetAt(uv_index)[1];
			}

		}
	}

	// normal buffer
	FbxGeometryElementNormal * pNormal = fMesh->GetElementNormal(0);
	mapping_mode = pNormal->GetMappingMode();
	ref_mode = pNormal->GetReferenceMode();

	if(mapping_mode == FbxLayerElement::eByControlPoint)
	{
		for(int i = 0; i < num_ctrl_pt; i++)
		{
			if(ref_mode == FbxLayerElement::eDirect)
			{
				oMesh.vertex_buffer_[i].nx_ = pNormal->GetDirectArray().GetAt(i)[0];
				oMesh.vertex_buffer_[i].ny_ = pNormal->GetDirectArray().GetAt(i)[1];
				oMesh.vertex_buffer_[i].nz_ = pNormal->GetDirectArray().GetAt(i)[2];
			}
			else if(ref_mode == FbxLayerElement::eIndexToDirect)
			{
				int id = pNormal->GetIndexArray().GetAt(i);
				oMesh.vertex_buffer_[i].nx_ = pNormal->GetDirectArray().GetAt(id)[0];
				oMesh.vertex_buffer_[i].ny_ = pNormal->GetDirectArray().GetAt(id)[1];
				oMesh.vertex_buffer_[i].nz_ = pNormal->GetDirectArray().GetAt(id)[2];
			}			
		}
	}
	else if(mapping_mode == FbxLayerElement::eByPolygonVertex)
	{
		int vertex_counter = 0;
		for(int i = 0; i < num_polygon; i++)
		{
			int size_polygon = fMesh->GetPolygonSize(i);
			for(int j = 0; j < size_polygon; j++)
			{
				int ctrl_index = fMesh->GetPolygonVertex(i, j);

				if(ref_mode == FbxLayerElement::eDirect)
				{
					oMesh.vertex_buffer_[ctrl_index].nx_ = pNormal->GetDirectArray().GetAt(vertex_counter)[0];
					oMesh.vertex_buffer_[ctrl_index].ny_ = pNormal->GetDirectArray().GetAt(vertex_counter)[1];
					oMesh.vertex_buffer_[ctrl_index].nz_ = pNormal->GetDirectArray().GetAt(vertex_counter)[2];
				}
				else
				{
					int id = pNormal->GetIndexArray().GetAt(vertex_counter);
					oMesh.vertex_buffer_[ctrl_index].nx_ = pNormal->GetDirectArray().GetAt(id)[0];
					oMesh.vertex_buffer_[ctrl_index].ny_ = pNormal->GetDirectArray().GetAt(id)[1];
					oMesh.vertex_buffer_[ctrl_index].nz_ = pNormal->GetDirectArray().GetAt(id)[2];
				}

				vertex_counter++;
			}

		}
	}

	// material indices
	oMesh.material_indices_.resize(num_polygon);		
	if(fMesh->GetElementMaterial())
	{
		FbxLayerElementArrayTemplate<int>* pMaterialIndices = &fMesh->GetElementMaterial()->GetIndexArray();
		FbxGeometryElement::EMappingMode materialMappingMode = fMesh->GetElementMaterial()->GetMapingMode();

		if(pMaterialIndices)
		{
			if(materialMappingMode == FbxGeometryElement::eByPolygon)
			{
				for(int i = 0; i < num_polygon; i++)
				{
					int mid = pMaterialIndices->GetAt(i);
					oMesh.material_indices_[i] = mid;
				}
			}
			else if(materialMappingMode == FbxGeometryElement::eAllSame)
			{
				int mid = pMaterialIndices->GetAt(0);
				for(int i = 0; i < num_polygon; i++)
					oMesh.material_indices_[i] = mid;
			}
		}
	}

	
	// materials
	int material_count = node->GetMaterialCount();
	oMesh.materials_.resize(material_count);

	for(int i = 0; i < material_count; i++)
	{
		FbxSurfaceMaterial * pSurfaceMaterial = node->GetMaterial(i);

		FbxSurfacePhong* phong = (FbxSurfacePhong*)pSurfaceMaterial;
		
		FbxDouble3 d = phong->Ambient;
		oMesh.materials_[i].ambient_[0] = d[0];
		oMesh.materials_[i].ambient_[1] = d[1];
		oMesh.materials_[i].ambient_[2] = d[2];

		d = phong->Emissive;
		oMesh.materials_[i].emissive_[0] = d[0];
		oMesh.materials_[i].emissive_[1] = d[1];
		oMesh.materials_[i].emissive_[2] = d[2];

		d = phong->Diffuse;
		oMesh.materials_[i].diffuse_[0] = d[0];
		oMesh.materials_[i].diffuse_[1] = d[1];
		oMesh.materials_[i].diffuse_[2] = d[2];

		d = phong->Specular;
		oMesh.materials_[i].specular_[0] = d[0];
		oMesh.materials_[i].specular_[1] = d[1];
		oMesh.materials_[i].specular_[2] = d[2];

		FbxDouble s = phong->Shininess;
		oMesh.materials_[i].shinines_ = s;	
	}

	return true;
}