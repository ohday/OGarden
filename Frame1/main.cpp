#include "Application.h"
#include "SUtilities.h"
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	CApplication window;
	if(window.Initialize(hInstance, lpCmdLine, nShowCmd))
	{
		if(window.LoadWindow("My Swanky Window", 350, 300, WIN_WIDTH, WIN_HEIGHT))
		{
//			window.LoadScene("tree0.fbx");
			window.LoadScene("../data/scene/garden2.3ds");
			window.LoadTerrain(NULL, "../data/textures/grass.jpg");
			window.LoadSky("../data/skybox");
			return window.Run();
		}
	}
}