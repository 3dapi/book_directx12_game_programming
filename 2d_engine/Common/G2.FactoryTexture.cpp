
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "DDSTextureLoader.h"
#include "G2.FactoryTexture.h"
#include "G2.Util.h"

using std::string;
namespace G2 {

FactoryTexture* FactoryTexture::instance()
{
	static FactoryTexture inst;
	return &inst;
}

TD3D_TEXTURE* FactoryTexture::ResourceLoad(const std::string& name, const std::string& file)
{
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());

	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}

	// load
	auto pItem = std::make_unique<TD3D_TEXTURE>();
	pItem->name = name;
	pItem->file = file;

	ComPtr<ID3D12Resource> rs_tx{};
	ComPtr<ID3D12Resource> rs_up{};
	std::wstring wFile = ansiToWstr(file);
	HRESULT hr = DirectX::CreateDDSTextureFromFile12(d3dDevice, d3dCommandList, wFile.c_str(), rs_tx, rs_up);
	ThrowIfFailed(hr);

	pItem->rs = std::move(rs_tx);
	pItem->uh = std::move(rs_up);
	//c++17
	auto [it, success] = m_db.insert({ name, std::move(pItem) });
	auto ret = it->second.get();
	return ret;
	//c++14
	//auto it = m_db.insert({ name, std::move(pItem) });
	//auto ret = it.first;
	//bool success = it.second;
	//return ret->second.get(); // TD3D_TEXTURE*
}
TD3D_TEXTURE* FactoryTexture::ResourceFind(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	static TD3D_TEXTURE dummy{ "<none>", "<none>" };
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

ID3D12Resource* FactoryTexture::FindRes(const std::string& name)
{
	auto item = this->Find(name)->rs.Get();
	if (item)
		return item;
	return {};
}

} // namespace G2
