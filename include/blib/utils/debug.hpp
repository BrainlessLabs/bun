#pragma once
namespace soci {
	;
	template <>
	struct type_conversion<backery::Bun> {
		typedef values base_type;
		using ClassType = backery::Bun;
		inline static void from_base(values const& v, indicator ind, ClassType& c) {
			;
			c.bun_name = v.get<decltype(c.bun_name)>("bun_name");
			c.sugar_quantity = v.get<decltype(c.sugar_quantity)>("sugar_quantity");
			c.bun_length = v.get<decltype(c.bun_length)>("bun_length");
			;
		}
		inline static void to_base(const ClassType& c, values& v, indicator& ind) {
			;
			v.set("bun_name", blib::bun::__private::convertToSOCISupportedType(c.bun_name));
			v.set("sugar_quantity",
				blib::bun::__private::convertToSOCISupportedType(c.sugar_quantity));
			v.set("bun_length", blib::bun::__private::convertToSOCISupportedType(c.bun_length));
			;
		}
	};
}
namespace blib {
	namespace bun {
		namespace __private {
			;
			template <>
			struct PRefHelper<backery::Bun> {
				using T = backery::Bun;
				inline static void createSchema() {
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query =
						"CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY "
						"AUTOINCREMENT, oid_low INTEGER NOT NULL"
						", "
						"bun_name"
						" {}"
						", "
						"sugar_quantity"
						" {}"
						", "
						"bun_length"
						" {}"
						")";
					static std::string const sql = fmt::format(
						query, class_name,
						blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::bun_name)>(),
						blib::bun::cppTypeToDbTypeString<decltype(
							backery::Bun::sugar_quantity)>(),
						blib::bun::cppTypeToDbTypeString<decltype(backery::Bun::bun_length)>());
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const& e) {
						l().error("createSchema: {} ", e.what());
					}
				}
				inline static void deleteSchema() {
					;
					static std::string const class_name = "backery::Bun";
					static std::string const sql = fmt::format("DROP TABLE '{}'", class_name);
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const& e) {
						l().error("deleteSchema: {} ", e.what());
					}
				}
				inline static SimpleOID persistObj(T* obj) {
					;
					static std::string const class_name = "backery::Bun";
					SimpleOID oid;
					oid.populateLow();
					static std::string const query =
						"INSERT INTO '{}' (oid_low"
						","
						"bun_name"
						","
						"sugar_quantity"
						","
						"bun_length"
						") VALUES ({}"
						","
						"{}"
						","
						"{}"
						","
						"{}"
						")";
					std::string const sql = fmt::format(query, class_name, oid.low,
						":"
						"bun_name",
						":"
						"sugar_quantity",
						":"
						"bun_length");
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, use(*obj);
					}
					catch (std::exception const& e) {
						l().error("persistObj: {} ", e.what());
					}
					return oid;
				}
				inline static void updateObj(T* obj, SimpleOID const& oid) {
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query =
						"UPDATE '{}' SET "
						"{} = {}"
						","
						"{} = {}"
						","
						"{} = {}"
						" WHERE oid_low={} AND oid_high={}";
					std::string const sql = fmt::format(query, class_name, "bun_name",
						":"
						"bun_name",
						"sugar_quantity",
						":"
						"sugar_quantity",
						"bun_length",
						":"
						"bun_length",
						oid.low, oid.high);
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, use(*obj);
					}
					catch (std::exception const& e) {
						l().error("updateObj: {} ", e.what());
					}
				}
				inline static void deleteObj(SimpleOID const& oid) {
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query =
						"DELETE FROM '{}' WHERE oid_high={} AND oid_low={}";
					std::string const sql = fmt::format(query, class_name, oid.high, oid.low);
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const& e) {
						l().error("deleteObj: {} ", e.what());
					}
				}
				inline static std::unique_ptr<T> getObj(SimpleOID const& oid) {
					;
					static std::string const class_name = "backery::Bun";
					std::unique_ptr<T> obj = std::make_unique<T>();
					static std::string const query =
						"SELECT * FROM {} WHERE oid_high={} AND oid_low={}";
					std::string const sql = fmt::format(query, class_name, oid.high, oid.low);
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, into(*obj);
					}
					catch (std::exception const& e) {
						l().error("getObj: {} ", e.what());
					}
					return std::move(obj);
				}
				inline static std::string objToJson(T* obj, SimpleOID const& oid) {
					;
					static std::string const class_name = "backery::Bun";
					static std::string const obj_json_str =
						std::string("class:{},oid_high: {},oid_low:{}") + std::string(
							","
							"bun_name"
							":{}"
							","
							"sugar_quantity"
							":{}"
							","
							"bun_length"
							":{}");
					l().info(obj_json_str);
					std::string obj_json;
					try {
						obj_json = fmt::format(obj_json_str, class_name, oid.high, oid.low,
							blib::bun::tojson_string(obj->bun_name),
							blib::bun::tojson_string(obj->sugar_quantity),
							blib::bun::tojson_string(obj->bun_length));
					}
					catch (std::exception const& e) {
						l().error("objToJson: {} ", e.what());
					}
					obj_json = "{" + obj_json + "}";
					return std::move(obj_json);
				}
				inline static std::string objToString(T* obj, SimpleOID const& oid) {
					;
					return std::move(objToJson(obj, oid));
				}
				inline static std::string md5(T* obj, SimpleOID const& oid) {
					;
					const std::string md5 = blib::md5(objToString(obj, oid));
					return std::move(md5);
				}
			};
			;
			;
			;
			template <>
			struct QueryHelper<backery::Bun> {
				using T = backery::Bun;
				inline static std::vector<SimpleOID> getAllOids() {
					;
					static std::string const class_name = "backery::Bun";
					return std::vector<SimpleOID>();
				}
				inline static std::vector<PRef<T>> getAllObjects() {
					;
					static std::string const class_name = "backery::Bun";
					return std::vector<PRef<T>>();
				}
			};
			;
		}
	}
};
