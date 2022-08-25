#ifndef JDECOMPILER_VARIABLE_H
#define JDECOMPILER_VARIABLE_H

namespace jdecompiler {

	struct Variable {
		protected:
			static string getRawNameByType(const Type*, bool* unchecked);

			static string getNameByType(const Type*);


		protected:
			mutable const Type* type;
			mutable bool declared;

			const bool isFixedType;

			mutable vector<const Operation*> bindedOperations;

			mutable bool typeSettingLocked = false;

			Variable(const Type*, bool declared, bool isFixedType);


		public:
			inline const Type* getType() const {
				return type;
			}

			template<bool widest = true>
			const Type* setType(const Type*) const;

			inline const Type* setTypeShrinking(const Type* newType) const {
				return setType<false>(newType);
			}

			inline const Type* castTypeTo(const Type* requiredType) const {
				return setType(type->castTo(requiredType));
			}

			inline void bind(const Operation* operation) const {
				bindedOperations.push_back(operation);
				setType(operation->getReturnTypeAsWidest(type));
			}


			inline bool isDeclared() const {
				return declared;
			}

			inline void setDeclared(bool declared) const {
				this->declared = declared;
			}

			virtual void makeCounter() const = 0;

			virtual bool isCounter() const = 0;


			virtual string getName() const = 0;

			virtual void addName(const string&) const = 0;

			inline bool operator==(const Variable& other) const {
				return this == &other;
			}


			virtual ~Variable() {}
	};


	struct NamedVariable: Variable {
		protected:
			const string name;

		public:
			NamedVariable(const Type*, bool declared, const string& name, bool isFixedType = false);

			virtual void makeCounter() const override;

			virtual bool isCounter() const override;

			virtual string getName() const override;

			virtual void addName(const string&) const override;
	};


	struct UnnamedVariable: Variable {
		protected:
			mutable uset<string> names;

			mutable bool counter = false;

		public:
			UnnamedVariable(const Type*, bool declared, bool isFixedType = false);

			UnnamedVariable(const Type*, bool declared, const string& name, bool isFixedType = false);

			UnnamedVariable(const UnnamedVariable&) = delete;

			virtual void makeCounter() const override;

			virtual bool isCounter() const override;

			virtual string getName() const override;

			virtual void addName(const string& name) const override;
	};
}

#endif
