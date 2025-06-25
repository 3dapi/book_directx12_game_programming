
#pragma once
#ifndef __G2_FACTORY_H__
#define __G2_FACTORY_H__

#include <string>
#include <unordered_map>

namespace G2 {

template <typename T>
class IG2Factory
{
protected:
	std::unordered_map<std::string, std::unique_ptr<T>> m_db;
public:
	T* Load(const std::any& optional)
	{
		return ResourceLoad(optional);
	}
	T* Find(const std::string& name)
	{
		return ResourceFind(name);
	}
	int UnLoad(const std::string& name)
	{
		return ResourceUnLoad(name);
	}
	int UnLoadAll()
	{
		return ResourceUnLoadAll();
	}
protected:
	virtual T* ResourceLoad(const std::any& optional)   = 0;
	virtual T* ResourceFind(const std::string& name)    = 0;
	virtual int ResourceUnLoad(const std::string& name) = 0;
	virtual int ResourceUnLoadAll()                     = 0;
};

} // namespace G2
#endif // __G2_FACTORY_H__
