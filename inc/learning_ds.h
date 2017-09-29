#ifndef __LEARNING_DS_STD_IMP_H__
#define __LEARNING_DS_STD_IMP_H__

namespace learning {
	//possible implemntation of enable_if

	template<bool B, typename T = void>
	struct enable_if{};

	template<typename T>
	struct enable_if<true, T> {
		using type = T;
	};

	template<class T>
	typename std::add_rvalue_reference<T>::type declval() noexcept;
}

#endif//__LEARNING_DS_STD_IMP_H__
