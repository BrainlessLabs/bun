namespace soci {
	;
	template <>
	struct type_conversion<backery::Bun> {
		typedef values base_type;
		using ClassType = backery::Bun;
		inline static void from_base(values const& v, indicator ind, ClassType& c)
		{
			;
			c.bun_name = v.get<blib::bun::__private::ConvertCPPTypeToSOCISupportType<decltype(c.bun_name)>::type>("bun_name");
			c.sugar_quantity = v.get<blib::bun::__private::ConvertCPPTypeToSOCISupportType<decltype(c.sugar_quantity)>::type>("sugar_quantity");
			c.bun_length = v.get<blib::bun::__private::ConvertCPPTypeToSOCISupportType<decltype(c.bun_length)>::type>("bun_length");
			;
		}
		inline static void to_base(const ClassType& c, values& v, indicator& ind)
		{
			;
			v.set("bun_name", blib::bun::__private::convertToSOCISupportedType(c.bun_name));
			v.set("sugar_quantity", blib::bun::__private::convertToSOCISupportedType(c.sugar_quantity));
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
				inline static void createSchema()
				{
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL"
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
					static std::string const sql = fmt::format(query, class_name, blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::bun_name)>::type>(), blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::sugar_quantity)>::type>(), blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::bun_length)>::type>());
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const& e) {
						l().error("createSchema: {} ", e.what());
					}
				}
				inline static void deleteSchema()
				{
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
				inline static SimpleOID persistObj(T* obj)
				{
					;
					static std::string const class_name = "backery::Bun";
					SimpleOID oid;
					oid.populateLow();
					static std::string const query = "INSERT INTO '{}' (oid_low"
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
					std::string const sql = fmt::format(query, class_name, oid.low, ":"
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
				inline static void updateObj(T* obj, SimpleOID const& oid)
				{
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query = "UPDATE '{}' SET "
						"{} = {}"
						","
						"{} = {}"
						","
						"{} = {}"
						" WHERE oid_low={} AND oid_high={}";
					std::string const sql = fmt::format(query, class_name, "bun_name", ":"
						"bun_name",
						"sugar_quantity", ":"
						"sugar_quantity",
						"bun_length", ":"
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
				inline static void deleteObj(SimpleOID const& oid)
				{
					;
					static std::string const class_name = "backery::Bun";
					static std::string const query = "DELETE FROM '{}' WHERE oid_high={} AND oid_low={}";
					std::string const sql = fmt::format(query, class_name, oid.high, oid.low);
					l().info(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const& e) {
						l().error("deleteObj: {} ", e.what());
					}
				}
				inline static std::unique_ptr<T> getObj(SimpleOID const& oid)
				{
					;
					static std::string const class_name = "backery::Bun";
					std::unique_ptr<T> obj = std::make_unique<T>();
					static std::string const query = "SELECT * FROM {} WHERE oid_high={} AND oid_low={}";
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
				inline static std::string objToJson(T* obj, SimpleOID const& oid)
				{
					;
					static std::string const class_name = "backery::Bun";
					static std::string const obj_json_str = std::string("class:{},oid_high: {},oid_low:{}") + std::string(","
						"bun_name"
						":{}"
						","
						"sugar_quantity"
						":{}"
						","
						"bun_length"
						":{}");
					std::string obj_json;
					try {
						obj_json = fmt::format(obj_json_str, class_name, oid.high, oid.low, blib::bun::tojson_string(obj->bun_name), blib::bun::tojson_string(obj->sugar_quantity), blib::bun::tojson_string(obj->bun_length));
					}
					catch (std::exception const& e) {
						l().error("objToJson: {} ", e.what());
					}
					obj_json = "{" + obj_json + "}";
					return std::move(obj_json);
				}
				inline static std::string objToString(T* obj, SimpleOID const& oid)
				{
					;
					return std::move(objToJson(obj, oid));
				}
				inline static std::string md5(T* obj, SimpleOID const& oid)
				{
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
				inline static std::vector<SimpleOID> getAllOids()
				{
					;
					static std::string const class_name = "backery::Bun";
					return std::vector<SimpleOID>();
				}
				inline static std::vector<PRef<T> > getAllObjects()
				{
					;
					static std::string const class_name = "backery::Bun";
					const std::vector<PRef<T> > ret = getAllObjWithQuery("");
					return std::move(ret);
				}
				inline static std::vector<PRef<T> > getAllObjWithQuery(std::string const& in_query)
				{
					;
					static std::string const class_name = "backery::Bun";
					const std::string query = "SELECT oid_high, oid_low"
						","
						"bun_name"
						","
						"sugar_quantity"
						","
						"bun_length"
						" FROM '{}' {}";
					const std::string where_clasue = in_query.empty() ? "" : "WHERE " + in_query;
					std::vector<PRef<T> > ret;
					try {
						const std::string sql = fmt::format(query, class_name, where_clasue);
						l().info(sql);
						soci::rowset<soci::row> rows = (blib::bun::__private::DbBackend<>::i().session().prepare << sql);
						for (soci::rowset<soci::row>::const_iterator row_itr = rows.begin(); row_itr != rows.end(); ++row_itr) {
							auto const& row = *row_itr;
							const blib::bun::SimpleOID oid(row.get<ConvertCPPTypeToSOCISupportType<blib::bun::SimpleOID::OidHighType>::type>(0), row.get<ConvertCPPTypeToSOCISupportType<blib::bun::SimpleOID::OidLowType>::type>(1));
							auto obj = std::make_unique<T>();
							obj->bun_name = row.get<ConvertCPPTypeToSOCISupportType<decltype(obj->bun_name)>::type>(0 + 2);
							obj->sugar_quantity = row.get<ConvertCPPTypeToSOCISupportType<decltype(obj->sugar_quantity)>::type>(1 + 2);
							obj->bun_length = row.get<ConvertCPPTypeToSOCISupportType<decltype(obj->bun_length)>::type>(2 + 2);
							;
							ret.emplace_back(oid, obj.release());
						}
					}
					catch (std::exception const& e) {
						l().error("getAllObjWithQuery: {} ", e.what());
					}
					return std::move(ret);
				}
			};
			;
		}
	}
};
namespace blib {
	namespace bun {
		template<> struct IsPersistant<backery::Bun)> : std::true_type {
	};
	}
};
namespace blib {
	namespace bun {
		namespace __private {
			template <>
			struct TypeMetaData<backery::Bun> {
				static std::string const& table_name()
				{
					static const std::string name = "backery::Bun";
					return name;
				}
				static std::map<std::string, std::string> const& type_maps()
				{
					static const std::map<std::string, std::string> type_maps = {
						{ "bun_name", blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::bun_name)>::type>() },{ "sugar_quantity", blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::sugar_quantity)>::type>() },{ "bun_length", blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(backery::Bun::bun_length)>::type>() },
					};
					return type_maps;
				}
			};
		}
	}
};;
