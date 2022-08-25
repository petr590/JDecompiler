#ifndef JDECOMPILER_GENERICS_TYPE_CPP
#define JDECOMPILER_GENERICS_TYPE_CPP

namespace jdecompiler {

	struct GenericType: ReferenceType {

		virtual string getVarName() const override {
			throw IllegalStateException("Seriously? Variable of unknown generic type?");
		}

		virtual bool isSubtypeOfImpl(const Type* other) const override {
			return instanceof<const ReferenceType*>(other);
		}
	};


	struct DefinedGenericType: GenericType {
		const ReferenceType* const type;

		DefinedGenericType(const char*& str): type(parseParameter(str)) {}
	};


	struct ExtendingGenericType: DefinedGenericType {
		ExtendingGenericType(const char*& str): DefinedGenericType(str) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return "? extends " + type->toString(classinfo);
		}

		virtual string toString() const override {
			return "ExtendingGenericType(" + type->toString() + ')';
		}
	};


	struct SuperGenericType: DefinedGenericType {
		SuperGenericType(const char*& str): DefinedGenericType(str) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return "? super " + type->toString(classinfo);
		}

		virtual string toString() const override {
			return "SuperGenericType(" + type->toString() + ')';
		}
	};


	struct AnyGenericType: GenericType {
		private:
			AnyGenericType() noexcept {}

		public:
			static const AnyGenericType* getInstance() {
				static const AnyGenericType instance;
				return &instance;
			}

			virtual string toString(const ClassInfo&) const override {
				return "?";
			}

			virtual string toString() const override {
				return "AnyGenericType";
			}
	};

}

#endif