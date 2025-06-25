#include "G2ConstantsWin.h"
#include "d3dUtil.h"
#include "FactoryTexture.h"

FactoryTexture* FactoryTexture::instance()
{
	static FactoryTexture inst;
	return &inst;
}

Texture* FactoryTexture::ResourceLoad(const std::any& optional)
{
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto [name, file]   = std::any_cast<std::tuple<std::string, std::string>>(optional);

	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}

	// load
	auto texture = std::make_unique<Texture>();
	texture->name = name;
	texture->file = file;

	ComPtr<ID3D12Resource> rs_tx{};
	ComPtr<ID3D12Resource> rs_up{};
	std::wstring wFile = ansiToWstr(file);
	HRESULT hr = DirectX::CreateDDSTextureFromFile12(d3dDevice, d3dCommandList, wFile.c_str(), rs_tx, rs_up);
	ThrowIfFailed(hr);

	texture->rs = std::move(rs_tx);
	texture->uh = std::move(rs_up);
	//c++17
	auto [it, success] = m_db.insert({ name, std::move(texture) });
	auto ret = it->second.get();
	return ret;
	//c++14
	//auto it = m_db.insert({ name, std::move(texture) });
	//auto ret = it.first;
	//bool success = it.second;
	//return ret->second.get(); // Texture*
}
Texture* FactoryTexture::ResourceFind(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	static Texture dummy{ "<none>", "<none>" };
	return &dummy;
}
int FactoryTexture::ResourceUnLoad(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		m_db.erase(itr);
		return S_OK;
	}
	return E_FAIL;
}
int FactoryTexture::ResourceUnLoadAll()
{
	m_db.clear();
	return S_OK;
}


