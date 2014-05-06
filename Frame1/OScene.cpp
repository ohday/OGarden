#include "OScene.h"


namespace ohday
{
	OScene::OScene(void)
	{
		terrain_ = NULL;

		leafPath_.resize(2);
		
		ifstream f1("../data/leaf_path.txt");
		int n;
		f1 >> n;
		for(int i = 0; i < 2; i++)
		{
			leafPath_[i].resize(n);
			for(int j = 0; j < n; j++)
				f1 >> leafPath_[i][j];
		}
		f1.close();
	}


	OScene::~OScene(void)
	{
	}

	bool OScene::ImportScene(const char* filename, IDirect3DDevice9* device)
	{
		static int leaf_count = 0;

		scene_ = aiImportFile(filename, aiProcessPreset_TargetRealtime_MaxQuality);

		if(!scene_)
			return false;

		if(scene_)
		{
			// materials
			materials_.resize(scene_->mNumMaterials);
			textures_.resize(scene_->mNumMaterials);

			for(unsigned int i = 0; i < scene_->mNumMaterials; i++)
			{
				aiMaterial* pMaterial = scene_->mMaterials[i];

				aiColor4D diffuse;
				aiColor4D specular;
				aiColor4D ambient;
				aiColor4D emission;
				float shininess;

				if(AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
				{
					materials_[i].diffuse_[0] = diffuse[0];
					materials_[i].diffuse_[1] = diffuse[1];
					materials_[i].diffuse_[2] = diffuse[2];
					materials_[i].diffuse_[3] = diffuse[3];
				}

				if(AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient))
				{
					materials_[i].ambient_[0] = ambient[0];
					materials_[i].ambient_[1] = ambient[1];
					materials_[i].ambient_[2] = ambient[2];
					materials_[i].ambient_[3] = ambient[3];
				}

				if(AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &specular))
				{
					materials_[i].specular_[0] = specular[0];
					materials_[i].specular_[1] = specular[1];
					materials_[i].specular_[2] = specular[2];
					materials_[i].specular_[3] = specular[3];
				}

				if(AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_EMISSIVE, &emission))
				{
					materials_[i].emissive_[0] = emission[0];
					materials_[i].emissive_[1] = emission[1];
					materials_[i].emissive_[2] = emission[2];
					materials_[i].emissive_[3] = emission[3];
				}

				if(AI_SUCCESS == aiGetMaterialFloat(pMaterial, AI_MATKEY_SHININESS, &shininess))
				{
					materials_[i].shinines_ = shininess;
				}

				aiString texture_path;
				if(AI_SUCCESS == aiGetMaterialTexture(pMaterial, aiTextureType_DIFFUSE, 0, &texture_path))
				{
					string temp = texture_path.C_Str();
					string textureName = "../data/textures/";
					textureName += temp;
					D3DXCreateTextureFromFile(device, textureName.c_str(), &textures_[i].texture0_);
				}

				if(AI_SUCCESS == aiGetMaterialTexture(pMaterial, aiTextureType_SPECULAR, 0, &texture_path))
				{
					string temp = texture_path.C_Str();
					string textureName = "../data/textures/";
					textureName += temp;
					D3DXCreateTextureFromFile(device, textureName.c_str(), &textures_[i].texture1_);
				}

			}

			// mesh
			bLeaves_.resize(scene_->mNumMeshes);
			meshes_.resize(scene_->mNumMeshes);

//			for(int i = 0; i < scene_->mNumMeshes; i++)
			for(unsigned int i = 0; i < scene_->mNumMeshes; i++)
			{
				const aiMesh* pMesh = scene_->mMeshes[i];

				int num_vertex = pMesh->mNumVertices;
				int num_face = pMesh->mNumFaces;

				meshes_[i].numVertex_ = num_vertex;
				meshes_[i].numFace_ = num_face;

				// indices buffer
				device->CreateIndexBuffer(
					num_face * 3 * sizeof(WORD),
					D3DUSAGE_WRITEONLY,
					D3DFMT_INDEX16,
					D3DPOOL_DEFAULT,
					&meshes_[i].indexBuffer_,
					0);
				WORD *inx;
				meshes_[i].indexBuffer_->Lock(0, 0, (void**)&inx, 0);
				for(int j = 0; j < num_face; j ++)
				{
					aiFace *pFace = pMesh->mFaces + j;
					inx[3*j] = pFace->mIndices[0];
					inx[3*j + 1] = pFace->mIndices[1];
					inx[3*j + 2] = pFace->mIndices[2];
				}
				meshes_[i].indexBuffer_->Unlock();

				// vertex buffer
				device->CreateVertexBuffer(
					num_vertex * sizeof(OVertex),
					D3DUSAGE_WRITEONLY,
					FVFVER,
					D3DPOOL_DEFAULT,
					&meshes_[i].vertexBuffer_,
					0);

				OVertex * v = NULL;
				meshes_[i].vertexBuffer_->Lock(0, 0, (void**)&v, 0);

					// location
				for(int j = 0; j < num_vertex; j++)
				{
					v[j].x_ = pMesh->mVertices[j].x;
					v[j].y_ = pMesh->mVertices[j].y;
					v[j].z_ = pMesh->mVertices[j].z;
				}

					// normal
				if(pMesh->mNormals)
				{
					for(int j = 0; j < num_vertex; j++)
					{
						v[j].nx_ = pMesh->mNormals[j].x;
						v[j].ny_ = pMesh->mNormals[j].y;
						v[j].nz_ = pMesh->mNormals[j].z;
					}
				}

					// uv
				if(pMesh->mTextureCoords[0])
				{
					for(int j = 0; j < num_vertex; j++)
					{
						v[j].u_ = pMesh->mTextureCoords[0][j].x;
						v[j].v_ = pMesh->mTextureCoords[0][j].y;
					}
				}

				meshes_[i].vertexBuffer_->Unlock();

				// material index
				meshes_[i].materialIndex_ = pMesh->mMaterialIndex;

				if(textures_[meshes_[i].materialIndex_].texture1_ != NULL)
				{
					bLeaves_[i] = true;
					meshes_[i].leafIndex_ = leaf_count;
					leaf_count++;
				}
			}




