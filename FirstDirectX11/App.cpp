#include "App.h"



App::App()
	:
	wnd(800, 600, "DirectX Application")
{}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			//if return optional has a value, it means that the message is WM_QUIT
			return *ecode;
		}
		DoFrame();
	}
	
}

void App::DoFrame()
{
	wnd.getGraphicsPointer().EndFrame();
}

