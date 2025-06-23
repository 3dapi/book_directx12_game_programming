#pragma once

#include "StepTimer.h"
#include "D3DDevice.h"
#include "MainApp.h"

// Renders Direct3D content on the screen.
class D3DApp
{
public:
	D3DApp();
	void CreateRenderers(const std::shared_ptr<DX::D3DDevice>& deviceResources);
	void Update();
	bool Render();

	void OnWindowSizeChanged();
	void OnDeviceRemoved();

private:
	// TODO: Replace with your own content renderers.
	std::unique_ptr<MainApp> m_pmain;

	// Rendering loop timer.
	DX::StepTimer m_timer;
};
