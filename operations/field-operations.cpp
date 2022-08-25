#ifndef JDECOMPILER_FIELD_OPERATIONS_CPP
#define JDECOMPILER_FIELD_OPERATIONS_CPP

namespace jdecompiler {

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
				return object->isReferenceToThis(context) && !context.getCurrentScope()->hasVariable(descriptor.name) ?
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

			mutable int_fast8_t inc = 0;
			bool isStringAppend = false;

			const BinaryOperatorOperation* shortFormOperator = nullptr;
			const Operation* revokeIncrementOperation = nullptr;


			PutFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
					FieldOperation(clazz, descriptor), value(context.stack.popAs(&descriptor.type)) {

				value->allowImplicitCast();
				value->addVariableName(descriptor.name);

				// Merge with StoreOperation (duplicate code)

				bool isPostInc = returnType == VOID;

				const Operation* rawValue = value->getOriginalOperation();

				const BinaryOperatorOperation* binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(rawValue);

				if(binaryOperator == nullptr && instanceof<const CastOperation*>(rawValue))
					binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(static_cast<const CastOperation*>(rawValue)->value);

				if(binaryOperator != nullptr) {
					if(const GetFieldOperation* getFieldOperation = dynamic_cast<const GetFieldOperation*>(binaryOperator->operand1->getOriginalOperation())) {

						if(getFieldOperation->descriptor == descriptor && !getFieldOperation->isIncrement()) {
							shortFormOperator = binaryOperator;

							const AbstractConstOperation* constOperation = dynamic_cast<const AbstractConstOperation*>(binaryOperator->operand2);

							if(constOperation != nullptr) { // post increment
								int_fast8_t incValue = instanceof<const AddOperatorOperation*>(binaryOperator) ? 1 :
											instanceof<const SubOperatorOperation*>(binaryOperator) ? -1 : 0;

								if(incValue != 0) {
									//initReturnType<Dup1Operation, Dup2Operation>(context, binaryOperator->operand1);

									bool isShortInc =
										instanceof<const IConstOperation*>(constOperation) ?
											static_cast<const IConstOperation*>(constOperation)->value == 1 :
										instanceof<const LConstOperation*>(constOperation) ?
											static_cast<const LConstOperation*>(constOperation)->value == 1 :
										instanceof<const FConstOperation*>(constOperation) ?
											static_cast<const FConstOperation*>(constOperation)->value == 1 :
										instanceof<const DConstOperation*>(constOperation) ?
											static_cast<const DConstOperation*>(constOperation)->value == 1 : false;

									if(isShortInc) {
										inc = incValue << isPostInc; // post-inc if return type is void, else pre-inc
									} else if(returnType != VOID) {
										revokeIncrementOperation = constOperation;
										context.warning("cannot decompile bytecode exactly: it contains post-increment by a number,"
														"other than one or minus one");
									}
								}
							}
						}
					}

				} else if(const ConcatStringsOperation* concatOperation = dynamic_cast<const ConcatStringsOperation*>(rawValue)) {
					vector<const Operation*>& operands = concatOperation->operands;

					if(const GetFieldOperation* getFieldOperation = dynamic_cast<const GetFieldOperation*>(operands[0])) {
						if(getFieldOperation->descriptor == descriptor) {
							isStringAppend = true;
							operands.erase(operands.begin());
							if(operands.size() > 1)
								concatOperation->insertEmptyStringIfNecessary();
						}
					}
				}
			}

		public:
			virtual string toString(const StringifyContext& context) const override {

				if(shortFormOperator != nullptr) { // Increment
					if(inc != 0) {
						const string
								name = fieldToString(context),
								incStr = inc > 0 ? "++" : "--";

						return inc & 0x1 ? incStr + name : name + incStr;
					}

					return revokeIncrementOperation != nullptr ?
							'(' + shortFormOperator->toShortFormString(context, fieldToString(context)) + ") " + // x++ is equivalent (x += 1) - 1
									shortFormOperator->getOppositeOperator() + ' ' + revokeIncrementOperation->toString(context) :
							shortFormOperator->toShortFormString(context, fieldToString(context));
				}

				if(isStringAppend) {
					return fieldToString(context) + " += " + value->toString(context);
				}

				return fieldToString(context) + " = " + value->toString(context);
			}

			virtual string fieldToString(const StringifyContext&) const = 0;

			virtual const Type* getReturnType() const override {
				return returnType;
			}

			virtual Priority getPriority() const override {
				return Priority::ASSIGNMENT;
			}

			virtual bool isIncrement() const override {
				return inc != 0;
			}

			virtual bool canAddToCode() const {
				const Class* clazz = JDecompiler::getInstance().getClass(this->clazz.getEncodedName());
				if(clazz != nullptr) {
					const Field* field = clazz->getField(descriptor);
					if(field != nullptr)
						return !field->isSynthetic() || JDecompiler::getInstance().showSynthetic();
				}

				return true;
			}
	};

	struct PutStaticFieldOperation: PutFieldOperation {
		public:
			PutStaticFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
					PutFieldOperation(context, clazz, descriptor) {
				returnType = getDupReturnType<Dup1Operation, Dup2Operation>(context, value);
			}

			PutStaticFieldOperation(const DecompilationContext& context, const FieldrefConstant* fieldref):
					PutStaticFieldOperation(context, fieldref->clazz, fieldref->nameAndType) {}

			virtual string fieldToString(const StringifyContext& context) const override {
				return staticFieldToString(context);
			}
	};

	struct PutInstanceFieldOperation: PutFieldOperation {
		public:
			const Operation* const object;

			PutInstanceFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& decriptor):
					PutFieldOperation(context, clazz, decriptor), object(context.stack.popAs(&clazz)) {
				returnType = getDupReturnType<DupX1Operation, Dup2X1Operation>(context, value);
			}

			PutInstanceFieldOperation(const DecompilationContext& context, const FieldrefConstant* fieldref):
					PutInstanceFieldOperation(context, fieldref->clazz, fieldref->nameAndType) {}

			virtual string fieldToString(const StringifyContext& context) const override {
				return instanceFieldToString(context, object);
			}
	};
}

#endif
