#ifndef JDECOMPILER_STRINGIFIED_CPP
#define JDECOMPILER_STRINGIFIED_CPP

namespace jdecompiler {

	struct Stringified {
		public:
			virtual string toString(const ClassInfo&) const = 0;

			virtual ~Stringified() {}
	};
}

#endif
