#include <type_traits>
#include <string>
#include <map>
#include <cassert>

namespace argparse {
	using std::string;
	using std::map;
	using std::is_enum_v;

	enum class ArgsCount {
		ONE, ANY, ONE_AND_MORE
	};

	class Option {
		const char* const name;

		constexpr Option(const char* name) noexcept: name(name) {}

		virtual bool required() const {
			return false;
		}
	};

	class BoolOption: public Option {
		constexpr Option(const char* name) noexcept: name(name) {}

		virtual bool required() const override {

		}
	};

	class ArgParser {
		private:
			map<string, Option*> options;

		public:
			inline ArgParser() noexcept: options() {}

			inline ArgParser& boolOption(const char* name, ) {
				assert(name[0] == '-');

				options[name] = new BoolOption(name);
				return *this;
			}

			template<typename E>
			inline ArgParser& enumOption(const char* name) {
				static_assert(is_enum_v<E>);
				assert(name[0] == '-');

				options[name] = new EnumOption<E>(name);
				return *this;
			}
	};
}

int main() {
	using namespace argparse;

	ArgParser argparser();
}
