#ifndef JDECOMPILER_CONST_POOL_CPP
#define JDECOMPILER_CONST_POOL_CPP

#include "util.cpp"
#include "jdecompiler-instance.cpp"
#include "primitive-to-string.cpp"

#define DEFINE_CONSTANT_NAME(name)\
	static constexpr const char* CONSTANT_NAME = #name;\
	virtual const char* getConstantName() const override {\
		return CONSTANT_NAME;\
	}

namespace jdecompiler {

	struct Constant {
		virtual const char* getConstantName() const = 0;

		virtual ~Constant() {}

		virtual uint8_t getPositions() const {
			return 1;
		}
	};

	struct InterConstant {
		virtual const Constant* createConstant(const ConstantPool& constPool) const = 0;
	};


	struct ConstantPool {
		public:
			const uint16_t size;

		private:
			const Constant** pool;
			const InterConstant** interPool;

		public:
			ConstantPool(ClassInputStream& instream);

		private:
			template<typename C>
			static inline constexpr void checkTemplate() {
				static_assert(is_base_of<Constant, C>(), "template type C of method ConstantPool::get0 is not subclass of class Constant");
			}

			inline void checkIndex(uint16_t index) const {
				if(index >= size)
					throw ConstantPoolIndexOutOfBoundsException(index, size);
			}


			mutable stack<uint16_t> backtrace;

			void initInterConstant(uint16_t index) const {
				if(pool[index] != nullptr)
					return;

				if(backtrace.has(index)) {
					string backtraceString;

					for(const uint16_t backtraceIndex : backtrace)
						backtraceString += "\n#" + to_string(backtraceIndex);

					throw ConstantPoolInitializingException("Recursion detected while initializing a constant pool: " + backtraceString);
				}

				if(interPool[index] == nullptr)
					throw ConstantPoolInitializingException("Cannot initialize constant #" + to_string(index) + ": inter constant is null");

				pool[index] = interPool[index]->createConstant(*this);
			}


			template<class C>
			const C* get0(uint16_t index) const {
				checkTemplate<C>();
				checkIndex(index);

				const Constant* const constant = pool[index];

				if(instanceof<const C*>(constant))
					return static_cast<const C*>(constant);

				throw InvalidConstantPoolReferenceException("Invalid constant pool reference " + hexWithPrefix<4>(index) +
						": expected " + C::CONSTANT_NAME + ", got " + (constant == nullptr ? "null" : constant->getConstantName()));
						//": expected " + typenameof(C) + ", got " + (constant == nullptr ? "null" : typenameof(*constant)));
			}

		public:
			const Constant*& operator[] (uint16_t index) const {
				checkIndex(index);
				return pool[index];
			}


			template<class C>
			const C* getInter(uint16_t index) const {
				initInterConstant(index);

				return get0<C>(index);
			}

			inline const Utf8Constant& getInterUtf8Constant(uint16_t index) const {
				return *getInter<Utf8Constant>(index);
			}


			template<class C>
			const C* getNullable(uint16_t index) const {
				return index == 0 ? nullptr : get0<C>(index);
			}

			template<class C>
			inline const C* get(uint16_t index) const {
				return get0<C>(index);
			}

			template<class C>
			const C* getOrDefault(uint16_t index, function<const C*()> defaultValueGetter) const {
				if(index == 0)
					return defaultValueGetter();
				return get0<C>(index);
			}

			inline const Utf8Constant& getUtf8Constant(uint16_t index) const {
				return *get<Utf8Constant>(index);
			}
	};


	struct Utf8Constant: Constant, string {
		DEFINE_CONSTANT_NAME(Utf8);

		Utf8Constant(const char* str, size_t length): string(str, length) {}

		Utf8Constant(const char* str): string(str) {}
	};


	struct ConstValueConstant: Constant {
		DEFINE_CONSTANT_NAME(ConstantValue);

		virtual string toString(const ClassInfo& classinfo) const = 0;

		virtual const Operation* toOperation(const ConstantDecompilationContext) const = 0;
	};


