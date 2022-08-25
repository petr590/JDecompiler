#ifndef JDECOMPILER_INVOKE_OPERATIONS_CPP
#define JDECOMPILER_INVOKE_OPERATIONS_CPP

namespace jdecompiler {

	struct InvokeOperation: Operation {
		public:
			const MethodDescriptor& descriptor;
			const vector<const Operation*> arguments;
			const bool isStatic;

		protected:
			const vector<const Operation*> popArguments(const DecompilationContext& context) const {
				const uint32_t size = descriptor.arguments.size();

				vector<const Operation*> arguments;
				arguments.reserve(size);

				for(uint32_t i = size; i > 0; ) {
					const Type* requiredType = descriptor.arguments[--i];

					const Operation* argument = context.stack.popAs(requiredType);
					const Type* argType = argument->getReturnType();

					if(argType != requiredType && requiredType->isIntegral()) {
						const IntegralType* integralRequiredType = safe_cast<const IntegralType*>(requiredType);

						if((argType->isIntegral() && safe_cast<const IntegralType*>(argType)->getCapacity() < integralRequiredType->getCapacity()) ||
							(argType == CHAR && integralRequiredType->getCapacity() > VariableCapacityIntegralType::CHAR_CAPACITY)) {

							argument = new CastOperation(argument, requiredType, false);
						}
					}

					arguments.push_back(argument);
				}

				return arguments;
			}

			InvokeOperation(const DecompilationContext& context, const MethodDescriptor& descriptor, bool isStatic):
					descriptor(descriptor), arguments(popArguments(context)), isStatic(isStatic) {}

			InvokeOperation(const DecompilationContext& context, uint16_t index, bool isStatic):
					InvokeOperation(context, context.constPool.get<MethodrefConstant>(index), isStatic) {}

			inline string argumentsToString(const StringifyContext& context) const {
				if(!arguments.empty()) {

					const Class* clazz = JDecompiler::getInstance().getClass(descriptor.clazz.getEncodedName());

					if(clazz != nullptr) {
						const vector<const Method*> overloadedMethods = clazz->getMethods([this] (const Method* method) {
							// From a non-static context, we can invoke both non-static and static methods
							return (!this->isStatic || method->isStatic()) && method->descriptor.name == this->descriptor.name
									&& method->descriptor.arguments.size() == this->descriptor.arguments.size() && method->descriptor != this->descriptor;
						});

						const function<void()> allowAllImplicitCast = [this] () {
							for(const Operation* argument : arguments)
								argument->allowImplicitCast();
						};

						if(overloadedMethods.empty()) {
							allowAllImplicitCast();

						} else {
							using status_t = Type::status_t;

							vector<const Type*> implicitTypes;
							implicitTypes.reserve(arguments.size());

							for(auto it = arguments.end(), begin = arguments.begin(); it != begin; )
								implicitTypes.push_back((*(--it))->getImplicitType());

							if(implicitTypes != descriptor.arguments) {

								const function<bool(const vector<const Type*>&, const vector<const Type*>&, status_t*)> canImplicitCast =
									[] (const vector<const Type*>& types1, const vector<const Type*>& types2, status_t* status) {
										return equal(types1.begin(), types1.end(), types2.begin(),
											[status] (const Type* type1, const Type* type2) {
												const status_t currentStatus = type1->implicitCastStatus(type2);
												*status += currentStatus;
												return currentStatus != Type::N_STATUS;
											});
								};


								map<const Method*, status_t> resolvedMethods;

								status_t baseStatus = 0;

								if(canImplicitCast(implicitTypes, descriptor.arguments, &baseStatus)) {

									for(const Method* method : overloadedMethods) {
										status_t status = 0;

										if(canImplicitCast(implicitTypes, method->descriptor.arguments, &status)) {
											resolvedMethods[method] = status;
										}
									}

									if(resolvedMethods.empty() || all_of(resolvedMethods.begin(), resolvedMethods.end(),
											[baseStatus] (const auto& it) { return it.second > baseStatus; })) {

										allowAllImplicitCast();
									}
								}
							}
						}
					}
				}

				return '(' + rjoin<const Operation*>(arguments, [&context] (const Operation* operation) { return operation->toString(context); }) + ')';
			}

