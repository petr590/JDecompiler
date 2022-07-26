#ifndef JDECOMPILER_ENUM_CLASS_H
#define JDECOMPILER_ENUM_CLASS_H

#include "class.h"

namespace jdecompiler {
	struct EnumClass final: Class {

		protected:
			struct EnumField: Field {
				const vector<const Operation*> arguments;

				EnumField(const Field&, const vector<const Operation*>&);

				virtual string toString(const StringifyContext&) const override;
			};


			struct EnumConstructorDescriptor: MethodDescriptor {
				const vector<const Type*> factualArguments;

				EnumConstructorDescriptor(const MethodDescriptor&);

				virtual string toString(const StringifyContext&, const Attributes&) const override;
			};

			vector<const EnumField*> enumFields;
			vector<const Field*> otherFields;

			static vector<MethodDataHolder> processMethodData(const vector<MethodDataHolder>&);

		public:
			EnumClass(const Version&, const ClassType&, const ClassType*, const ConstantPool&, uint16_t, const vector<const ClassType*>&,
					const Attributes&, const vector<FieldDataHolder>&, const vector<MethodDataHolder>&, const vector<const GenericParameter*>&);


		protected:
			virtual string bodyToString(const ClassInfo&) const override;

			virtual string fieldsToString(const ClassInfo&) const override;
	};
}

#endif
