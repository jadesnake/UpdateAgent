#pragma once
//使用C++ 11 原子操作特性
#include <atomic> 
template<typename theIBase >
class IRef : public theIBase
{
public:
	IRef() : m_ref(1) {

	}
	virtual ~IRef()	{

	}
	inline long AddRef()
	{
		return (m_ref++);
	}
	inline long RelRef()
	{
		if( 0 == (m_ref--) )
		{
			delete this;
			return 0;
		}
		return m_ref;
	}
	inline long GetRef()
	{
		return m_ref;
	}
private:
	std::atomic_long m_ref;
};

class RefImpl
{
public:
	RefImpl() : m_ref(1) {

	}
	virtual ~RefImpl() {

	}
	inline long AddRef()
	{
		return (m_ref++);
	}
	inline long RelRef()
	{
		if (0 == (m_ref--))
		{
			delete this;
			return 0;
		}
		return m_ref;
	}
	inline long GetRef()
	{
		return m_ref;
	}
private:
	std::atomic_long m_ref;
};