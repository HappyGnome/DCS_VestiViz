#pragma once
#ifndef _TEMPLATEINTERCHANGEWRAPPER_H_
#define _TEMPLATEINTERCHANGEWRAPPER_H_

#include<memory>

template<template<typename> typename TType, typename... Types>
class TemplateInterchangeWrapper {

	template<typename T>
	struct Type {};

	template<typename T, typename...Rest>
	class AccessorBase : public AccessorBase<Rest...> {
	public:
		virtual std::shared_ptr<TType<T>> get(Type<T> spec) const { return nullptr; }

		using AccessorBase<Rest...>::get;
	};

	template<typename T>
	class AccessorBase<T> {
	public:
		virtual ~AccessorBase() = default;
		virtual std::shared_ptr<TType<T>> get(Type<T> spec) const { return nullptr; }
	};
	using WrappedRaw = AccessorBase<Types...>;
public:
	using Wrapped = std::unique_ptr<WrappedRaw>;

private:
	template<typename T>
	class Holder : public WrappedRaw {
		std::shared_ptr<TType<T>> mPtr;
	public:
		explicit Holder(std::shared_ptr<TType<T>> wrappedVal) :mPtr(wrappedVal) {};

		std::shared_ptr<TType<T>> get(Type<T> spec) const final {
			return mPtr;
		}
	};
public:
	template<typename T>
	static Wrapped Wrap(std::shared_ptr<TType<T>> value) {
		return std::make_unique<Holder<T>>(value);
	}

	template<typename T>
	static std::shared_ptr<TType<T>> Unwrap(Wrapped&& value) {
		if (value == nullptr) 
			return nullptr;
		return value->get(Type<T>());
	}
};
#endif