	template<typename T>
	struct NumberConstant: ConstValueConstant {
		const T value;
		NumberConstant(const T value): value(value) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return primitiveToString(value);
		}
	};


	struct IntegerConstant: NumberConstant<int32_t> {
		DEFINE_CONSTANT_NAME(Integer);

		IntegerConstant(const int32_t value): NumberConstant(value) {};

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};

	struct FloatConstant: NumberConstant<float> {
		DEFINE_CONSTANT_NAME(Float);

		FloatConstant(const float value): NumberConstant(value) {};

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};

	struct LongConstant: NumberConstant<int64_t> {
		DEFINE_CONSTANT_NAME(Long);

		LongConstant(const int64_t value): NumberConstant(value) {};

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;

		virtual uint8_t getPositions() const override {
			return 2;
		}
	};

	struct DoubleConstant: NumberConstant<double> {
		DEFINE_CONSTANT_NAME(Double);

		DoubleConstant(const  double value): NumberConstant(value) {};

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;

		virtual uint8_t getPositions() const override {
			return 2;
		}
	};


	struct ClassConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(Class);

		const Utf8Constant& name;

		ClassConstant(const Utf8Constant& name): name(name) {}

		ClassConstant(const ConstantPool& constPool, uint16_t index): name(constPool.getInterUtf8Constant(index)) {}

		virtual string toString(const ClassInfo&) const override;

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};


	struct StringConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(String);

		const Utf8Constant& value;

		StringConstant(const Utf8Constant& value): value(value) {}

		StringConstant(const ConstantPool& constPool, uint16_t index): value(constPool.getInterUtf8Constant(index)) {}

		virtual string toString(const ClassInfo&) const override {
			return stringToLiteral(value);
		}

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};



	struct NameAndTypeConstant: Constant {
		DEFINE_CONSTANT_NAME(NameAndType);

		const Utf8Constant & name, & descriptor;

		NameAndTypeConstant(const ConstantPool& constPool, uint16_t nameIndex, uint16_t descriptorIndex):
				name(constPool.getInterUtf8Constant(nameIndex)), descriptor(constPool.getInterUtf8Constant(descriptorIndex)) {}
	};

	struct ReferenceConstant: Constant {
		DEFINE_CONSTANT_NAME(Reference);

		const ClassConstant* const clazz;
		const NameAndTypeConstant* const nameAndType;

		ReferenceConstant(const ConstantPool& constPool, uint16_t classIndex, uint16_t nameAndTypeIndex):
				clazz(constPool.getInter<ClassConstant>(classIndex)), nameAndType(constPool.getInter<NameAndTypeConstant>(nameAndTypeIndex)) {}
	};

	struct FieldrefConstant: ReferenceConstant {
		DEFINE_CONSTANT_NAME(Fieldref);

		FieldrefConstant(const ConstantPool& constPool, uint16_t classIndex, uint16_t nameAndTypeIndex):
				ReferenceConstant(constPool, classIndex, nameAndTypeIndex) {}
	};

	struct MethodrefConstant: ReferenceConstant {
		DEFINE_CONSTANT_NAME(Methodref);

		MethodrefConstant(const ConstantPool& constPool, uint16_t classIndex, uint16_t nameAndTypeIndex):
				ReferenceConstant(constPool, classIndex, nameAndTypeIndex) {}
	};

	struct InterfaceMethodrefConstant: MethodrefConstant {
		DEFINE_CONSTANT_NAME(InterfaceMethodref);

		InterfaceMethodrefConstant(const ConstantPool& constPool, uint16_t classIndex, uint16_t nameAndTypeIndex):
				MethodrefConstant(constPool, classIndex, nameAndTypeIndex) {}
	};


	struct MethodHandleConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(MethodHandle);

		public:
			enum class ReferenceKind {
				GETFIELD = 1, GETSTATIC, PUTFIELD, PUTSTATIC, INVOKEVIRTUAL, INVOKESTATIC, INVOKESPECIAL, NEWINVOKESPECIAL, INVOKEINTERFACE
			};

			enum class KindType { FIELD, METHOD };

			const ReferenceKind referenceKind;
			const KindType kindType;

			const ReferenceConstant* const referenceConstant;

		private:
			static KindType getKindType(const ReferenceKind referenceKind) {
				switch(referenceKind) {
					case ReferenceKind::GETFIELD: case ReferenceKind::GETSTATIC: case ReferenceKind::PUTFIELD: case ReferenceKind::PUTSTATIC:
						return KindType::FIELD;
					default: return KindType::METHOD;
				}
		}

		public:
			MethodHandleConstant(const ConstantPool& constPool, uint8_t referenceKind, uint16_t referenceIndex):
					referenceKind((ReferenceKind)referenceKind), kindType(getKindType((ReferenceKind)referenceKind)),
					referenceConstant(constPool.getInter<ReferenceConstant>(referenceIndex)) {

				if(referenceKind < 1 || referenceKind > 9)
					throw IllegalStateException("referenceKind is " + to_string(referenceKind) + ", must be in the range 1 to 9");
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				return "#MethodHandle#";
			}

			virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};


	struct MethodTypeConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(MethodType);

		const Utf8Constant& descriptor;

		MethodTypeConstant(const Utf8Constant& descriptor): descriptor(descriptor) {}

		MethodTypeConstant(const ConstantPool& constPool, uint16_t descriptorIndex):
				descriptor(constPool.getInterUtf8Constant(descriptorIndex)) {}

		virtual string toString(const ClassInfo&) const override;

		virtual const Operation* toOperation(const ConstantDecompilationContext) const override;
	};

	struct InvokeDynamicConstant: Constant {
		DEFINE_CONSTANT_NAME(InvokeDynamic);

		const uint16_t bootstrapMethodAttrIndex;

		const NameAndTypeConstant* const nameAndType;

		InvokeDynamicConstant(const ConstantPool& constPool, uint16_t bootstrapMethodAttrIndex, uint16_t nameAndTypeIndex):
				bootstrapMethodAttrIndex(bootstrapMethodAttrIndex), nameAndType(constPool.getInter<NameAndTypeConstant>(nameAndTypeIndex)) {}
	};



	template<typename Const, typename... Args>
	struct InterConstantImpl: InterConstant {
		static_assert(is_base_of<Constant, Const>(), "Const must be inherited from Constant");

		const tuple<Args...> args;

		InterConstantImpl(Args... args): args(args...) {}

		template<size_t... indexes>
		inline const Constant* createConstant0(const ConstantPool& constPool, const index_sequence<indexes...>&) const {
			return new Const(constPool, get<indexes>(args)...);
		}

		virtual const Constant* createConstant(const ConstantPool& constPool) const override {
			return createConstant0(constPool, index_sequence_for<Args...>());
		}
	};




	ConstantPool::ConstantPool(ClassInputStream& instream): size(instream.readUShort()),
			pool(new const Constant*[size]), interPool(new const InterConstant*[size]) {

		for(uint16_t i = 0; i < size; i++) {
			pool[i] = nullptr;
			interPool[i] = nullptr;
		}

		for(uint16_t i = 1; i < size; i++) {
			uint8_t tag = instream.readUByte();

			switch(tag) {
				case  1: {
					uint16_t length = instream.readUShort();
					const char* bytes = instream.readString(length);
					pool[i] = new Utf8Constant(bytes, length);
					delete[] bytes;
					break;
				}
				case  3:
					pool[i] = new IntegerConstant(instream.readInt());
					break;
				case  4:
					pool[i] = new FloatConstant(instream.readFloat());
					break;
				case  5:
					pool[i] = new LongConstant(instream.readLong());
					i++; // Long and Double constants have historically held two positions in the pool
					break;
				case  6:
					pool[i] = new DoubleConstant(instream.readDouble());
					i++;
					break;
				case  7:
					interPool[i] = new InterConstantImpl<ClassConstant, uint16_t>(instream.readUShort());
					break;
				case  8:
					interPool[i] = new InterConstantImpl<StringConstant, uint16_t>(instream.readUShort());
					break;
				case  9:
					interPool[i] = new InterConstantImpl<FieldrefConstant, uint16_t, uint16_t>(instream.readUShort(), instream.readUShort());
					break;
				case 10:
					interPool[i] = new InterConstantImpl<MethodrefConstant, uint16_t, uint16_t>(instream.readUShort(), instream.readUShort());
					break;
				case 11:
					interPool[i] = new InterConstantImpl<InterfaceMethodrefConstant, uint16_t, uint16_t>(instream.readUShort(), instream.readUShort());
					break;
				case 12:
					interPool[i] = new InterConstantImpl<NameAndTypeConstant, uint16_t, uint16_t>(instream.readUShort(), instream.readUShort());
					break;
				case 15:
					interPool[i] = new InterConstantImpl<MethodHandleConstant, uint16_t, uint16_t>(instream.readUByte(), instream.readUShort());
					break;
				case 16:
					interPool[i] = new InterConstantImpl<MethodTypeConstant, uint16_t>(instream.readUShort());
					break;
				case 18:
					interPool[i] = new InterConstantImpl<InvokeDynamicConstant, uint16_t, uint16_t>(instream.readUShort(), instream.readUShort());
					break;
				default:
					throw ClassFormatError("Illegal constant type " + hexWithPrefix<2>(tag) + " at index #" + to_string(i) +
							" at pos " + hexWithPrefix((uint32_t)instream.getPos()));
			}
		}

		for(uint16_t i = 1; i < size; i += pool[i]->getPositions())
			this->initInterConstant(i);
	}

}

#undef DEFINE_CONSTANT_NAME
#endif
