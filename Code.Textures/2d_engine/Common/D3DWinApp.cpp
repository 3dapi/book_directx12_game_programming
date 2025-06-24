//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <WindowsX.h>
#include "D3DWinApp.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return IG2AppFrameWin::instance()->MsgProc(hwnd, msg, wParam, lParam);
}

D3DWinApp::~D3DWinApp()
{
}

HINSTANCE D3DWinApp::AppInst()const
{
	return mhAppInst;
}

HWND D3DWinApp::MainWnd()const
{
	return mhMainWnd;
}

float D3DWinApp::AspectRatio()const
{
	return static_cast<float>(m_screenSize.cx) / m_screenSize.cx;
}

void D3DWinApp::Set4xMsaaState(bool value)
{
	int hr = IG2Graphics::instance()->command(EG2GRAPHICS_D3D::CMD_MSAASTATE4X, value);
	if(SUCCEEDED(hr))
	{
		Resize(false);
	}
}

int D3DWinApp::Run()
{
	MSG msg = {0};
 
	mTimer.Reset();

	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
        {
			Render3D();
        }
    }

	//IG2Graphics::instance()->FlushCommandQueue();

	return (int)msg.wParam;
}

int D3DWinApp::Render3D()
{
	int hr = S_OK;
	mTimer.Tick();
	if(mAppPaused)
	{
		Sleep(100);
		return S_OK;
	}
	CalculateFrameStats();
	Update(mTimer);
	Render();
	return S_OK;
}

int D3DWinApp::init(const std::any& initialValue)
{
	if(!InitMainWindow())
		return false;

	auto d3d = IG2Graphics::instance();
	int hr = d3d->init(std::make_tuple(mhMainWnd, m_screenSize, m_msaa4State));
	if(FAILED(hr))
		return false;

    Resize();
	return true;
}
 
int D3DWinApp::Resize(bool update)
{
	int hr = S_OK;
	if(update)
		hr = IG2Graphics::instance()->command(CMD_SCREEN_RESIZE, m_screenSize);
	return hr;
}
 
LRESULT D3DWinApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;
		}

		// WM_SIZE is sent when the user resizes the window.  
		case WM_SIZE:
		{
			// Save the new client area dimensions.
			long cx = LOWORD(lParam);
			long cy = HIWORD(lParam);
			if ((m_screenSize.cx == cx) && (m_screenSize.cy == cy))
				return 0;

			m_screenSize = { cx, cy };
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				Resize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					Resize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					Resize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or m_d3dSwapChain->SetFullscreenState.
				{
					Resize();
				}
			}
			return 0;
		}
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
		{
			mAppPaused = true;
			mResizing = true;
			mTimer.Stop();
			return 0;
		}

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
		{
			mAppPaused = false;
			mResizing = false;
			mTimer.Start();
			Resize();
			return 0;
		}

		// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
		{
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);
		}
		// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			OnMouseDown(wParam, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			OnMouseUp(wParam, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			OnMouseMove(wParam, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
			return 0;
		}
		case WM_KEYUP:
		{
			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			else if ((int)wParam == VK_F2)
			{
				m_msaa4State = !m_msaa4State;
				IG2Graphics::instance()->command(CMD_MSAASTATE4X, m_msaa4State);
			}

			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DWinApp::InitMainWindow()
{
	WNDCLASS wc{};
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.hInstance     = mhAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszClassName = L"MainWnd";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_screenSize.cx, m_screenSize.cy };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(), 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0); 
	if( !mhMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

void D3DWinApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
