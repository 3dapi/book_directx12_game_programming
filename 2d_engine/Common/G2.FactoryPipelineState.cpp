
#include <tuple>
#include <string>
#include "d3dUtil.h"
#include "G2.ConstantsWin.h"
#include "G2.FactoryPipelineState.h"
#include "G2.Util.h"

using std::string;

namespace G2 {

FactoryPipelineState* FactoryPipelineState::instance()
{
	static FactoryPipelineState inst;
	return &inst;
}

TD3D_PS* FactoryPipelineState::ResourceLoad(const string& name)
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
	auto pItem = std::make_unique<TD3D_PS>();
	pItem->name = name;





	auto [it, success] = m_db.insert({ name, std::move(pItem) });
	auto ret = it->second.get();
	return ret;
}
TD3D_PS* FactoryPipelineState::ResourceFind(const string& name)
{
	auto itr = this->m_db.find(name);
	if (itr != this->m_db.end())
	{
		return itr->second.get();
	}
	static TD3D_PS dummy{ "<none>", };
	return &dummy;
}


} // namespace G2
