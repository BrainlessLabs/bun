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

//SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity));

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


namespace soci {
	template<>
	struct type_conversion<backery::Bun> { 
		typedef values base_type; 
		using ClassType = backery::Bun;
		
		inline static void from_base(values const& v, indicator ind, ClassType& c) {
			c.bun_name = v.get<decltype(c.bun_name)>("bun_name"); 
			c.sugar_quantity = v.get<decltype(c.sugar_quantity)>("sugar_quantity");; 
			
		}
		
		inline static void to_base(const ClassType& c, values& v, indicator& ind) {
			v.set("bun_name", c.bun_name); 
			v.set("sugar_quantity", c.sugar_quantity);; 
		}
	};
}
namespace blib { namespace bun { namespace __private {
	template<>
	struct PRefHelper<backery::Bun> { 
		using T = backery::Bun;
		
		inline static void createSchema() {
			static std::string const class_name = "backery::Bun";
			static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL" "bun_name" " {}" "sugar_quantity" " {}" ")";
			static std::string const sql = fmt::format(query, class_name, blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::bun_name)>(), blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::sugar_quantity)>()); 
			l().info(sql);
			DbBackend<>::i().session() << sql; 
		}
		
		inline static void deleteSchema() {
			static std::string const class_name = "backery::Bun";
			static std::string const sql = fmt::format("DROP TABLE '{}'", class_name);
			l().info(sql);
			DbBackend<>::i().session() << sql;
		}
		
		inline static SimpleOID persistObj(T* obj) {
			static std::string const class_name = "backery::Bun";
			SimpleOID oid;
			oid.populateLow();
			static std::string const query = "INSERT INTO '{}' (oid_low" ","  "bun_name" ","  "sugar_quantity"") VALUES ({}" ","  "{}" ","  "{}";
			std::string const sql = fmt::format(query, class_name, oid.low, obj->bun_name, obj->sugar_quantity);
			l().info(sql);
			DbBackend<>::i().session() << sql, use(*obj);
			return oid;
		}
		
		inline static void updateObj(T* obj, SimpleOID const& oid) {
			static std::string const class_name = "backery::Bun";
			static std::string const query = "UPDATE {} SET " "{} = {}" ",""{} = {}" " WHERE oid_low={} AND oid_high={}";
			std::string const sql = fmt::format(query, class_name, "bun_name", obj->bun_name, "sugar_quantity", obj->sugar_quantity, oid.low, oid.high);
			l().info(sql);
			DbBackend<>::i().session() << sql, use(*obj);
		}
		
		inline static void deleteObj(SimpleOID const& oid) {
			static std::string const class_name = "backery::Bun";
			static std::string const query = "DELETE FROM {} WHERE oid_high={} AND oid_low={}";
			std::string const sql = fmt::format(query, class_name, oid.high, oid.low);
			l().info(sql);
			DbBackend<>::i().session() << sql;
		}
		
		inline static std::unique_ptr<T> getObj(SimpleOID const& oid) {
			static std::string const class_name = "backery::Bun";
			std::unique_ptr<T> obj = std::make_unique<T>();
			static std::string const query = "SELECT * FROM {} WHERE oid_high={} AND oid_low={}";
			std::string const sql = fmt::format(query, class_name, oid.high, oid.low);
			l().info(sql);
			DbBackend<>::i().session() << sql, into(*obj);
			return std::move(obj); 
		}
	};
	
	template<>
	struct QueryHelper<backery::Bun> {
		using T = backery::Bun;
		
		inline static std::vector<SimpleOID> getAllOids() {
			static std::string const class_name = "backery::Bun";
			return std::vector<SimpleOID>(); 
		}
		
		inline static std::vector<blib::bun::PRef<T>> getAllObjects() {
			static std::string const class_name = "backery::Bun";
			std::vector<blib::bun::PRef<T>> v;
			return v;
		}
	};
}
}
};
