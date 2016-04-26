#pragma once

#include <functional>
#include <memory>
#include <utility>
namespace svy {

	//用于表示callback应用生存周期
	class Flag {

	};
	//弱调用
	template<typename T>
	class WeakCall {
	public:
		WeakCall(const std::weak_ptr<Flag> &flag, const T &t)
			:mFlag_(flag), mF_(t) {

		}
		//右值引用处理
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

	class SupportWeakCall
	{
	public:
		virtual ~SupportWeakCall() {};

		template<typename CallbackType>
		auto ToWeakCall(const CallbackType& closure)
			->WeakCall<CallbackType>
		{
			return WeakCall<CallbackType>(GetWeakFlag(), closure);
		}

		std::weak_ptr<Flag> GetWeakFlag()
		{
			if (m_weakFlag.use_count() == 0) {
				m_weakFlag.reset((Flag*)NULL);
			}
			return m_weakFlag;
		}

	private:
		template<typename ReturnValue, typename... Param, typename Flag>
		static std::function<ReturnValue(Param...)> ConvertToWeakCall(
			const std::function<ReturnValue(Param...)>& callback, std::weak_ptr<Flag> expiredFlag)
		{
			auto weakCall = [expiredFlag, callback](Param... p) {
				if (!expiredFlag.expired()) {
					return callback(p...);
				}
				return ReturnValue();
			};
			return weakCall;
		}

	protected:
		std::shared_ptr<Flag> m_weakFlag;
	};

	class WeakCallFlag final : public SupportWeakCall
	{
	public:
		void Cancel()
		{
			m_weakFlag.reset();
		}

		bool HasUsed()
		{
			return m_weakFlag.use_count() != 0;
		}
	};
	// global function 
	template<class F, class... Args, class = typename std::enable_if<!std::is_member_function_pointer<F>::value>::type>
	auto Bind(F && f, Args && ... args)
		->decltype(std::bind(f, args...))
	{
		return std::bind(f, args...);
	}
	// const class member function 
	template<class R, class C, class... DArgs, class P, class... Args>
	auto Bind(R(C::*f)(DArgs...) const, P && p, Args && ... args)
		->WeakCall<decltype(std::bind(f, p, args...))>
	{
		std::weak_ptr<Flag> weak_flag = ((SupportWeakCall*)p)->GetWeakFlag();
		auto bind_obj = std::bind(f, p, args...);
		static_assert(std::is_base_of<SupportWeakCall, C>::value, "SupportWeakCall should be base of C");
		WeakCall<decltype(bind_obj)> weak_callback(weak_flag, std::move(bind_obj));
		return weak_callback;
	}
	// non-const class member function 
	template<class R, class C, class... DArgs, class P, class... Args>
	auto Bind(R(C::*f)(DArgs...), P && p, Args && ... args)
		->WeakCall<decltype(std::bind(f, p, args...))>
	{
		std::weak_ptr<Flag> weak_flag = ((SupportWeakCall*)p)->GetWeakFlag();
		auto bind_obj = std::bind(f, p, args...);
		static_assert(std::is_base_of<SupportWeakCall, C>::value, "SupportWeakCall should be base of C");
		WeakCall<decltype(bind_obj)> weak_callback(weak_flag, std::move(bind_obj));
		return weak_callback;
	}

};