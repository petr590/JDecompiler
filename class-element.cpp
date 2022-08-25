#ifndef JDECOMPILER_CLASS_ELEMENT_CPP
#define JDECOMPILER_CLASS_ELEMENT_CPP

#include "stringified.cpp"

namespace jdecompiler {

	struct ClassElement: Stringified {
		public:
			const modifiers_t modifiers;

			constexpr ClassElement(modifiers_t modifiers) noexcept: modifiers(modifiers) {}

			virtual bool canStringify(const ClassInfo&) const = 0;

			inline bool isSynthetic() const {
				return modifiers & ACC_SYNTHETIC;
			}
	};
}

#endif
