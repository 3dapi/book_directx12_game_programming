#include <tuple>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "d3dUtil.h"
#include "G2.FactoryShader.h"

namespace G2 {

FactoryShader* FactoryShader::instance()
{
	static FactoryShader inst;
	return &inst;
}

TD3D_SHADER* FactoryShader::ResourceLoad(const std::any& optional)
{
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto [name, file, ep, tg]
						= std::any_cast<
								std::tuple<	std::string
											, std::string
											, std::string
											, std::string	> >(optional);

	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}

	// load
	auto pItem = std::make_unique<TD3D_SHADER>();
	pItem->name = name;
	pItem->file = file;
	pItem->ep = ep;
	pItem->tg = tg;

	ComPtr<ID3D12Resource> rs_tx{};
	ComPtr<ID3D12Resource> rs_up{};
	std::wstring wFile = ansiToWstr(file);
	auto rs = d3dUtil::CompileShader(wFile.c_str(), nullptr, ep, tg);
	if (rs.Get() == nullptr)
		return {};

	pItem->rs = std::move(rs);
	//c++17
	auto [it, success] = m_db.insert({ name, std::move(pItem) });
	auto ret = it->second.get();
	return ret;
}
TD3D_SHADER* FactoryShader::ResourceFind(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	static TD3D_SHADER dummy{ "<none>", "<none>" };
	return &dummy;
}
int FactoryShader::ResourceUnLoad(const std::string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		m_db.erase(itr);
		return S_OK;
	}
	return E_FAIL;
}
int FactoryShader::ResourceUnLoadAll()
{
	m_db.clear();
	return S_OK;
}

} // namespace G2
