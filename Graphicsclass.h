

#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

//#include <directx11Main.h>
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true; 
const float SCREEN_DEPTH = 1000.0f; 
const float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public :
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int);
	void Shutdown();
	bool Frame();
private:
	bool Render();
//private:
//	directx11::directx11Main m_d3D;
}; 
#endif