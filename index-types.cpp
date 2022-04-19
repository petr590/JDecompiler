#ifndef JDECOMPILER_INDEX_TYPES_CPP
#define JDECOMPILER_INDEX_TYPES_CPP

namespace jdecompiler {

#	ifdef STRICT_INDEX_TYPES

	enum class index_marker_t {
		POS_MARKER, INDEX_MARKER, OFFSET_MARKER
	};

	template<typename T, index_marker_t M>
	struct abstract_index_impl;


	typedef abstract_index_impl<uint32_t, index_marker_t::INDEX_MARKER> index_t;
	typedef abstract_index_impl<uint32_t, index_marker_t::POS_MARKER> pos_t;
	typedef abstract_index_impl<uint32_t, index_marker_t::OFFSET_MARKER> offset_t;


	template<typename T>
	struct abstract_index_t {
		static_assert(is_integral<T>(), "Type must be integral");

		protected:
			T value;

		public:
			inline constexpr abstract_index_t() noexcept: value(0) {}
			inline constexpr abstract_index_t(T value) noexcept: value(value) {}

			inline constexpr abstract_index_t(const abstract_index_t<T>& val) noexcept = default;


			inline ~abstract_index_t() = default;

			inline operator T() const {
				return value;
			}

		template<index_marker_t M>
		inline friend abstract_index_impl<T, M>& operator++ (abstract_index_impl<T, M>& val) {
			val.value++;
			return val;
		}

		template<index_marker_t M>
		inline friend abstract_index_impl<T, M>& operator-- (abstract_index_impl<T, M>& val) {
			val.value--;
			return val;
		}


		template<index_marker_t M>
		inline friend abstract_index_impl<T, M> operator++ (abstract_index_impl<T, M>& val, int) {
			abstract_index_impl<T, M> copy(val);
			val.value++;
			return copy;
		}

		template<index_marker_t M>
		inline friend abstract_index_impl<T, M> operator-- (abstract_index_impl<T, M>& val, int) {
			abstract_index_impl<T, M> copy(val);
			val.value--;
			return copy;
		}


		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M> operator+ (uint32_t val1, const abstract_index_impl<T, M>& val2) {
			return abstract_index_impl<T, M>(val1 + val2.value);
		}

		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M> operator+ (const abstract_index_impl<T, M>& val1, uint32_t val2) {
			return abstract_index_impl<T, M>(val1.value + val2);
		}


		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M> operator- (uint32_t val1, const abstract_index_impl<T, M>& val2) {
			return abstract_index_impl<T, M>(val1 - val2.value);
		}

		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M> operator- (const abstract_index_impl<T, M>& val1, uint32_t val2) {
			return abstract_index_impl<T, M>(val1.value - val2);
		}


		inline friend constexpr pos_t operator+ (const pos_t& pos, const offset_t& offset);
		inline friend constexpr pos_t operator- (const pos_t& pos, const offset_t& offset);


		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M>& operator+= (abstract_index_impl<T, M>& val1, uint32_t val2) {
			val1.value += val2;
			return val1;
		}

		template<index_marker_t M>
		inline friend constexpr abstract_index_impl<T, M>& operator-= (abstract_index_impl<T, M>& val1, uint32_t val2) {
			val1.value -= val2;
			return val1;
		}

		inline friend ostream& operator<< (ostream& out, const abstract_index_t& val) {
			return out << val.value;
		}

		inline friend string to_string(const abstract_index_t& val) {
			return to_string(val.value);
		}
	};


	template<typename T, index_marker_t M>
	struct abstract_index_impl final: abstract_index_t<T> {
		inline constexpr abstract_index_impl() {}
		inline constexpr abstract_index_impl(T value) noexcept: abstract_index_t<T>(value) {}

		inline constexpr abstract_index_impl(const abstract_index_impl& val) noexcept: abstract_index_t<T>(val.value) {}
	};


	template<>
	struct abstract_index_impl<uint32_t, index_marker_t::INDEX_MARKER>: abstract_index_t<uint32_t> {
		inline constexpr abstract_index_impl() {}
		inline constexpr abstract_index_impl(uint32_t value) noexcept: abstract_index_t<uint32_t>(value) {}

		inline constexpr abstract_index_impl(const abstract_index_impl& val) noexcept: abstract_index_t<uint32_t>(val.value) {}


		inline constexpr operator pos_t() const = delete;
		inline constexpr operator offset_t() const = delete;
	};

	template<>
	struct abstract_index_impl<uint32_t, index_marker_t::POS_MARKER>: abstract_index_t<uint32_t> {
		inline constexpr abstract_index_impl() {}
		inline constexpr abstract_index_impl(uint32_t value) noexcept: abstract_index_t<uint32_t>(value) {}

		inline constexpr abstract_index_impl(const abstract_index_impl& val) noexcept: abstract_index_t<uint32_t>(val.value) {}


		inline constexpr operator index_t() const = delete;
		inline constexpr operator offset_t() const = delete;


		inline friend constexpr pos_t operator+ (const pos_t& val1, const pos_t& val2) {
			return pos_t(val1.value + val2.value);
		}

		inline friend constexpr pos_t operator- (const pos_t& val1, const pos_t& val2) {
			return pos_t(val1.value - val2.value);
		}
	};

	template<>
	struct abstract_index_impl<int32_t, index_marker_t::OFFSET_MARKER>: abstract_index_t<int32_t> {
		inline constexpr abstract_index_impl() {}
		inline constexpr abstract_index_impl(int32_t value) noexcept: abstract_index_t<int32_t>(value) {}

		inline constexpr abstract_index_impl(const abstract_index_impl& val) noexcept: abstract_index_t<int32_t>(val.value) {}


		inline constexpr operator index_t() const = delete;
		inline constexpr operator pos_t() const {
			return pos_t(value);
		}
	};


	inline constexpr pos_t operator+ (const pos_t& pos, const offset_t& offset) {
		return pos_t(pos.value + offset.value);
	}

	inline constexpr pos_t operator- (const pos_t& pos, const offset_t& offset) {
		return pos_t(pos.value - offset.value);
	}

#	else

	template<typename T>
	using abstract_index_t = T;

	typedef uint32_t pos_t;
	typedef uint32_t index_t;
	typedef int32_t offset_t;

#	endif
}

#endif
