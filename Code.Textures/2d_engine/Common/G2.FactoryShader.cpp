
#include <tuple>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "d3dUtil.h"
#include "G2.FactoryShader.h"
#include "G2.Util.h"

namespace G2 {

FactoryShader* FactoryShader::instance()
{
	static FactoryShader inst;
	return &inst;
}

// optional: name, file, shader model, entrypoint
TD3D_SHADER* FactoryShader::ResourceLoad(const std::any& optional)
{
	auto d3d            = IG2GraphicsD3D::instance();
	auto d3dDevice      = std::any_cast<ID3D12Device*>(d3d->getDevice());
	auto d3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(d3d->getCommandList());
	auto [name, file, sm, ep]
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
	pItem->sm = sm;
	pItem->ep = ep;
	auto rs = DXCompileShaderFromFile(file, sm, ep);
	if (rs == nullptr)
		return {};

	pItem->rs.Attach(rs);
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
