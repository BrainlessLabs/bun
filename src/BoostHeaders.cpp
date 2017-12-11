#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include "blib/utils/MD5.hpp"
#include <soci/soci.h>
#include <string>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/bun.hpp"

using namespace soci;
using namespace std;

namespace backery {
	struct Bun {
		std::string bun_name;
		float sugar_quantity;
		float flour_quantity;
		float milk_quantity;
		float yeast_quantity;
		float butter_quantity;
		int bun_length;
	};
}

SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity));

int main() {
	try {
		session sql(sqlite3, "database_filename");
		connection_parameters c;
		int count = 0;
		sql << "select * from sqlite_master";
	}
	catch (exception const & e) {
		cerr << "Error: " << e.what() << "\n";
	}

	auto t = BOOST_PP_TUPLE_ELEM(5, (1, 2, 3, 4, 5, 6));
	std::cout << "Tuple Out: " << t << std::endl;
	blib::bun::l().info("test");

	return 1;
}


namespace soci { "@brief --Specialization for SOCI ORM Start---"; 
template<>
struct type_conversion<backery::Bun> { typedef values base_type; using ClassType = backery::Bun; static void from_base(values const& v, indicator ind, ClassType& c) { "@brief from_base gets the values from db"; c.bun_name = v.get<decltype(c.bun_name)>("bun_name"); c.sugar_quantity = v.get<decltype(c.sugar_quantity)>("sugar_quantity");; }static void to_base(const ClassType& c, values& v, indicator& ind) { "@brief to_base puts the values to db"; v.set("bun_name", c.bun_name); v.set("sugar_quantity", c.sugar_quantity);; } }; }namespace blib { namespace bun { namespace __private { "@brief --Specialization for PRefHelper Start---"; template<>struct PRefHelper<backery::Bun> { using ClassType = backery::Bun; inline static void createSchema() { "@brief createSchema for creating the schema of an object"; static std::string const class_name = "backery::Bun"; static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL" "bun_name" " {}" "sugar_quantity" " {}" ")"; static std::string const sql = fmt::format(query, class_name, blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::bun_name)>(), blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::sugar_quantity)>()); l().info(sql); blib::bun::_private::DbBackend().i().session() << sql; }inline static void deleteSchema() { "@brief deleteSchema for deleting the schema of an object"; static std::string const class_name = "backery::Bun"; static std::string const sql = fmt::format("DROP TABLE '{}'", class_name); l().info(sql); blib::bun::_private::DbBackend().i().session() << sql; }inline static SimpleOID persistObj(T* obj) { "@brief persistObj for persisting the object"; static std::string const class_name = "backery::Bun"; SimpleOID oid; oid.populateLow(); static std::string const class_name = "backery::Bun"; static std::string const query = "INSERT INTO '{}' (oid_low" ","  "bun_name" ","  "sugar_quantity"") VALUES ({}" ","  "{}" ","  "{}"; std::string const sql = fmt::format(query, class_name, oid.low, obj->bun_name, obj->sugar_quantity); l().info(sql); return oid; }inline static void updateObj(T* obj, SimpleOID const& oid) { "@brief updateObj for updating a persisted object"; static std::string const class_name = "backery::Bun"; static std::string const query = "UPDATE {} SET " "{} = {}" ",""{} = {}" " WHERE oid_low={} AND oid_high={}"; std::string const sql = fmt::format(query, class_name, "bun_name", obj->bun_name, "sugar_quantity", obj->sugar_quantity, oid.low, oid.high); l().info(sql); }inline static void deleteObj(SimpleOID const& oid) { "@brief deleteObj for deleting a persisted object"; static std::string const class_name = "backery::Bun"; static std::string const query = "DELETE FROM {} WHERE oid_high={} AND oid_low={}"; std::string const sql = fmt::format(query, class_name, oid.high, oid.low); l().info(sql); }inline static std::unique_ptr<T> getObj(SimpleOID const& oid) { "@brief getObj for getting a persisted object with the oid"; static std::string const class_name = "backery::Bun"; std::unique_ptr<T> obj = std::make_unique<T>(); static std::string const query = "SELECT "EXPAND_QUERY_VARIABLES_getObj()" FROM {} WHERE oid_high={} AND oid_low={}"; std::string const sql = fmt::format(query, class_name, oid.high, oid.low); l().info(sql); return std::move(obj); } }; "@brief ---Specialization for PRefHelper End---"; "@brief ---===---"; "@brief --Specialization for QueryHelper Start---"; template<>struct QueryHelper<backery::Bun> { using ClassType = backery::Bun; inline static std::vector <SimpleOID> getAllOids() {} }; "@brief ---Specialization for QueryHelper End---"; } } };
