#ifndef JDECOMPILER_CLASS_HOLDER_CPP
#define JDECOMPILER_CLASS_HOLDER_CPP

namespace jdecompiler {

	struct ClassHolder {
		public:
			const string outputPath;
			const Class* clazz;

			ClassHolder(const string& outputPath, const Class* clazz):
					outputPath(outputPath), clazz(clazz) {}

			inline const Class* operator->() const {
				return clazz;
			}

			inline const Class& operator*() const {
				return *clazz;
			}
	};
}

#endif
