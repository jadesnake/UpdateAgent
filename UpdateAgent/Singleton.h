#pragma once

#include <atomic> 
template<class T>
class  Singleton
{
public:
	static T* get()
	{
		if (nullptr == m_instance)		{
			m_instance = new T();
		}
		return m_instance;
	}
	static void release() {
		if (m_instance == nullptr)
			return;
		if (0 == m_instance->delRef()) {
			delete m_instance;
			m_instance = nullptr;
		}
	}
	void addRef() {
		m_ref++;
	}
protected:
	Singleton() { m_ref = 1; }
	virtual ~Singleton() {}

	void delRef() {
		return (m_ref--);
	}
private:
	std::atomic_long	m_ref;
	static T * volatile m_instance; // The one and only instance
};
