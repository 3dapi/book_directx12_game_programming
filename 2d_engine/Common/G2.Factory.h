
#pragma once
#ifndef __G2_FACTORY_H__
#define __G2_FACTORY_H__

#include <string>
#include <unordered_map>

namespace G2 {

template <typename TCLASS_FACTORY, typename TDB_RESOURCE>
class IG2Factory
{
protected:
	std::unordered_map<std::string, std::unique_ptr<TDB_RESOURCE>> m_db;

public:
	template <typename... Args>
	TDB_RESOURCE* Load(Args&&... args)
	{
		return static_cast<TCLASS_FACTORY*>(this)->ResourceLoad(std::forward<Args>(args)...);
	}
	TDB_RESOURCE* Find(const std::string& name)
	{
		return static_cast<TCLASS_FACTORY*>(this)->ResourceFind(name);
	}
	int UnLoad(const std::string& name)
	{
		return static_cast<TCLASS_FACTORY*>(this)->ResourceUnLoad(name);
	}
	int UnLoadAll()
	{
		return static_cast<TCLASS_FACTORY*>(this)->ResourceUnLoadAll();
	}

public:
	// 파생 클래스에서 오버라이드 가능하도록 virtual 유지
	virtual TDB_RESOURCE* ResourceFind(const std::string& name)
	{
		auto it = m_db.find(name);
		return (it != m_db.end()) ? it->second.get() : nullptr;
	}
	virtual int ResourceUnLoad(const std::string& name)
	{
		return static_cast<int>(m_db.erase(name));
	}
	virtual int ResourceUnLoadAll()
	{
		m_db.clear();
		return 0;
	}
};


} // namespace G2
#endif // __G2_FACTORY_H__
