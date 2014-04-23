#include "Application.h"


CApplication::CApplication(void)
{
	graphEngine_ = NULL;
}


CApplication::~CApplication(void)
{
	if(graphEngine_ != NULL)
		delete graphEngine_;
}

bool CApplication::Initialize(HINSTANCE hInstance, LPSTR cmdLine, int cmdShow)
{

	instanceHandle_ = hInstance;
	commandLine_ = cmdLine;
	cmdShow_ = cmdShow;

	return true;
}

bool CApplication::LoadWindow(const string &windowTitle, int posX, int posY, int sizeX, int sizeY)
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	windowClass.lpszClassName = "WindowClassName";

	windowClass.lpfnWndProc = (WNDPROC)CApplication::WndProc;
	windowClass.hInstance = instanceHandle_;

	RegisterClassEx(&windowClass);

	windleHandle_ = CreateWindow(
		"WindowClassName", windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
		posX, posY, sizeX, sizeY, NULL, NULL, instanceHandle_, NULL);

	ShowWindow(windleHandle_, cmdShow_);
	UpdateWindow(windleHandle_);


	graphEngine_ = new OEngine();
	if(!graphEngine_->InitialWithWindowHandle(windleHandle_))
	{
		delete graphEngine_;
		return false;
	}

	graphEngine_->hWnd = windleHandle_;

	return true;
}

int CApplication::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while(msg.message != WM_QUIT)
	{
		while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}	

		graphEngine_->RenderScene();
		Sleep(0);
	}
	return 0;
}

bool CApplication::Close()
{
	return true;
}

LRESULT CApplication::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

}

bool CApplication::LoadScene(const char* st)
{
	return graphEngine_->LoadScene(st);
}

bool CApplication::LoadTerrain(const char* heightfile, const char* texturefile)
{
	return graphEngine_->LoadTerrain(heightfile, texturefile);
}

bool CApplication::LoadSky(const char* filename)
{
	return graphEngine_->LoadSky(filename);
}