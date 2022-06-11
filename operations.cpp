#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

#include "class.cpp"

namespace jdecompiler::operations {

	template<class T = Type>
	struct ReturnableOperation: Operation { // ReturnableOperation is an operation which returns specified type
		static_assert(is_base_of<Type, T>::value, "template class T of struct ReturnableOperation is not subclass of class Type");

		protected:
			const T* const returnType;

		public:
			ReturnableOperation(const T* returnType): returnType(returnType) {}

			virtual const Type* getReturnType() const override { return returnType; }
	};

	struct IntOperation: Operation {
		inline IntOperation() {}

		virtual const Type* getReturnType() const override { return INT; }
	};

	struct AnyIntOperation: Operation {
		inline AnyIntOperation() {}

		virtual const Type* getReturnType() const override { return ANY_INT; }
	};

	struct BooleanOperation: Operation {
		inline BooleanOperation() {}

		virtual const Type* getReturnType() const override { return BOOLEAN; }
	};

	struct VoidOperation: Operation {
		VoidOperation() {}

		virtual const Type* getReturnType() const override { return VOID; }
	};


	struct TransientReturnableOperation: Operation {
		protected:
			mutable const Type* returnType;

			TransientReturnableOperation(const Type* returnType): returnType(returnType) {}
			TransientReturnableOperation(): returnType(VOID) {}

			template<class D, class... Ds>
			inline void initReturnType(const DecompilationContext& context, const Operation* operation) {
				if(returnType == VOID)
					returnType = getDupReturnType<D, Ds...>(context, operation);
			}

			virtual const Type* getReturnType() const override {
				return returnType;
			}
	};


	// ----------------------------------------------------------------------------------------------------


	template<TypeSize size>
	struct TypeSizeTemplatedOperation {
		protected:
			template<TypeSize S = size>
			void checkTypeSize(const Type* type) const {
				if(type->getSize() != S)
					throw TypeSizeMismatchException(TypeSize_nameOf(S), TypeSize_nameOf(type->getSize()), type->toString());
			}
	};
}

#include "operations/dup.cpp"
#include "operations/pop.cpp"

#include "operations/const-operations.cpp"
#include "operations/ldc.cpp"

#include "operations/load-operations.cpp"
#include "operations/array-load-operations.cpp"

#include "operations/operator-operations.cpp"
#include "operations/iinc.cpp"
#include "operations/cast-operations.cpp"
#include "operations/instanceof.cpp"

#include "operations/new.cpp"
#include "operations/new-array-operations.cpp"
#include "operations/array-store-operations.cpp"
#include "operations/invoke-operations.cpp"

#include "operations/store-operations.cpp"
#include "operations/try-catch-scopes.cpp"
#include "operations/field-operations.cpp"

#include "operations/conditions.cpp"

#include "operations/array-length.cpp"
#include "operations/athrow.cpp"
#include "operations/return-operations.cpp"

#include "operations/const-operation-definitions.cpp"

namespace jdecompiler {

	void StaticInitializerScope::addOperation(const Operation* operation, const StringifyContext& context) const {
		using namespace operations;

		if(!fieldsInitialized) {
			const PutStaticFieldOperation* putOperation = dynamic_cast<const PutStaticFieldOperation*>(operation);

			if(putOperation != nullptr && ClassType(putOperation->clazz) == context.classinfo.thisType) {
				const Field* field = context.classinfo.clazz.getField(putOperation->descriptor.name);

				if(field != nullptr) {
					if(field->initializer == nullptr) {
						field->initializer = putOperation->value;
						field->context = &context;
					} else {
						code.push_back(operation);
					}
				}
			} else {
				fieldsInitialized = true;
				code.push_back(operation);
			}
		} else {
			code.push_back(operation);
		}
	}
}

#endif
