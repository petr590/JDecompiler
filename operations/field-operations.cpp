#ifndef JDECOMPILER_FIELD_OPERATIONS_CPP
#define JDECOMPILER_FIELD_OPERATIONS_CPP

namespace jdecompiler::operations {

	struct FieldOperation: Operation {
		public:
			const ClassType clazz;
			const FieldDescriptor descriptor;

		protected:
			FieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
					clazz(clazz), descriptor(descriptor) {}

			FieldOperation(const FieldrefConstant* fieldref):
					FieldOperation(fieldref->clazz, fieldref->nameAndType) {}

			FieldOperation(const DecompilationContext& context, uint16_t index):
					FieldOperation(context.constPool.get<FieldrefConstant>(index)) {}

			inline string staticFieldToString(const StringifyContext& context) const {
				return clazz == context.classinfo.thisType && !context.getCurrentScope()->hasVariable(descriptor.name) ?
						descriptor.name : clazz.toString(context.classinfo) + '.' + descriptor.name;
			}

			inline string instanceFieldToString(const StringifyContext& context, const Operation* object) const {
				return object->isReferenceToThis(context.classinfo) && !context.getCurrentScope()->hasVariable(descriptor.name) ?
							descriptor.name : object->toString(context) + '.' + descriptor.name;
			}
	};


	struct GetFieldOperation: FieldOperation {
		protected:
			GetFieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
					FieldOperation(clazz, descriptor) {}

		public:
			virtual const Type* getReturnType() const override {
				return &descriptor.type;
			}
	};

	struct GetStaticFieldOperation: GetFieldOperation {
		public:
			GetStaticFieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
					GetFieldOperation(clazz, descriptor) {}

			GetStaticFieldOperation(const FieldrefConstant* fieldref):
					GetStaticFieldOperation(fieldref->clazz, fieldref->nameAndType) {}

			virtual string toString(const StringifyContext& context) const override {
				static const FieldDescriptor TypeField("TYPE", CLASS);

				if(descriptor == TypeField) {
					if(clazz == javaLang::Void) return "void.class";
					if(clazz == javaLang::Byte) return "byte.class";
					if(clazz == javaLang::Short) return "short.class";
					if(clazz == javaLang::Character) return "char.class";
					if(clazz == javaLang::Integer) return "int.class";
					if(clazz == javaLang::Long) return "long.class";
					if(clazz == javaLang::Float) return "float.class";
					if(clazz == javaLang::Double) return "double.class";
					if(clazz == javaLang::Boolean) return "boolean.class";
				}

				return staticFieldToString(context);
			}
	};

	struct GetInstanceFieldOperation: GetFieldOperation {
		public:
			const Operation* const object;

			GetInstanceFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
					GetFieldOperation(clazz, descriptor), object(context.stack.popAs(&clazz)) {}

			GetInstanceFieldOperation(const DecompilationContext& context, const FieldrefConstant* fieldref):
					GetInstanceFieldOperation(context, fieldref->clazz, fieldref->nameAndType) {}

			virtual string toString(const StringifyContext& context) const override {
				return instanceFieldToString(context, object);
			}
	};


	struct PutFieldOperation: FieldOperation {
		public:
			const Operation* const value;

		protected:
			const Type* returnType;

			PutFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
					FieldOperation(clazz, descriptor), value(context.stack.popAs(&descriptor.type)) {

				value->allowImplicitCast();
				value->addVariableName(descriptor.name);
			}

		public:
			virtual const Type* getReturnType() const override {
				return returnType;
			}

			virtual Priority getPriority() const override {
				return Priority::ASSIGNMENT;
			}
	};

	struct PutStaticFieldOperation: PutFieldOperation {
		public:
			PutStaticFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
					PutFieldOperation(context, clazz, descriptor) {
				returnType = getDupReturnType<Dup1Operation, Dup2Operation>(context, value);
			}

			PutStaticFieldOperation(const DecompilationContext& context, const MethodrefConstant* methodref):
					PutStaticFieldOperation(context, methodref->clazz, methodref->nameAndType) {}

			virtual string toString(const StringifyContext& context) const override {
				return staticFieldToString(context) + " = " + value->toString(context);
			}
	};

	struct PutInstanceFieldOperation: PutFieldOperation {
		public:
			const Operation* const object;

			PutInstanceFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& decriptor):
					PutFieldOperation(context, clazz, decriptor), object(context.stack.popAs(&clazz)) {
				returnType = getDupReturnType<DupX1Operation, Dup2X1Operation>(context, value);
			}

			PutInstanceFieldOperation(const DecompilationContext& context, const MethodrefConstant* methodref):
					PutInstanceFieldOperation(context, methodref->clazz, methodref->nameAndType) {}

			virtual string toString(const StringifyContext& context) const override {
				return instanceFieldToString(context, object) + " = " + value->toString(context);
			}
	};
}

#endif
