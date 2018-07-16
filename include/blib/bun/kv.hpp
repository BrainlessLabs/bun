#pragma once
#include <string>
#include <vector>
#include <unqlite.h>

namespace blib {
	namespace bun {
		namespace __private {
			template<typename T = std::string>
			struct ToByte {
				static std::vector<std::uint8_t> to_byte(T const& val) {
					std::vector<std::uint8_t> ret;
					for (const auto v : val) {
						ret.push_back(static_cast<std::uint8_t>(v));
					}
					return std::move(ret);
				}
			};

			template<>
			struct ToByte<std::uint16_t> {
				static std::vector<std::uint8_t> to_byte(std::uint16_t const val) {
					using T = std::uint16_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::int16_t> {
				static std::vector<std::uint8_t> to_byte(std::int16_t const val) {
					using T = std::int16_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::int16_t> {
				static std::vector<std::uint8_t> to_byte(std::int16_t const val) {
					using T = std::int16_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::uint32_t> {
				static std::vector<std::uint8_t> to_byte(std::uint32_t const val) {
					using T = std::uint32_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::int32_t> {
				static std::vector<std::uint8_t> to_byte(std::int32_t const val) {
					using T = std::int32_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::uint64_t> {
				static std::vector<std::uint8_t> to_byte(std::uint64_t const val) {
					using T = std::uint64_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::int64_t> {
				static std::vector<std::uint8_t> to_byte(std::int64_t const val) {
					using T = std::int64_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::float_t> {
				static std::vector<std::uint8_t> to_byte(std::float_t const val) {
					using T = std::float_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};

			template<>
			struct ToByte<std::double_t> {
				static std::vector<std::uint8_t> to_byte(std::double_t const val) {
					using T = std::double_t;
					static const int size = sizeof(T) / sizeof(std::uint8_t);
					std::vector<std::uint8_t> ret(size);
					union V {
						std::uint8_t c[size];
						T v;
						V(T const val) {
							v = val;
						}
					};
					const V v = val;
					for (int i = 0; i < size; ++i) {
						ret[i] = v.c[i];
					}
					return ret;
				}
			};
		}

		struct DbKVStoreType {};
		struct DBKVStoreUnqlite : DbKVStoreType {};

		template<typename T>
		std::vector<std::uint8_t> to_byte_vec(T const& val) {
			return __private::ToByte<T>::to_byte(val);
		}

		template<typename T = DBKVStoreUnqlite>
		class KVDb {
		private:
			unqlite* _db;
			bool _ok;

		public:
			KVDb(std::string const& param) :_db(nullptr), _ok(false) {
				const auto rc = unqlite_open(&_db, param.c_str(), UNQLITE_OPEN_CREATE);
				if ( UNQLITE_OK == rc) {
					_ok = true;
				}
			}

			KVDb(KVDb const& other) {
				_db = other._db;
				_ok = other._ok;
			}

			~KVDb() {
				if (nullptr != _db) {
					unqlite_close(_db);
				}
			}

			bool ok() const {
				return _ok;
			}

			std::string last_status() const {
				const char *zBuf = nullptr;
				int iLen = 0;
				unqlite_config(pDb, UNQLITE_CONFIG_ERR_LOG, &zBuf, &iLen);
				const std::string ret(zBuf);
				return ret;
			}

			template<typename Key, typename Value>
			bool put(Key const& key, Value const& value) {
				
				const auto key_vec = to_byte_vec(key);
				const auto val_vec = to_byte_vec(value);
				const auto rc = unqlite_kv_store(_db, key_vec.data(), key_vec.size(), val_vec.data(), val_vec.size());
				const bool ret = rc == UNQLITE_OK ? true : false;
				return ret;
			}

			template<typename Key>
			bool get(Key const& key, std::vector<std::uint8_t>& value) {
				const auto key_vec = to_byte_vec(key);
				size_t nBytes = 0;
				auto rc = unqlite_kv_fetch(_db, key_vec.data(), key_vec.size(), NULL, &nBytes);
				if ( UNQLITE_OK == rc) {
					char* zBuf = new char[nBytes];
					rc = unqlite_kv_fetch(_db, key_vec.data(), key_vec.size(), zBuf, nBytes);
					value.reserve(nBytes);
					for (int i = 0; i < nBytes; ++i) {
						value.push_back(zBuf[i]);
					}
					delete zBuf;
				}
				
				const bool ret = rc == UNQLITE_OK ? true : false;
				return ret;
			}

			template<typename T>
			bool del(Key const& key) {
				const auto key_vec = to_byte_vec(key);
			}
		};
	}
}
