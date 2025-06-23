#include "pch.h"
#include "D3DApp.h"
#include "DirectXHelper.h"

D3DApp::D3DApp()
{
}

// Creates and initializes the renderers.
void D3DApp::CreateRenderers(const std::shared_ptr<DX::D3DDevice>& deviceResources)
{
	m_pmain = std::unique_ptr<MainApp>(new MainApp(deviceResources));
	OnWindowSizeChanged();
}

void D3DApp::Update()
{
	m_timer.Tick([&]()
	{
		m_pmain->Update(m_timer);
	});
}

bool D3DApp::Render()
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}
	return m_pmain->Render();
}

void D3DApp::OnWindowSizeChanged()
{
	m_pmain->CreateWindowSizeDependentResources();
}

void D3DApp::OnDeviceRemoved()
{
	m_pmain = nullptr;
}
