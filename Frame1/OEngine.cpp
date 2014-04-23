#include "OEngine.h"

namespace ohday
{
	OEngine::OEngine(void)
	{
		scene_ = NULL;
		d3dObject_ = NULL;
		d3dDevice_ = NULL;

		meshEffect_ = NULL;

		light_.ambient_ = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		light_.color_ = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		light_.direction_ = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.0f);


		cameraPosition_ = D3DXVECTOR4(20.0f, 0.0f, 0.0f, 1.0f);

		target_[0] = target_[1] = target_[2] = 0.0f;

		dayTime_ = 0.5f;
		yearTime_ = 0.5f;

		isLeavesFalling_ = false;
	}


	OEngine::~OEngine(void)
	{
		if(!scene_)
			free(scene_);
	}


	bool OEngine::InitialWithWindowHandle(HWND hWnd)
	{
		if(!InitDirect3DObject())
			return false;

		D3DPRESENT_PARAMETERS presentationParameters;
		ZeroMemory(&presentationParameters, sizeof(presentationParameters));
		presentationParameters.Windowed = TRUE;
		presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
		presentationParameters.BackBufferFormat = D3DFMT_UNKNOWN;

		// depth buffer
		presentationParameters.EnableAutoDepthStencil = TRUE;
		presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;

		HRESULT hResult = d3dObject_->CreateDevice(
			D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&presentationParameters,&d3dDevice_);

		//HRESULT hResult = d3dObject_->CreateDevice(
		//	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		//	&presentationParameters,&d3dDevice_);



		scene_ = new OScene();

		InitialShaders();

		if(FAILED(hResult))
			return false;
		
		d3dDevice_->SetRenderState(D3DRS_ZENABLE, TRUE);
		d3dDevice_->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

		return true;
	}


	bool OEngine::InitDirect3DObject()
	{
		d3dObject_ = Direct3DCreate9(D3D_SDK_VERSION);
		return d3dObject_ != NULL;
	}

	bool OEngine::InitialShaders()
	{

		ID3DXBuffer *errors = NULL;
		HRESULT hr = D3DXCreateEffectFromFile(
			d3dDevice_,
			"shaders/mesh.fx",
			0, 0,
			D3DXSHADER_DEBUG,
			0,
			&meshEffect_,
			&errors);

		if(errors)
		{
			MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
			return false;
		}



		D3DXMatrixPerspectiveFovLH(
			&projctionMatrix_, D3DX_PI * 0.25f,
			(float)WIN_WIDTH / (float)WIN_HEIGHT, 1.0f, 1000.0f);

		D3DXHANDLE hProjectionMatrix = meshEffect_->GetParameterByName(0, "projMatrix");
		hr = meshEffect_->SetMatrix(hProjectionMatrix, &projctionMatrix_);


		return true;
	}

	void OEngine::RenderScene()
	{
		UpdateView();
		UpdateTime();

		UpdateLeaves();


		static D3DXHANDLE hLeafLerper = meshEffect_->GetParameterByName(0, "leafLerper");
		meshEffect_->SetVector(hLeafLerper, &leafLerper_);

		UpdateSkyLerper();
		static D3DXHANDLE hSkyLerper = meshEffect_->GetParameterByName(0, "skyLerper");
		meshEffect_->SetVector(hSkyLerper, &skyLerper_);


		d3dDevice_->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(101, 156, 239), 1.0f, 0);
		
		d3dDevice_->BeginScene();
		meshEffect_->Begin(NULL, NULL);
		RenderTerrain();

		RenderSky();

		RenderMesh();

		meshEffect_->End();
		d3dDevice_->EndScene();
		d3dDevice_->Present(0, 0, 0, 0);


		timeLastRender_ = clock();
	}

	void OEngine::RenderTerrain()
	{
		OTerrain* pTerrain = scene_->terrain_;

		static D3DXHANDLE hCamera = meshEffect_->GetParameterByName(0, "camera_position");
		HRESULT hr = meshEffect_->SetVector(hCamera, &cameraPosition_);

		static D3DXHANDLE hLight = meshEffect_->GetParameterByName(0, "light");
		hr = meshEffect_->SetRawValue(hLight, &light_, 0, sizeof(OLight));

		static D3DXHANDLE hWorldMatrix = meshEffect_->GetParameterByName(0, "worldMatrix");
		D3DXMATRIX worldMatrix;
		D3DXMatrixIdentity(&worldMatrix);
		meshEffect_->SetMatrix(hWorldMatrix, &worldMatrix);

		static D3DXHANDLE hViewMatrix = meshEffect_->GetParameterByName(0, "viewMatrix");
		meshEffect_->SetMatrix(hViewMatrix, &viewMatrix_);

		static D3DXHANDLE hDifTexture = meshEffect_->GetParameterByName(0, "t0");
		hr = meshEffect_->SetTexture(hDifTexture, pTerrain->texture_);

		hr = d3dDevice_->SetFVF(FVFVER);
		hr = d3dDevice_->SetStreamSource(0, pTerrain->vertex_buffer_, 0, sizeof(OVertex));				
		hr = d3dDevice_->SetIndices(pTerrain->index_buffer_);

		hr = meshEffect_->BeginPass(1);
		d3dDevice_->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0,
			0,
			pTerrain->num_vertex_,
			0,
			pTerrain->num_index_);

		meshEffect_->EndPass();
		
	}

	void OEngine::RenderSky()
	{
		HRESULT hr; 
		
		static float alpha = 0.0f;
		static float step = 0.001f;
		static D3DXVECTOR3 axis(0.0f, 1.0f, 0.0f);
		static D3DXMATRIX rot;

		D3DXMatrixRotationAxis(&rot, &axis, alpha);
		alpha += step;
		static D3DXHANDLE hSkyRot = meshEffect_->GetParameterByName(0, "skyRotation");
		hr = meshEffect_->SetMatrix(hSkyRot, &rot);



		OSky *pSky = &scene_->sky_;

		static D3DXHANDLE hInvMVP = meshEffect_->GetParameterByName(0, "invMVP");
		D3DXMATRIX temp = viewMatrix_ * projctionMatrix_, invMVP;
		D3DXMatrixInverse(&invMVP, NULL, &temp);
		hr = meshEffect_->SetMatrix(hInvMVP, &invMVP);





		hr = d3dDevice_->SetFVF(FVFVER);
		hr = d3dDevice_->SetStreamSource(0, pSky->vertex_buffer_, 0, sizeof(OVertex));				

		hr = meshEffect_->BeginPass(2);

		d3dDevice_->DrawPrimitive(
			D3DPT_TRIANGLELIST,
			0,
			2);
		meshEffect_->EndPass();

	}

	void OEngine::RenderMesh()
	{
		if(!meshEffect_)
			return;


		aiNode* root = scene_->scene_->mRootNode;

		D3DXMATRIX releMatrix;
		D3DXMatrixIdentity(&releMatrix);

		
		RecursiveRender(root, releMatrix);

	}

	void OEngine::RecursiveRender(aiNode *node, D3DXMATRIX releMat)
	{
		static D3DXHANDLE hCamera = meshEffect_->GetParameterByName(0, "camera_position");
		HRESULT hr = meshEffect_->SetVector(hCamera, &cameraPosition_);

		static D3DXHANDLE hLight = meshEffect_->GetParameterByName(0, "light");
		hr = meshEffect_->SetRawValue(hLight, &light_, 0, sizeof(OLight));


		static D3DXHANDLE hWorldMatrix = meshEffect_->GetParameterByName(0, "worldMatrix");
		static D3DXHANDLE hViewMatrix = meshEffect_->GetParameterByName(0, "viewMatrix");
		hr = meshEffect_->SetMatrix(hViewMatrix, &viewMatrix_);

		static D3DXHANDLE hMaterial = meshEffect_->GetParameterByName(0, "mtrl");
		static D3DXHANDLE hTexture0 = meshEffect_->GetParameterByName(0, "t0");
		static D3DXHANDLE hTexture1 = meshEffect_->GetParameterByName(0, "t1");

		D3DXMATRIX transformMatrix;
		D3DXMatrixIdentity(&transformMatrix);
		for(int ir = 0; ir < 4; ir++)
			for(int ic = 0; ic < 4; ic++)
				transformMatrix(ir, ic) = node->mTransformation[ic][ir];

		D3DXMATRIX worldMatrix = transformMatrix * releMat;
		hr = meshEffect_->SetMatrix(hWorldMatrix, &worldMatrix);

		for(int i = 0; i < node->mNumMeshes; i++)
		{
			// �����������ԣ�������һ���ڵ�ֻ��Ӧһ��mesh�����ֶ��mesh������£�ֻ��ʾ���һ��
			i = node->mNumMeshes - 1;

			int meshId = node->mMeshes[i];

//			D3DXMATRIX mvpMatrix = modelMatrix * viewMatrix_ * projctionMatrix_;
			//			D3DXMATRIX mvpMatrix = viewMatrix_ * projctionMatrix_;
						
			OMesh *pMesh = &scene_->meshes_[meshId];


			int mid = pMesh->material_index_;

			if(scene_->bLeaves_[meshId])
			{
				// render leaves
				if(mid != -1)
				{
					hr = meshEffect_->SetRawValue(hMaterial, &scene_->materials_[mid], 0, sizeof(OMaterial));
					hr = meshEffect_->SetTexture(hTexture0, scene_->textures_[mid].texture0_);
					hr = meshEffect_->SetTexture(hTexture1, scene_->textures_[mid].texture1_);
				}

				int leaf_index = pMesh->leaf_index_;
				OLeaves * pLeaves = &scene_->leaves_[leaf_index];
				hr = d3dDevice_->SetFVF(FVFLEAF);
				hr = d3dDevice_->SetStreamSource(0, pLeaves->vertex_buffer_, 0, sizeof(OLeafVertex));
				hr = d3dDevice_->SetIndices(pLeaves->index_buffer_);


				hr = meshEffect_->BeginPass(3);
				d3dDevice_->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					0,
					0,
					pLeaves->num_vertex_,
					0,
					pLeaves->num_index_);

				meshEffect_->EndPass();
			}
			else
			{
				// render mesh
				if(mid != -1)
				{
					hr = meshEffect_->SetRawValue(hMaterial, &scene_->materials_[mid], 0, sizeof(OMaterial));
					hr = meshEffect_->SetTexture(hTexture0, scene_->textures_[mid].texture0_);
				}
				hr = d3dDevice_->SetFVF(FVFVER);
				hr = d3dDevice_->SetStreamSource(0, pMesh->vertex_buffer_, 0, sizeof(OVertex));				
				hr = d3dDevice_->SetIndices(pMesh->index_buffer_);

				hr = meshEffect_->BeginPass(0);
				d3dDevice_->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST,
					0,
					0,
					pMesh->num_vertex_,
					0,
					pMesh->num_index_);

				meshEffect_->EndPass();
			}

		}

		for(int i = 0; i < node->mNumChildren; i++)
		{
			aiNode* child = node->mChildren[i];
			RecursiveRender(child, worldMatrix);
		}


	}


	void OEngine::UpdateView()
	{
		D3DXVECTOR3 test, tt;
		static float r_step = 0.1f;
		static float w_step = 0.1f;
		static float u_step = 0.1f;
		static float h_angle_step = 100.0f;
		static float v_angle_step = 100.0f;

		static D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		static POINT pStart, pEnd;
		static bool bPushed = false;


		static D3DXVECTOR3 vLook;
		static D3DXVECTOR3 lLook;


		D3DXVECTOR3 tempCamera(cameraPosition_.x, cameraPosition_.y, cameraPosition_.z);

		vLook = target_ - tempCamera;

		D3DXVec3Normalize(&vLook, &vLook);
		D3DXVec3Cross(&lLook, &vLook, &up);
		
		if(GetAsyncKeyState('W') & 0x8000f)
		{
			tempCamera += r_step * vLook;
		}

		if(GetAsyncKeyState('S') & 0x8000f)
		{
			tempCamera -= r_step * vLook;
		}

		if(GetAsyncKeyState('A') & 0x8000f)
		{
			tempCamera += w_step * lLook;
		}

		if(GetAsyncKeyState('D') & 0x8000f)
		{
			tempCamera -= w_step * lLook;
		}

		if(GetAsyncKeyState('Q') & 0x8000f)
		{
			tempCamera += u_step * up;
		}

		if(GetAsyncKeyState('E') & 0x8000f)
		{
			tempCamera -= u_step * up;
		}


		SHORT a = GetAsyncKeyState(VK_LBUTTON);
		if(a < 0)
		{
			if(!bPushed)
			{
				bPushed = true;
				pStart = GetMousePostion();
			}
			else
			{
				pEnd = GetMousePostion();

				if(pStart.x != pEnd.x || pStart.y != pEnd.y)
				{
					float h_angle = (pEnd.x - pStart.x) / h_angle_step;
					float v_angle = (pStart.y - pEnd.y) / v_angle_step;

					D3DXMATRIX rotv, roth;
					D3DXMatrixRotationAxis(&rotv, &lLook, v_angle);
					D3DXMatrixRotationAxis(&roth, &up, h_angle);

					D3DXMATRIX rot = rotv * roth;

					D3DXVECTOR4 vLook4(vLook, 0.0f);
					D3DXVECTOR4 lLook4(lLook, 0.0f);


					D3DXVec4Transform(&vLook4, &vLook4, &rot);
					D3DXVec4Transform(&lLook4, &lLook4, &rot);

					vLook[0] = vLook4[0];	vLook[1] = vLook4[1];	vLook[2] = vLook4[2];
					lLook[0] = lLook4[0];	lLook[1] = lLook4[1];	lLook[2] = lLook4[2];

					pStart = pEnd;
				}			
			}
		}
		else
			bPushed = false;


		target_ = tempCamera + vLook;

		D3DXMatrixLookAtLH(&viewMatrix_, &tempCamera, &target_, &up);

		cameraPosition_[0] = tempCamera[0];
		cameraPosition_[1] = tempCamera[1];
		cameraPosition_[2] = tempCamera[2];
	}

	void OEngine::UpdateTime()
	{
		static float dayTimeStep = 0.003f;
		static float yearTimeStep = 0.003f;
		if(GetAsyncKeyState('T') & 0x8000f)
		{
			dayTime_ += dayTimeStep;
		}

		if(dayTime_ > 1.0f)
			dayTime_ = 0.0f;

		if(GetAsyncKeyState('Y') & 0x8000f)
		{
			yearTime_ += yearTimeStep;
		}

		if(yearTime_ > 1.0f)
			yearTime_ = 0.0f;

	}

	POINT OEngine::GetMousePostion()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hWnd, &p);
		return p;
	}

	bool OEngine::LoadScene(const char * st)
	{
//		return scene_->ImportFbxFile(st, d3dDevice_);

		

		return scene_->ImportScene(st, d3dDevice_);
	}

	bool OEngine::LoadSky(const char * st)
	{
		bool flag = scene_->InitialSky(st, d3dDevice_);

		if(!flag)
			return false;

		OSky* pSky = &scene_->sky_;
		D3DXHANDLE hTexture0 = meshEffect_->GetParameterByName(0, "sky0");
		meshEffect_->SetTexture(hTexture0, pSky->textures_[0]);

		D3DXHANDLE hTexture1 = meshEffect_->GetParameterByName(0, "sky1");
		meshEffect_->SetTexture(hTexture1, pSky->textures_[1]);

		D3DXHANDLE hTexture2 = meshEffect_->GetParameterByName(0, "sky2");
		meshEffect_->SetTexture(hTexture2, pSky->textures_[2]);




		return true;
	}

	bool OEngine::LoadTerrain(const char *heightfile, const char* texturefile)
	{
		return scene_->ImportTerrain(d3dDevice_, heightfile, texturefile);	
	}

	void OEngine::UpdateSkyLerper()
	{
		if(dayTime_ < 0.2)
		{
			skyLerper_[0] = 1;
			skyLerper_[1] = skyLerper_[2] = 0;
			return;
		}

		if(dayTime_ < 0.4)
		{
			skyLerper_[0] = 1 - (dayTime_ - 0.2) / 0.2;
			skyLerper_[1] = 1 - skyLerper_[0];
			skyLerper_[2] = 0;
			return;
		}

		if(dayTime_ < 0.8)
		{
			skyLerper_[1] = 1;
			skyLerper_[0] = skyLerper_[2] = 0;
			return;
		}


		skyLerper_[1] = 1 - (dayTime_ - 0.8) / 0.2;
		skyLerper_[2] = 1 - skyLerper_[1];
		skyLerper_[0] = 0;
	}

	void OEngine::UpdateLeaves()
	{
		static bool toggle = true;
		if(GetAsyncKeyState('F') & 0x8000f)
		{
			ToggleLeavesFalling(toggle);
			toggle = !toggle;
		}

		if(isLeavesFalling_)
		{
			UpdateFallingLeaves();
		}

		UpdateLeafParameters();

		UpdateLeafBuffer();



	}

	void OEngine::UpdateLeafBuffer()
	{
		for(int i = 0; i < scene_->leaves_.size(); i++)
		{
			OLeaves * pLeaves = &scene_->leaves_[i];
			OLeafVertex * pVertex = NULL;

			pLeaves->vertex_buffer_->Lock(0, pLeaves->num_vertex_ * sizeof(OLeafVertex), (void**)&pVertex, D3DLOCK_NOOVERWRITE);

			for(int j = 0; j < pLeaves->num_vertex_; j++)
			{
				pVertex[j].scalar_ = leafScalar_;

			}

			pLeaves->vertex_buffer_->Unlock();
			
		}
	}

	void OEngine::UpdateLeafParameters()
	{
		UpdateLeafLerper();


		// first test leaf scalar
		if(yearTime_ < 0.25)
		{
			leafScalar_ = yearTime_ / 0.25;
			return;
		}

		if(yearTime_ < 0.75)
		{
			leafScalar_ = 1.0;
			return;
		}

		if(yearTime_ < 1.0)
		{
			leafScalar_ = (1.0 - yearTime_) / 0.25;
			return;
		}

		// falling function



	}

	void OEngine::UpdateLeafLerper()
	{
		if(yearTime_ < 0.25)
		{
			leafLerper_[0] = 1;
			leafLerper_[1] = 0;
			return;
		}

		if(yearTime_ < 0.5)
		{
			leafLerper_[0] = 1 - (yearTime_ - 0.25) / 0.25;
			leafLerper_[1] = 1 - leafLerper_[0];
			return;
		}

		if(yearTime_ < 0.75)
		{
			leafLerper_[0] = 0;
			leafLerper_[1] = 1;
			return;
		}

		leafLerper_[1] = 1 - (yearTime_ - 0.75) / 0.25;
		leafLerper_[0] = 1 - leafLerper_[1];
	}

	void OEngine::ToggleLeavesFalling(bool t)
	{
		if(t)
		{
			isLeavesFalling_ = true;
			timeFallingStart_ = clock();
		}
		else
			isLeavesFalling_ = false;
		
	}

	void OEngine::UpdateFallingLeaves()
	{
		time_t timeCurrent = clock();
		float seconds = float(timeCurrent - timeFallingStart_) / CLOCKS_PER_SEC;

		HRESULT hr = S_OK;

		for(int i = 0; i < scene_->leaves_.size(); i++)
		{
			OLeaves* pLeaves = &scene_->leaves_[i];
			
			OLeafVertex * pVertex = NULL;
			
			hr = pLeaves->vertex_buffer_->Lock(0, pLeaves->num_vertex_ * sizeof(OLeafVertex), (void**)&pVertex, D3DLOCK_NOOVERWRITE);

			for(int j = 0; j < pLeaves->mesh_->mNumFaces; j++)
			{
				OLeafMotion * pLeafMotion = &pLeaves->leaf_motion_parameters_[j / 8];

				aiFace* pFace = pLeaves->mesh_->mFaces + j;

				for(int k = 0; k < 3; k++)
				{
					int t = pFace->mIndices[k];
					
					if(pVertex[t].cz_ > 0.001f && seconds > pLeafMotion->delayTime_)
					{
						float fallTime = seconds - pLeafMotion->delayTime_;
						// ��ֱ���򣨶���Ҷ��˵��z���򣩣���������, 
						pVertex[t].cz_ = pLeaves->original_vertices_[t].cz_ - pLeafMotion->falling_v_ * fallTime;

						// xyƽ�棬��ѭ�켣

						float fTimeLerper = pVertex[t].cz_ / pLeaves->original_vertices_[t].cz_;
						if(fTimeLerper > 1)
							fTimeLerper = 1;
						if(fTimeLerper < 0)
							fTimeLerper = 0;

						float pathLerper = pLeafMotion->path_s_ * fTimeLerper + pLeafMotion->path_e_ * (1 - fTimeLerper);
						int pathIndex = int(scene_->leaf_path_[0].size() * pathLerper);
						int pathStart = int(scene_->leaf_path_[0].size() * pLeafMotion->path_s_);

						pVertex[t].cx_ = pLeaves->original_vertices_[t].cx_ + pLeafMotion->scalar1_ * (scene_->leaf_path_[0][pathIndex] - scene_->leaf_path_[0][pathStart]);
						pVertex[t].cy_ = pLeaves->original_vertices_[t].cy_ + pLeafMotion->scalar2_ * (scene_->leaf_path_[1][pathIndex] - scene_->leaf_path_[1][pathStart]);

						// �����˶�
						pVertex[t].beta_ = pLeafMotion->rollingW_ * fallTime;
					}


				}
			}
			hr = pLeaves->vertex_buffer_->Unlock();
			


// 			for(int j = 0; j < pLeaves->num_vertex_; j++)
// 			{
// 				OLeafMotion *pLeafMotion = &pLeaves->leaf_motion_parameters_[j / 9];
// 
// 
// 				pVertex[j].cy_ = pLeaves->original_vertices_[j].cy_ - pLeafMotion->falling_v_ * seconds;
// 
// 				// need to consider about terrain height later
// 				float lerp = seconds / c_leaf_falling_time;
// 				if(lerp < 0)
// 					lerp = 0;
// 				if(lerp > 1)
// 					lerp = 1;
// 
// 				float pathLerp = pLeafMotion->path_s_ * (1 - lerp) + pLeafMotion->path_e_ * lerp;
// 
// 				int pathK = scene_->leaf_path_[0].size() * pathLerp;
// 
// 				pVertex[j].cx_ = pLeaves->original_vertices_[j].cx_ + scene_->leaf_path_[0][pathK];
// 				pVertex[j].cz_ = pLeaves->original_vertices_[j].cz_ + scene_->leaf_path_[1][pathK];
// 				
// 			}			

//			hr = pLeaves->vertex_buffer_->Unlock();

		}
	}

}