			// leaves
			RandomDevice randomDevice;

			for(int i = 0; i < scene_->mNumMeshes; i++)
			{
				if(!bLeaves_[i])
					continue;

				const aiMesh* pMesh = scene_->mMeshes[i];
				int num_vertex = pMesh->mNumVertices;

				OLeaves lea;
				lea.meshIndex_ = i;

				lea.numVertex_ = num_vertex;
				lea.numIndex_ = pMesh->mNumFaces * 3;
				lea.numLeaves_ = pMesh->mNumFaces / 8;


				// leaf motion parameters
				lea.leafMotionParameters_.resize(lea.numLeaves_);
				for(int j = 0; j < lea.numLeaves_; j++)
				{
//					lea.leafMotionParameters_[j].pathS_ = randomDevice.GetFloatLine(0, 1.0f - PATH_MIN_LENGTH);
//					lea.leafMotionParameters_[j].pathE_ = randomDevice.GetFloatLine(lea.leafMotionParameters_[j].pathS_ + PATH_MIN_LENGTH, 1.0);

//					lea.leafMotionParameters_[j].scalarRoll_ = randomDevice.GetFloatLine(0.5f, 1.5f);
//					lea.leafMotionParameters_[j].scalar1_ = randomDevice.GetFloatLine(0.5f, 2.5f);
//					lea.leafMotionParameters_[j].scalar2_ = randomDevice.GetFloatLine(0.5f, 2.5f);

					lea.leafMotionParameters_[j].yV_ = randomDevice.GetFloatLine(0.5, 1);
					
					lea.leafMotionParameters_[j].delayTime_ = randomDevice.GetFloatLine(0, LEAF_FALLING_TIME);

					lea.leafMotionParameters_[j].rollW_ = randomDevice.GetFloatLine(-4, 4);

					lea.leafMotionParameters_[j].rotW_ = randomDevice.GetFloatLine(-4, 4);

					lea.leafMotionParameters_[j].xScaler_ = randomDevice.GetFloatLine(-1, 1);
					lea.leafMotionParameters_[j].xW_ = randomDevice.GetFloatLine(0.001f, 1.0f);
					lea.leafMotionParameters_[j].xPhi_ = randomDevice.GetFloatLine(0, 2 * FLOAT_PI);

					lea.leafMotionParameters_[j].zScaler_ = randomDevice.GetFloatLine(-1, 1);
					lea.leafMotionParameters_[j].zW_ = randomDevice.GetFloatLine(0.001f, 1.0f);
					lea.leafMotionParameters_[j].zPhi_ = randomDevice.GetFloatLine(0, 2 * FLOAT_PI);




//					lea.leaf_motion_parameters_[j].rollingW_ = 10;
				}

				// mesh
				lea.mesh_ = pMesh;

				// index_buffer_ 
				lea.indexBuffer_ = meshes_[i].indexBuffer_;

				// vertex_buffer_
				lea.originalVertices_.resize(num_vertex);


				device->CreateVertexBuffer(
					num_vertex * sizeof(OLeafVertex),
					D3DUSAGE_WRITEONLY,
					FVFLEAF,
					D3DPOOL_DEFAULT,
					&lea.vertexBuffer_,
					NULL);

				OLeafVertex* v = NULL;
				lea.vertexBuffer_->Lock(0, 0, (void**)&v, 0);

				for(int i = 0; i < pMesh->mNumFaces / 8; i++)
				{
					OLeafMotion * pLeafMotion = &lea.leafMotionParameters_[i];


					D3DXVECTOR3 centerLoc(0, 0, 0);
					D3DXVECTOR3 centerNor(0, 0, 0);

			


					for(int j = 0; j < 8; j++)
					{
						aiFace *pFace = pMesh->mFaces + 8 * i + j;
						int fv[3];
						fv[0] = pFace->mIndices[0];
						fv[1] = pFace->mIndices[1];
						fv[2] = pFace->mIndices[2];

						for(int k = 0; k < 3; k++)
						{
							centerLoc[0] += pMesh->mVertices[fv[k]][0];
							centerLoc[1] += pMesh->mVertices[fv[k]][1];
							centerLoc[2] += pMesh->mVertices[fv[k]][2];

							centerNor[0] += pMesh->mNormals[fv[k]][0];
							centerNor[1] += pMesh->mNormals[fv[k]][1];
							centerNor[2] += pMesh->mNormals[fv[k]][2];
						}
					}

					centerLoc /= 24;
					centerNor /= 24;

					for(int j = 0; j < 8; j++)
					{
						aiFace *pFace = pMesh->mFaces + 8 * i + j;
						int fv[3];
						fv[0] = pFace->mIndices[0];
						fv[1] = pFace->mIndices[1];
						fv[2] = pFace->mIndices[2];

						for(int k = 0; k < 3; k++)
						{
							int t = fv[k];
							v[t].cx_ = centerLoc[0];
							v[t].cy_ = centerLoc[1];
							v[t].cz_ = centerLoc[2];

							
							v[t].nx_  = centerNor[0];
							v[t].ny_  = centerNor[1];
							v[t].nz_  = centerNor[2];

							v[t].ovx_ = pMesh->mVertices[t][0] - centerLoc[0];
							v[t].ovy_ = pMesh->mVertices[t][1] - centerLoc[1];
							v[t].ovz_ = pMesh->mVertices[t][2] - centerLoc[2];
							 
//							v[t].alpha_ = 0;
//							v[t].scalar_ = 1;

							v[t].u_ = pMesh->mTextureCoords[0][t].x;
							v[t].v_ = pMesh->mTextureCoords[0][t].y;

//							v[t].beta_ = 0;

							v[t].rollW_ = pLeafMotion->rollW_;
							v[t].rotW_ = pLeafMotion->rotW_;

							v[t].delayTime_ = pLeafMotion->delayTime_;
							v[t].yV_ = pLeafMotion->yV_;

							v[t].xPhi_ = pLeafMotion->xPhi_;
							v[t].xW_ = pLeafMotion->xW_;
							v[t].xScalar_ = pLeafMotion->xScaler_;

							v[t].zPhi_ = pLeafMotion->zPhi_;
							v[t].zW_ = pLeafMotion->zW_;
							v[t].zScalar_ = pLeafMotion->zScaler_;


							lea.originalVertices_[t] = v[t];
						}				
					}
				}
				lea.vertexBuffer_->Unlock();

				leaves_.push_back(lea);
			}

