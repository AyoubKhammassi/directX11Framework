#include "App.h"



App::App()
	:
	timer(),
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
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.getGraphicsPointer().ClearBuffer(c, c, 1.0f);
	wnd.getGraphicsPointer().EndFrame();
}

