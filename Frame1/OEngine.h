#ifndef OHDAY_ENGINE
#define OHDAY_ENGINE

#include "OScene.h"
#include "SUtilities.h"


namespace ohday
{
	class OEngine
	{
	public:
		OEngine(void);
		~OEngine(void);

		OScene *scene_;

		HWND hWnd;

		void RenderScene();


		bool InitialWithWindowHandle(HWND hWnd);

		bool LoadScene(const char * st);

		bool LoadSky(const char * st);
		
		bool LoadTerrain(const char *heightfile, const char* texturefile);

	private:
		bool InitDirect3DObject();
		bool InitialShaders();

		void UpdateView();
		void UpdateTime();


		void RenderTerrain();

		void RenderMesh();

		void RecursiveRender(aiNode *node, D3DXMATRIX releMat);

		void RenderSky();

		POINT GetMousePostion();

		void UpdateSkyLerper();

		void UpdateLeaves();

		void UpdateLeafParameters();

		void UpdateLeafTextureLerper();
		
//		void UpdateFallingLeaves();
//		void UpdateLeafScaler();

		void ToggleLeavesFalling(bool t);

		void ResetLeaves();
		
		OWind wind_;

		float dayTime_;
		float yearTime_;

		D3DXVECTOR4 skyLerper_;
		D3DXVECTOR4 leafLerper_;

		float leafScalar_;

		ID3DXEffect* meshEffect_;


		IDirect3D9* d3dObject_;
		IDirect3DDevice9* d3dDevice_;

		D3DXMATRIX projctionMatrix_;
		D3DXMATRIX viewMatrix_;

		D3DXVECTOR4 cameraPosition_;

		D3DXVECTOR3 target_;

		OLight light_;

		bool isLeavesFalling_;
		time_t timeFallingStart_;
		time_t timeLastRender_;
	};
}


#endif