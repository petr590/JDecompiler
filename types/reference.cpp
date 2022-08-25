#ifndef JDECOMPILER_REFERENCE_TYPE_CPP
#define JDECOMPILER_REFERENCE_TYPE_CPP

namespace jdecompiler {

	struct ReferenceType: BasicType {
		protected:
			ReferenceType(const string& encodedName, const string& name): BasicType(encodedName, name) {}

			ReferenceType(): BasicType(EMPTY_STRING, EMPTY_STRING) {}

		public:
			virtual TypeSize getSize() const override final {
				return TypeSize::FOUR_BYTES;
			}

			virtual string getClassEncodedName() const {
				return encodedName;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}
	};

}

#endif