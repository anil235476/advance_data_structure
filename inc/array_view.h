#ifndef __ARRAY_VIEW_H__
#define __ARRAY_VIEW_H__
#include <cassert>
#include <type_traits>

namespace experiment {

	namespace {

		// Determines if the given class has zero-argument .data() and .size() methods
		// whose return values are convertible to T* and size_t, respectively.
		template <class DS, typename T>
		class HasDataAndSize {
		private:
			template 
			<
				class C,
				typename std::enable_if
				<
					std::is_convertible
					<
						decltype(std::declval<C>().data()), 
						T*
					>::value 
					&&
					std::is_convertible
					<
						decltype(std::declval<C>().size()),
						std::size_t
					>::value
				>::type* = nullptr
			>
			static int Test(int);

			template <typename>
			static char Test(...);

		public:
			static constexpr bool value = std::is_same<decltype(Test<DS>(0)), int>::value;
		};
	}

	namespace impl {

		// Magic constant for indicating that the size of an ArrayView is variable
		// instead of fixed.
		enum : std::ptrdiff_t { kArrayViewVarSize = -4711 };

		// Base class for ArrayViews of fixed nonzero size.
		template <typename T, std::ptrdiff_t Size>
		class ArrayViewBase {
			static_assert(Size > 0, "ArrayView size must be variable or non-negative");

		public:
			ArrayViewBase(T* data, size_t size) : data_(data) {}

			static constexpr size_t size() { return Size; }
			static constexpr bool empty() { return false; }
			T* data() const { return 
				 ; }

		protected:
			static constexpr bool fixed_size() { return true; }

		private:
			T* data_;
		};

		// Specialized base class for ArrayViews of fixed zero size.
		template <typename T>
		class ArrayViewBase<T, 0> {
		public:
			explicit ArrayViewBase(T* data, size_t size) {}

			static constexpr size_t size() { return 0; }
			static constexpr bool empty() { return true; }
			T* data() const { return nullptr; }

		protected:
			static constexpr bool fixed_size() { return true; }
		};

		// Specialized base class for ArrayViews of variable size.
		template <typename T>
		class ArrayViewBase<T, impl::kArrayViewVarSize> {
		public:
			ArrayViewBase(T* data, size_t size)
				: data_(size == 0 ? nullptr : data), size_(size) {}

			size_t size() const { return size_; }
			bool empty() const { return size_ == 0; }
			T* data() const { return data_; }

		protected:
			static constexpr bool fixed_size() { return false; }

		private:
			T* data_;
			size_t size_;
		};

	}  // namespace impl

	template <typename T, std::ptrdiff_t Size = impl::kArrayViewVarSize>
	class ArrayView final : public impl::ArrayViewBase<T, Size> {
	public:
		using value_type = T;
		using const_iterator = const T*;

		// Construct an ArrayView from a pointer and a length.
		template <typename U>
		ArrayView(U* data, size_t size)
			: impl::ArrayViewBase<T, Size>::ArrayViewBase(data, size) {
			assert(size == 0 ? nullptr : data == this->data());
			assert(size == this->size());
			assert(this->size() == 0? !this->data(): this->data());  // data is null iff size == 0.
		}

		// Construct an empty ArrayView. Note that fixed-size ArrayViews of size > 0
		// cannot be empty.
		ArrayView() : ArrayView(nullptr, 0) {}
		ArrayView(std::nullptr_t) : ArrayView() {}
		ArrayView(std::nullptr_t, size_t size)
			: ArrayView(static_cast<T*>(nullptr), size) {
			static_assert(Size == 0 || Size == impl::kArrayViewVarSize, "");
			assert(0 == size);
		}

		// Construct an ArrayView from an array.
		template <typename U, size_t N>
		ArrayView(U(&array)[N]) : ArrayView(array, N) {
			static_assert(Size == N || Size == impl::kArrayViewVarSize,
				"Array size must match ArrayView size");
		}

		// (Only if size is fixed.) Construct an ArrayView from any type U that has a
		// static constexpr size() method whose return value is equal to Size, and a
		// data() method whose return value converts implicitly to T*. In particular,
		// this means we allow conversion from ArrayView<T, N> to ArrayView<const T,
		// N>, but not the other way around. We also don't allow conversion from
		// ArrayView<T> to ArrayView<T, N>, or from ArrayView<T, M> to ArrayView<T,
		// N> when M != N.
		template <typename U,
			typename std::enable_if<
			Size != impl::kArrayViewVarSize &&
			HasDataAndSize<U, T>::value>::type* = nullptr>
			ArrayView(U& u) : ArrayView(u.data(), u.size()) {
			static_assert(U::size() == Size, "Sizes must match exactly");
		}

		// (Only if size is variable.) Construct an ArrayView from any type U that
		// has a size() method whose return value converts implicitly to size_t, and
		// a data() method whose return value converts implicitly to T*. In
		// particular, this means we allow conversion from ArrayView<T> to
		// ArrayView<const T>, but not the other way around. Other allowed
		// conversions include
		// ArrayView<T, N> to ArrayView<T> or ArrayView<const T>,
		// std::vector<T> to ArrayView<T> or ArrayView<const T>,
		// const std::vector<T> to ArrayView<const T>,
		// rtc::Buffer to ArrayView<uint8_t> or ArrayView<const uint8_t>, and
		// const rtc::Buffer to ArrayView<const uint8_t>.
		template <
			typename U,
			typename std::enable_if<Size == impl::kArrayViewVarSize &&
			HasDataAndSize<U, T>::value>::type* = nullptr>
			ArrayView(U& u) : ArrayView(u.data(), u.size()) {}

		// Indexing and iteration. These allow mutation even if the ArrayView is
		// const, because the ArrayView doesn't own the array. (To prevent mutation,
		// use a const element type.)
		T& operator[](size_t idx) const {
			assert(idx < this->size());
			assert(this->data());
			return this->data()[idx];
		}
		T* begin() const { return this->data(); }
		T* end() const { return this->data() + this->size(); }
		const T* cbegin() const { return this->data(); }
		const T* cend() const { return this->data() + this->size(); }

		ArrayView<T> subview(size_t offset, size_t size) const {
			return offset < this->size()
				? ArrayView<T>(this->data() + offset,
					std::min(size, this->size() - offset))
				: ArrayView<T>();
		}
		ArrayView<T> subview(size_t offset) const {
			return subview(offset, this->size());
		}
	};

	// Comparing two ArrayViews compares their (pointer,size) pairs; it does *not*
	// dereference the pointers.
	template <typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
	bool operator==(const ArrayView<T, Size1>& a, const ArrayView<T, Size2>& b) {
		return a.data() == b.data() && a.size() == b.size();
	}
	template <typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
	bool operator!=(const ArrayView<T, Size1>& a, const ArrayView<T, Size2>& b) {
		return !(a == b);
	}

	// Variable-size ArrayViews are the size of two pointers; fixed-size ArrayViews
	// are the size of one pointer. (And as a special case, fixed-size ArrayViews
	// of size 0 require no storage.)
	static_assert(sizeof(ArrayView<int>) == 2 * sizeof(int*), "");
	static_assert(sizeof(ArrayView<int, 17>) == sizeof(int*), "");
	static_assert(std::is_empty<ArrayView<int, 0>>::value, "");

	template <typename T>
	inline ArrayView<T> MakeArrayView(T* data, size_t size) {
		return ArrayView<T>(data, size);
	}

}  // namespace experiment

#endif  // __ARRAY_VIEW_H__
