#pragma once

#include <algorithm>
#include <vector>
#include <memory>
namespace svy
{
	class Locker
	{
	public:
		void Lock(){}
		void UnLock(){}
	};
	template< typename theItem,class theLock=Locker >
	class IListImpl
	{
	public:
		IListImpl(unsigned int nType = 0) : m_unType(nType)
		{	m_nSz = 0;		}
		~IListImpl()
		{	clear();		}
		std::shared_ptr<theItem> push_back()
		{
			std::shared_ptr<theItem> ret;
			ret = std::make_shared<theItem>()
			m_lock.Lock();
			m_lstC.push_back(ret);
			m_lock.UnLock();
			return ret;
		}
		void push_back(std::shared_ptr<theItem> one)
		{
			m_lock.Lock();
			m_lstC.push_back(one);
			m_lock.UnLock();
		}
		std::shared_ptr<theItem>	getLast() 
		{
			std::shared_ptr<theItem> ret;
			m_lock.Lock();
			if (m_lstC.empty())
			{
				m_lock.UnLock();
				return ret;
			}
			ret = m_lstC.back();
			m_lock.UnLock();
			return ret;
		}
		std::shared_ptr<theItem>	getBegin()
		{
			std::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( m_lstC.empty() )
			{
				m_lock.UnLock();
				return ret;
			}
			m_nSz = 0;
			ret = m_lstC[m_nSz];
			m_lock.UnLock();
			return ret;
		}
		std::shared_ptr<theItem>	 getNext()
		{
			std::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( m_nSz < m_lstC.size() )
			{
				ret = m_lstC[m_nSz];
				m_nSz++;
			}
			m_lock.UnLock();
			return ret;
		}
		std::shared_ptr<theItem>  at(unsigned int unV)
		{
			std::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( 0 == m_lstC.size() )
			{
				m_lock.UnLock();
				return ret;
			}
			if(unV < m_lstC.size())
			{
				ret = m_lstC.at(unV);
				m_lock.UnLock();
			}
			return ret;
		}
		std::shared_ptr<theItem>  operator[](unsigned int unV)
		{
			return at(unV);
		}
		void	reset()
		{	
			m_lock.Lock();
			m_nSz = 0;	
			m_lock.UnLock();
		}
		unsigned int count()
		{
			m_lock.Lock();
			unsigned int sz = m_lstC.size();	
			m_lock.UnLock();
			return sz;
		}
		unsigned int erase(std::weak_ptr<theItem> one)
		{
			unsigned int unR = 0;
			if (one.expired()) 
			{
				m_lock.Lock();
				unR = m_lstC.size();
				m_lock.UnLock();
				return unR;
			}
			m_lock.Lock();
			std::vector<std::shared_ptr<theItem>>::iterator it = std::find(m_lstC.begin(),m_lstC.end(), one.lock());
			if( it != m_lstC.end() )
			{
				if( m_nSz )
				{
					size_t nT = (it - m_lstC.begin());
					if( m_nSz >= nT )
						m_nSz--;
				}
				m_lstC.erase(it);
			}
			unR = m_lstC.size();
			m_lock.UnLock();
			return unR;
		}
		void clear()
		{
			m_lock.Lock();
			m_lstC.clear();				
			m_nSz = 0;
			m_lock.UnLock();
		}
		void  release()
		{	delete this;		}
		unsigned int getType(void)
		{	return m_unType;	}
	private:
		std::vector<std::shared_ptr<theItem>> m_lstC;
		size_t				  m_nSz;
		unsigned int		  m_unType;
		theLock               m_lock;
	};
};