		public:
			virtual const Type* getReturnType() const override {
				return descriptor.returnType;
			}

			template<size_t index>
			const Operation* getArgument() const {
				return index + 1 < arguments.size() ? arguments[arguments.size() - (index + 1)] : nullptr;
			}
	};


	struct InvokeNonStaticOperation: InvokeOperation {
		public:
			const Operation* const object;

		protected:
			/* Do not delegate the constructor, otherwise the arguments and the object
			   will be popped from the stack in the wrong order */
			InvokeNonStaticOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
					InvokeOperation(context, descriptor, false), object(context.stack.popAs(&descriptor.clazz)) {}

			InvokeNonStaticOperation(const DecompilationContext& context, const MethodDescriptor& descriptor, const Operation* object):
					InvokeOperation(context, descriptor, false), object(object) {
				object->castReturnTypeTo(&descriptor.clazz);
			}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return (object->isReferenceToThis(context) && JDecompiler::getInstance().omitReferenceToThis() ?
						EMPTY_STRING : toStringPriority(object, context, Associativity::LEFT) + '.') + descriptor.name + argumentsToString(context);
			}
	};


	struct InvokevirtualOperation: InvokeNonStaticOperation {
		InvokevirtualOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
				InvokeNonStaticOperation(context, descriptor) {

			if(descriptor.clazz == *STRING && (descriptor.name == "indexOf" || descriptor.name == "lastIndexOf") &&
					(equal_values(descriptor.arguments, {INT}) || equal_values(descriptor.arguments, {INT, INT}))) {

				const Operation* charOperation = arguments.back();

				if(instanceof<const IConstOperation*>(charOperation)) {
					const IConstOperation* iconstOperation = static_cast<const IConstOperation*>(charOperation);

					if((char16_t)iconstOperation->value == iconstOperation->value) {
						iconstOperation->castReturnTypeTo(CHAR);
					}
				}
			}
		}
	};


	struct InvokespecialOperation: InvokeNonStaticOperation {
		public:
			const bool isConstructor, isSuperConstructor, isEnumSuperConstructor;

			const Type* const returnType;

		private:
			inline bool getIsConstructor() {
				return descriptor.isConstructor();
			}

			inline bool getIsSuperConstructor(const DecompilationContext& context) const {
				return (object->isReferenceToThis(context) && // check that we invoking this (or super) constructor
						context.classinfo.superType != nullptr && descriptor.clazz == *context.classinfo.superType);
			}

			inline bool getIsEnumSuperConstructor(const DecompilationContext& context) const {
				return isSuperConstructor && context.classinfo.modifiers & ACC_ENUM;
			}

			inline const Type* getReturnType(const DecompilationContext& context) const {
				return isConstructor && checkDup<Dup1Operation>(context, object) ?
						object->getReturnType() : InvokeNonStaticOperation::getReturnType();
			}


			inline void checkEnumSuperConstructor(const DecompilationContext& context) const {
				if((isEnumSuperConstructor && arguments.size() != 2) || *descriptor.arguments[0] != *STRING || *descriptor.arguments[1] != *INT) {
					context.warning("invocation of non-standart enum super constructor");
				}
			}

			inline void init() const {
				if((descriptor.clazz == javaLang::Exception || descriptor.clazz == javaLang::Throwable)) {

					const Operation *firstArgument = getArgument<0>(), *secondArgument = getArgument<1>(),
							*thirdArgument = getArgument<2>(), *fourthArgument = getArgument<3>();

					if(firstArgument != nullptr) {
						if(*descriptor.arguments[0] == *STRING)
							firstArgument->addVariableName("message");

						else if(*descriptor.arguments[0] == javaLang::Throwable)
							firstArgument->addVariableName("cause");
					}

					if(secondArgument != nullptr && *descriptor.arguments[1] == javaLang::Throwable)
						secondArgument->addVariableName("cause");

					if(thirdArgument != nullptr && *descriptor.arguments[2] == *BOOLEAN)
						thirdArgument->addVariableName("enableSuppression");

					if(fourthArgument != nullptr && *descriptor.arguments[2] == *BOOLEAN)
						fourthArgument->addVariableName("writableStackTrace");
				}
			}

		public:
			InvokespecialOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
					InvokeNonStaticOperation(context, descriptor),
					isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(context)),
					isEnumSuperConstructor(getIsEnumSuperConstructor(context)), returnType(getReturnType(context)) {
				init();
			}

			InvokespecialOperation(const DecompilationContext& context, const MethodDescriptor& descriptor, const Operation* object):
					InvokeNonStaticOperation(context, descriptor, object),
					isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(context)),
					isEnumSuperConstructor(getIsEnumSuperConstructor(context)), returnType(getReturnType(context)) {
				init();
			}

			virtual string toString(const StringifyContext& context) const override {
				if(isConstructor) {
					if(const NewOperation* newOperation = dynamic_cast<const NewOperation*>(object->getOriginalOperation())) {
						const ClassType& classType = newOperation->clazz;
						if(classType.isAnonymous) {
							const Class* clazz = JDecompiler::getInstance().getClass(classType.getEncodedName());
							if(clazz != nullptr) {
								clazz->classinfo.copyFormattingFrom(context.classinfo);
								const string result = "new " + clazz->anonymousToString();
								clazz->classinfo.resetFormatting();

								return result;
							} else {
								context.warning("cannot load inner class " + classType.getName());
							}
						}
					}

					return (isSuperConstructor ? "super" : toStringPriority(object, context, Associativity::LEFT)) + argumentsToString(context);
				}

				if(object->isReferenceToThis(context) && context.classinfo.superType != nullptr &&
						descriptor.clazz == *context.classinfo.superType)
					return "super." + descriptor.name + argumentsToString(context);

				return InvokeNonStaticOperation::toString(context);
			}

			virtual const Type* getReturnType() const override {
				return returnType;
			}

			virtual bool canAddToCode() const override {
				return !((isSuperConstructor && arguments.empty()) || (isEnumSuperConstructor && arguments.size() == 2));
			}
	};


	struct InvokestaticOperation: InvokeOperation {
		public:
			InvokestaticOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
					InvokeOperation(context, descriptor, true) {}

			virtual string toString(const StringifyContext& context) const override {
				return (descriptor.clazz == context.classinfo.thisType ? EMPTY_STRING : descriptor.clazz.toString(context.classinfo) + '.')
						+ descriptor.name + argumentsToString(context);
			}
	};


	struct InvokeinterfaceOperation: InvokeNonStaticOperation {
		InvokeinterfaceOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
				InvokeNonStaticOperation(context, descriptor) {}
	};


	struct ConcatStringsOperation: InvokeOperation {
		public:
			const StringConstOperation* const pattern;

		protected:
			mutable vector<const Operation*> operands;
			friend struct StoreOperation;
			friend struct PutFieldOperation;

		public:
			ConcatStringsOperation(const DecompilationContext& context, const MethodDescriptor& concater,
					const StringConstOperation* pattern, const vector<const Operation*>& staticArguments):
					InvokeOperation(context, concater, true), pattern(pattern) {

				auto arg = arguments.end();
				auto staticArg = staticArguments.begin();

				string str;

				for(const char* cp = pattern->value.c_str(); *cp != '\0'; cp++) {
					if(*cp == '\1' || *cp == '\2') {
						if(!str.empty()) {
							operands.push_back(new StringConstOperation(str));
							str.clear();
						}

						switch(*cp) {
							case '\1': operands.push_back(*(--arg)); break;
							case '\2': operands.push_back(*(staticArg++)); break;
							default: throw Exception("WTF??"); // 🙃️
						}

					} else {
						str += *cp;
					}
				}

				if(!str.empty())
					operands.push_back(new StringConstOperation(str));

				insertEmptyStringIfNecessary();
			}

		protected:
			void insertEmptyStringIfNecessary() const {
				if((operands.size() == 1 && *operands[0]->getReturnType() != *STRING) ||
					(operands.size() > 1 && *operands[0]->getReturnType() != *STRING && *operands[1]->getReturnType() != *STRING)) {

					operands.insert(operands.begin(), EmptyStringConstOperation::getInstance());
				}
			}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return join<const Operation*>(operands,
						[this, &context] (const Operation* operation) { return toStringPriority(operation, context, Associativity::RIGHT); }, " + ");
			}

			virtual const Type* getReturnType() const override {
				return STRING;
			}

			virtual Priority getPriority() const override {
				return Priority::PLUS;
			}
	};
}

#endif
