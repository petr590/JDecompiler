#ifndef JDECOMPILER_VERSION_CPP
#define JDECOMPILER_VERSION_CPP

#include "version.h"

namespace jdecompiler {

	string to_string(const Version& version) {
		static const map<uint16_t, const string> versionTable {
				// I have not found an official indication of version JDK Beta, JDK 1.0 number, and I'm too lazy to check it
				{JDK_BETA, "JDK Beta"}, {JDK_1_0, "JDK 1.0"}, {JDK_1_1, "JDK 1.1"}, {JAVA_1_2, "Java 1.2"}, {JAVA_1_3, "Java 1.3"}, {JAVA_1_4, "Java 1.4"},
				{JAVA_5,   "Java 5"  }, {JAVA_6,  "Java 6" }, {JAVA_7,  "Java 7" }, {JAVA_8,   "Java 8"  }, {JAVA_9,   "Java 9"  }, {JAVA_10,  "Java 10"},
				{JAVA_11,  "Java 11" }, {JAVA_12, "Java 12"}, {JAVA_13, "Java 13"}, {JAVA_14,  "Java 14" }, {JAVA_15,  "Java 15" }, {JAVA_16,  "Java 16"},
				{JAVA_17,  "Java 17" }, {JAVA_18, "Java 18"}, {JAVA_19, "Java 19"}
		};

		return to_string(version.majorVersion) + '.' + to_string(version.minorVersion) +
				(has_key(versionTable, version.majorVersion) ? " (" + versionTable.at(version.majorVersion) + ')' : EMPTY_STRING);
	}
}

#endif
