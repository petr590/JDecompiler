#ifndef JDECOMPILER_TYPE_BY_BUILTIN_TYPE_H
#define JDECOMPILER_TYPE_BY_BUILTIN_TYPE_H

namespace jdecompiler {

	namespace builtinJavaTypes {
		/* Serves as a marker for the function typeByBuiltinJavaType */
		struct MarkerStruct {
			MarkerStruct() = delete;
			MarkerStruct(const MarkerStruct&) = delete;
			MarkerStruct& operator=(const MarkerStruct&) = delete;
		};

		struct Object: MarkerStruct {};
		struct String: MarkerStruct {};
		struct Class: MarkerStruct {};
		struct MethodType: MarkerStruct {};
		struct MethodHandle: MarkerStruct {};
	};

	template<typename>
	static const Type* typeByBuiltinJavaType();

	template<> inline const Type* typeByBuiltinJavaType<jbool>() { return BOOLEAN; }
	template<> inline const Type* typeByBuiltinJavaType<jint>() { return ANY_INT_OR_BOOLEAN; }
	template<> inline const Type* typeByBuiltinJavaType<jlong>() { return LONG; }
	template<> inline const Type* typeByBuiltinJavaType<jfloat>() { return FLOAT; }
	template<> inline const Type* typeByBuiltinJavaType<jdouble>() { return DOUBLE; }
	template<> inline const Type* typeByBuiltinJavaType<builtinJavaTypes::Object>() { return AnyObjectType::getInstance(); }
	template<> inline const Type* typeByBuiltinJavaType<builtinJavaTypes::String>() { return STRING; }
	template<> inline const Type* typeByBuiltinJavaType<builtinJavaTypes::Class>() { return CLASS; }
	template<> inline const Type* typeByBuiltinJavaType<builtinJavaTypes::MethodType>() { return METHOD_TYPE; }
	template<> inline const Type* typeByBuiltinJavaType<builtinJavaTypes::MethodHandle>() { return METHOD_HANDLE; }
	template<> inline const Type* typeByBuiltinJavaType<string>() { return STRING; }

	template<typename T>
	static inline const Type* exactTypeByBuiltinJavaType() {
		return typeByBuiltinJavaType<T>();
	}

	template<> inline const Type* exactTypeByBuiltinJavaType<jint>() { return INT; }
	template<> inline const Type* exactTypeByBuiltinJavaType<builtinJavaTypes::Object>() { return OBJECT; }

}

#endif
