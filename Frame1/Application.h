#ifndef APPLICATION
#define APPLICATION

#include <Windows.h>
#include <string>
#include "OEngine.h"
using namespace ohday;
using namespace std;

class CApplication
{
public:
	CApplication(void);
	~CApplication(void);

	bool Initialize(HINSTANCE hInstance, LPSTR cmdLine, int cmdShow);
	bool LoadWindow(const string &windowTitle, int posX, int posY, int sizeX, int sizeY);
	bool LoadScene(const char* st);

	bool LoadTerrain(const char* heightfile, const char* texturefile);
	bool LoadSky(const char* filename);

	int Run();
	bool Close();

private:
	HWND windleHandle_;
	HINSTANCE instanceHandle_;

	int cmdShow_;
	LPSTR commandLine_;
	
	OEngine * graphEngine_;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif