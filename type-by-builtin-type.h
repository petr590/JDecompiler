#ifndef JDECOMPILER_TYPE_BY_BUILTIN_TYPE_H
#define JDECOMPILER_TYPE_BY_BUILTIN_TYPE_H

namespace jdecompiler {

	namespace BuiltinTypes {
		/* Serves as a marker for the function typeByBuiltinType */
		struct MarkerStruct {
			MarkerStruct() = delete;
			MarkerStruct(const MarkerStruct&) = delete;
			MarkerStruct& operator= (const MarkerStruct&) = delete;
		};

		struct Object: MarkerStruct {};
		struct String: MarkerStruct {};
		struct Class: MarkerStruct {};
		struct MethodType: MarkerStruct {};
		struct MethodHandle: MarkerStruct {};
	};

	template<typename T>
	static const Type* typeByBuiltinType();

	template<> inline const Type* typeByBuiltinType<bool>() { return BOOLEAN; }
	template<> inline const Type* typeByBuiltinType<int32_t>() { return ANY_INT_OR_BOOLEAN; }
	template<> inline const Type* typeByBuiltinType<int64_t>() { return LONG; }
	template<> inline const Type* typeByBuiltinType<float>() { return FLOAT; }
	template<> inline const Type* typeByBuiltinType<double>() { return DOUBLE; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::Object>() { return AnyObjectType::getInstance(); }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::String>() { return STRING; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::Class>() { return CLASS; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::MethodType>() { return METHOD_TYPE; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::MethodHandle>() { return METHOD_HANDLE; }
	template<> inline const Type* typeByBuiltinType<string>() { return STRING; }

	template<typename T>
	static inline const Type* exactTypeByBuiltinType() {
		return typeByBuiltinType<T>();
	}

	template<>
	inline const Type* exactTypeByBuiltinType<int32_t>() { return INT; }

}

#endif
