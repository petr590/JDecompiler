#ifndef JDECOMPILER_INTEGRAL_TYPE_CPP
#define JDECOMPILER_INTEGRAL_TYPE_CPP

namespace jdecompiler {

	/*
		An integral type is an signed integer type that occupies 4 bytes on the stack: int, short and byte.
		Boolean and char are not included in this list, they are processed separately
	*/
	struct IntegralType: PrimitiveType {
		private:
			IntegralType(const string& encodedName, const string& name, const string& varName):
					PrimitiveType(encodedName, name, varName) {}

			friend struct ByteType;
			friend struct ShortType;
			friend struct IntType;
			friend struct LongType;

		public:
			virtual uint8_t getCapacity() const = 0;

			virtual bool isIntegral() const override final {
				return true;
			}
	};
}

#endif
