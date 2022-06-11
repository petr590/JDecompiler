#ifndef JDECOMPILER_CLASSINFO_CPP
#define JDECOMPILER_CLASSINFO_CPP

namespace jdecompiler {

	struct Stringified {
		public:
			virtual string toString(const ClassInfo& classinfo) const = 0;

			virtual ~Stringified() {}
	};


	struct ClassElement: Stringified {
		public:
			virtual bool canStringify(const ClassInfo& classinfo) const = 0;
	};


	struct ClassInfo {
		public:
			const Class& clazz;

			const ClassType &thisType, *const superType;
			const vector<const ClassType*> interfaces;
			const ConstantPool& constPool;
			const Attributes& attributes;
			const uint16_t modifiers;

			ClassInfo(const Class& clazz, const ClassType& thisType, const ClassType* superType, const vector<const ClassType*>& interfaces,
					const ConstantPool& constPool, const Attributes& attributes, uint16_t modifiers):
					clazz(clazz), thisType(thisType), superType(superType), interfaces(interfaces),
					constPool(constPool), attributes(attributes), modifiers(modifiers) {}

			const char* const EMPTY_INDENT = "";

		private:
			mutable vector<const ClassType*> imports;

			mutable uint16_t indentWidth = 0;
			mutable const char* indent = EMPTY_INDENT;

		public:
			bool addImport(const ClassType* clazz) const;

			string importsToString() const;

			inline void copyFormattingFrom(const ClassInfo& other) const {
				indentWidth = other.indentWidth;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth);
				imports = other.imports;
			}

			inline void resetFormatting() const {
				indentWidth = 0;
				indent = EMPTY_INDENT;
				imports.clear();
			}

			inline const char* getIndent() const {
				return indent;
			}

			void increaseIndent() const {
				if(indent != EMPTY_INDENT)
					delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), ++indentWidth);
			}

			void increaseIndent(uint16_t count) const {
				if(indent != EMPTY_INDENT)
					delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth += count);
			}

			void reduceIndent() const {
				if(indent != EMPTY_INDENT)
					delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), --indentWidth);
			}

			void reduceIndent(uint16_t count) const {
				if(indent != EMPTY_INDENT)
					delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth -= count);
			}


			const vector<const Field*>& getFields() const;

			const vector<const Field*>& getConstants() const;

			const vector<const Method*>& getMethods() const;
			const vector<const Method*> getMethods(const function<bool(const Method*)>&) const;
			const Method* getMethod(const MethodDescriptor&) const;
			bool hasMethod(const MethodDescriptor&) const;


		private:
			mutable const StringifyContext* fieldStringifyContext = nullptr;
			mutable const DisassemblerContext* emptyDisassemblerContext = nullptr;

		public:
			const StringifyContext* getFieldStringifyContext() const;

			const DisassemblerContext& getEmptyDisassemblerContext() const;


			ClassInfo(const ClassInfo&) = delete;

			ClassInfo& operator=(const ClassInfo&) = delete;
	};
}

#endif
