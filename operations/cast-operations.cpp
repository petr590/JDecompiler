#ifndef JDECOMPILER_CAST_OPERATIONS_CPP
#define JDECOMPILER_CAST_OPERATIONS_CPP

namespace jdecompiler {

	struct CastOperation: Operation {
		public:
			const Operation* const value;
			const Type* const type;
			const bool required;

		private:
			static inline const Operation* getValue(const Operation* value, const Type* type) {
				if((type == BYTE || type == SHORT || type == CHAR) && instanceof<const CastOperation*>(value)) {
					const CastOperation* castValue = static_cast<const CastOperation*>(value);
					if(castValue->type == INT && castValue->value->getReturnType()->isPrimitive())
						return castValue->value;
				}

				return value;
			}

			mutable bool implicit = false;

		public:
			CastOperation(const Operation* value, const Type* type, bool required):
					value(getValue(value, type)), type(type), required(required) {}

			CastOperation(const DecompilationContext& context, const Type* requiredType, const Type* type, bool required):
					CastOperation(context.stack.popAs(requiredType), type, required) {
			}

			virtual const Type* getReturnType() const override {
				return type;
			}

			virtual string toString(const StringifyContext& context) const override {
				return implicit ? value->toString(context) :
						'(' + type->toString(context.classinfo) + ')' + toStringPriority(value, context, Associativity::RIGHT);
			}

			virtual Priority getPriority() const override {
				return implicit ? value->getPriority() : Priority::CAST;
			}

			virtual void allowImplicitCast() const override {
				if(!required)
					implicit = true;
			}

			virtual const Type* getImplicitType() const override {
				return required ? Operation::getImplicitType() : value->getReturnType();
			}
	};


	struct CheckCastOperation: CastOperation {
		CheckCastOperation(const DecompilationContext& context, uint16_t index):
				CastOperation(context, AnyObjectType::getInstance(), parseReferenceType(context.constPool.get<ClassConstant>(index)->name), true) {}
	};
}

#endif
