#pragma once

#include <functional>
#include <memory>
#include <utility>
namespace svy {

	//���ڱ�ʾcallbackӦ����������
	class Flag {

	};
	//������
	template<typename T>
	class WeakCall {
	public:
		WeakCall(const std::weak_ptr<Flag> &flag, const T &t)
			:mFlag_(flag), mF_(t) {

		}
		//��ֵ���ô���
		WeakCall(const std::weak_ptr<Flag> &flag, T&& t)
			:mFlag_(flag), mF_(std::move(t)) {

		}
		WeakCall(const WeakCall& wc)
			:mFlag_(wc.mFlag_), mF_(wc.mF_)	{

		}

		template<typename... Args>
		auto operator()(Args&&... a) const -> decltype(mF_(std::forward<Args>(a)...)) {
			if (!mFlag_.expired()) {
				return mF_(std::forward<Args>(a)...);
			}
			return decltype(mF_(std::forward<Args>(a)...));
		}
		bool Expired() const {
			return mFlag_.expired();
		}
	public:
		std::weak_ptr<Flag>  mFlag_;
		mutable T mF_;
	};




};