			return true;
		}
	}

	bool OScene::ImportTerrain(IDirect3DDevice9* device, const char* heightfile, const char* texturefile)
	{
		if(!terrain_)
			free(terrain_);

		terrain_ = new OTerrain();


		terrain_->LoadHeightData(heightfile);
		terrain_->LoadTexture(device, texturefile);
		terrain_->CreateTerrainMesh(device);
		


		return true;
	}

	bool OScene::InitialSky(const char* filename, IDirect3DDevice9 *device)
	{
		sky_.textures_.resize(3);
		for(int i = 0; i < 3; i++)
		{
			char textureName[200];
			sprintf(textureName, "%s/%d.dds", filename, i);
			D3DXCreateCubeTextureFromFile(device, textureName, &sky_.textures_[i]);
		}


		device->CreateVertexBuffer(
			6 * sizeof(OVertex),
			D3DUSAGE_WRITEONLY,
			FVFVER,
			D3DPOOL_DEFAULT,
			&sky_.vertexBuffer_,
			0);

		OVertex *v = NULL;
		sky_.vertexBuffer_->Lock(0, 0, (void**)&v, 0);
		v[0] = OVertex(-1, 1, 0.9999);
		v[1] = OVertex(1, 1, 0.9999);
		v[2] = OVertex(1, -1, 0.9999);

		v[3] = OVertex(-1, 1, 0.9999);
		v[4] = OVertex(1, -1, 0.9999);
		v[5] = OVertex(-1, -1, 0.9999);
		sky_.vertexBuffer_->Unlock();


		return true;
	}
}
