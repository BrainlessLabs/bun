#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include <soci/soci.h>
#include <string>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/bun.hpp"

using namespace soci;
using namespace std;

namespace backery {
	struct Bun1 {
		std::string bun_name;
		float sugar_quantity;
		float flour_quantity;
		float milk_quantity;
		float yeast_quantity;
		float butter_quantity;
		int bun_length;
	};
}

namespace backery {
	struct Bun {
		std::string bun_name;
		float sugar_quantity;
		int bun_length;
	};
}

SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity, bun_length));

int main() {
	auto str = blib::bun::__private::SqlString<backery::Bun>::create_table_sql();
	str = blib::bun::__private::SqlString<backery::Bun>::insert_row_sql();
	return 1;
}
