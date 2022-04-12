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

			const ClassType &thisType, &superType;
			const ConstantPool& constPool;
			const Attributes& attributes;
			const uint16_t modifiers;

			ClassInfo(const Class& clazz, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool,
					const Attributes& attributes, uint16_t modifiers):
					clazz(clazz), thisType(thisType), superType(superType), constPool(constPool), attributes(attributes),
					modifiers(modifiers) {}

		private:
			mutable set<const ClassType*> imports;

			mutable uint16_t indentWidth = 0;
			mutable const char* indent = new char[1] {'\0'};

		public:
			bool addImport(const ClassType* clazz) const;

			string importsToString() const;

			/*inline void setFormatting(uint16_t indentWidth, const char* indent, const set<const ClassType*>& imports) const {
				this->indentWidth = indentWidth;
				this->indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth);
				this->imports = imports;
			}*/

			inline void copyFormattingFrom(const ClassInfo& other) const {
				indentWidth = other.indentWidth;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth);
				imports = other.imports;
			}

			inline void resetFormatting() const {
				indentWidth = 0;
				indent = new char[1] {'\0'};
				imports.clear();
			}

			inline const char* getIndent() const {
				return indent;
			}

			void increaseIndent() const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), ++indentWidth);
			}

			void increaseIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth += count);
			}

			void reduceIndent() const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), --indentWidth);
			}

			void reduceIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth -= count);
			}

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
