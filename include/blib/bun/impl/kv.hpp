#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <memory>
#include <unqlite.h>
#include <boost/iterator/iterator_facade.hpp>

namespace blib {
    namespace bun {
        namespace __private {
            using ByteVctorType = std::vector<std::uint8_t>;

            template<typename T = std::string>
            struct ToByte {
                static std::vector<std::uint8_t> to_byte(T const& val) {
                    std::vector<std::uint8_t> ret;
                    for (const auto v : val) {
                        ret.push_back(static_cast<std::uint8_t>(v));
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::uint16_t> {
                static std::vector<std::uint8_t> to_byte(std::uint16_t const val) {
                    using T = std::uint16_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t  i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::int16_t> {
                static std::vector<std::uint8_t> to_byte(std::int16_t const val) {
                    using T = std::int16_t;
                    static const std::size_t  size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::uint32_t> {
                static std::vector<std::uint8_t> to_byte(std::uint32_t const val) {
                    using T = std::uint32_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::int32_t> {
                static std::vector<std::uint8_t> to_byte(std::int32_t const val) {
                    using T = std::int32_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::uint64_t> {
                static std::vector<std::uint8_t> to_byte(std::uint64_t const val) {
                    using T = std::uint64_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::int64_t> {
                static std::vector<std::uint8_t> to_byte(std::int64_t const val) {
                    using T = std::int64_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::float_t> {
                static std::vector<std::uint8_t> to_byte(std::float_t const val) {
                    using T = std::float_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::double_t> {
                static std::vector<std::uint8_t> to_byte(std::double_t const val) {
                    using T = std::double_t;
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    std::vector<std::uint8_t> ret(size);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(T const val) {
                            v = val;
                        }
                    };
                    const V v = val;
                    for (std::size_t i = 0; i < size; ++i) {
                        ret[i] = v.c[i];
                    }
                    return ret;
                }
            };

            template<>
            struct ToByte<std::uint8_t> {
                using T = std::uint8_t;
                static ByteVctorType to_byte(T const val) {
                    ByteVctorType vec;
                    vec.push_back(val);
                    return vec;
                }
            };


            template<>
            struct ToByte<std::int8_t> {
                using T = std::int8_t;
                static ByteVctorType to_byte(T const val) {
                    ByteVctorType vec;
                    vec.push_back(static_cast<std::uint8_t>(val));
                    return vec;
                }
            };

            /// @class FromByte
            /// @brief A class to convert byte vectors to different data structures
            template<typename T = std::string>
            struct FromByte {
                static void from_byte(ByteVctorType const& vec, T& value) {
                    for (const auto v : vec) {
                        value.push_back(v);
                    }
                }
            };

            template<>
            struct FromByte<std::uint16_t> {
                using T = std::uint16_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::int16_t> {
                using T = std::int16_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val):v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::uint32_t> {
                using T = std::uint32_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) : v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::int32_t> {
                using T = std::int32_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::uint64_t> {
                using T = std::uint64_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::int64_t> {
                using T = std::int64_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::float_t> {
                using T = std::float_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::double_t> {
                using T = std::double_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    static const std::size_t size = sizeof(T) / sizeof(std::uint8_t);
                    union V {
                        std::uint8_t c[size];
                        T v;
                        V(ByteVctorType const& val) :v() {
                            for (std::size_t i = 0; i < size && i < val.size(); ++i) {
                                c[i] = val[i];
                            }
                        }
                    };

                    V v = vec;
                    value = v.v;
                }
            };

            template<>
            struct FromByte<std::uint8_t> {
                using T = std::uint8_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    if (vec.size()) {
                        value = vec[0];
                    }
                }
            };

            template<>
            struct FromByte<std::int8_t> {
                using T = std::int8_t;
                static void from_byte(ByteVctorType const& vec, T& value) {
                    if (vec.size()) {
                        value = static_cast<std::int8_t>(vec[0]);
                    }
                }
            };
        }

        using ByteVctorType = __private::ByteVctorType;

        struct DbKVStoreType {};
        struct DBKVStoreUnqlite : DbKVStoreType {};

        template<typename T>
        inline std::vector<std::uint8_t> to_byte_vec(T const& val) {
            return __private::ToByte<T>::to_byte(val);
        }

        template<typename T>
        inline void from_byte_vec(ByteVctorType const& vec, T& value) {
            __private::FromByte<T>::from_byte(vec, value);
        }

        template<typename T = DBKVStoreUnqlite>
        class KVIterator : public boost::iterator_facade<
            KVIterator<DBKVStoreUnqlite>,
            std::pair<ByteVctorType, ByteVctorType>,
            boost::forward_traversal_tag
        > {
        public:
            using ByteVecPair = std::pair<ByteVctorType, ByteVctorType>;

        private:
            std::unique_ptr<ByteVecPair> _val;
            unqlite* _db;
            unqlite_kv_cursor* _pcursor;

        public:
            KVIterator() noexcept :
                _val(std::make_unique<ByteVecPair>()),
                _db(nullptr),
                _pcursor(nullptr) {}

            KVIterator(unqlite* db) :
                _val(std::make_unique<ByteVecPair>()),
                _db(db),
                _pcursor(nullptr) {
                open();
            }

            KVIterator(KVIterator const& in_other) :
                _val(std::make_unique<ByteVecPair>()),
                _db(in_other._db),
                _pcursor(in_other._pcursor) {

            }

            ~KVIterator() {
                close();
            }

        private:
            friend class boost::iterator_core_access;

            void open() {
                if(_db != nullptr) {
                    if(_pcursor != nullptr) {
                        const auto rc = unqlite_kv_cursor_valid_entry(_pcursor);
                        if (rc != UNQLITE_OK) {
                            close();
                        }
                    }
                    else{
                        const auto rc = unqlite_kv_cursor_init(_db, &_pcursor);
                        if (rc != UNQLITE_OK) {
                            close();
                        }
                    }
                }

                if(_pcursor != nullptr) {
                    const auto rc = unqlite_kv_cursor_first_entry(_pcursor);
                    if (rc != UNQLITE_OK) {
                        close();
                    }
                }
            }

            void close() {
                unqlite_kv_cursor_release(_db, _pcursor);
                _pcursor = nullptr;
                _db = nullptr;
            }

            void increment() {
                const auto rc = unqlite_kv_cursor_next_entry(_pcursor);
                _val->first.clear();
                _val->second.clear();
                if (rc != UNQLITE_OK) {
                    close();
                }
            }

            bool equal(KVIterator const& other) const {
                return _pcursor == other._pcursor;
            }

            ByteVecPair& dereference() const {
                populate_key();
                populate_value();
                return *_val;
            }

            void populate_key() const {
                int buff_size = 0;
                const auto rc = unqlite_kv_cursor_key(_pcursor, nullptr, &buff_size);
                if (rc == UNQLITE_OK) {
                    std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(buff_size);
                    const auto rc = unqlite_kv_cursor_key(_pcursor, buffer.get(), &buff_size);
                    if (rc == UNQLITE_OK) {
                        _val->first.reserve(buff_size);
                        for (int i = 0; i < buff_size; ++i) {
                            _val->first.push_back(buffer[i]);
                        }
                    }
                }
            }

            void populate_value() const {
                unqlite_int64 buff_size = 0;
                const auto rc = unqlite_kv_cursor_data(_pcursor, nullptr, &buff_size);
                if (rc == UNQLITE_OK) {
                    std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(buff_size);
                    const auto rc = unqlite_kv_cursor_data(_pcursor, buffer.get(), &buff_size);
                    if (rc == UNQLITE_OK) {
                        _val->second.reserve(buff_size);
                        for (int i = 0; i < buff_size; ++i) {
                            _val->second.push_back(buffer[i]);
                        }
                    }
                }
            }
        };

        /// @class KVDb
        /// @brief The main class for the key value store
        template<typename T = DBKVStoreUnqlite>
        class KVDb {
        public:
            using ByteVctorType = std::vector<std::uint8_t>;

        private:
            /// @var _db
            /// @brief The default is of type unqlite
            unqlite* _db;
            /// @var _ok
            /// @brief The value is true if all things is right with the database else it is false
            bool _ok;

        public:
            /// @fn KVDb
            /// @param param
            /// @brief The constructor for the KV class
            KVDb(std::string const& param) :_db(nullptr), _ok(false) {
                const auto rc = unqlite_open(&_db, param.c_str(), UNQLITE_OPEN_CREATE);
                if ( UNQLITE_OK == rc) {
                    _ok = true;
                }
            }

            /// @fn KVDb
            /// @param other. The other KVDb from which we can copy values.
            /// @brief The copy constructor for the KV class
            KVDb(KVDb const& other) {
                _db = other._db;
                _ok = other._ok;
            }

            /// @fn ~KVDb
            /// @brief destructor for the KV class
            ~KVDb() {
                if (nullptr != _db) {
                    unqlite_close(_db);
                }
            }

            /// @fn ok
            /// @brief Returns Ok
            bool ok() const {
                return _ok;
            }

            std::string last_status() const {
                const char *zBuf = nullptr;
                int iLen = 0;
                unqlite_config(_db, UNQLITE_CONFIG_ERR_LOG, &zBuf, &iLen);
                const std::string ret(zBuf);
                return ret;
            }

            /// @fn put
            /// @param key The key
            /// @param value the value that needs to be stored
            /// @details Put stores the key and value and returns true of the store is done, else it returns false
            ///          All primary C++ data types including std::string is supported as key and value
            template<typename Key, typename Value>
            bool put(Key const& key, Value const& value) {
                const auto key_vec = to_byte_vec(key);
                const auto val_vec = to_byte_vec(value);
                const auto rc = unqlite_kv_store(_db, key_vec.data(), key_vec.size(), val_vec.data(), val_vec.size());
                const bool ret = rc == UNQLITE_OK ? true : false;
                return ret;
            }

            /// @fn get
            /// @param key The key
            /// @param value the value is of type ByteVctorType. This carries the out value
            /// @details Gets the value corresponding the key. If the retrieval it returns true else it returns false.
            ///          All primary C++ data types including std::string is supported as key. The value is a byte (std::uint8_t) value
            template<typename Key>
            bool get(Key const& key, ByteVctorType& value) {
                const auto key_vec = to_byte_vec(key);
                unqlite_int64 buff_size = 0;
                auto rc = unqlite_kv_fetch(_db, key_vec.data(), key_vec.size(), NULL, &buff_size);
                if ( UNQLITE_OK == rc) {
                    std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(buff_size);
                    rc = unqlite_kv_fetch(_db, key_vec.data(), key_vec.size(), buffer.get(), &buff_size);
                    value.reserve(buff_size);
                    for (int i = 0; i < buff_size; ++i) {
                        value.push_back(buffer[i]);
                    }
                }

                const bool ret = rc == UNQLITE_OK ? true : false;
                return ret;
            }

            /// @fn get
            /// @param key The key
            /// @param value the value is of type ByteVctorType. This carries the out value
            /// @details Gets the value corresponding the key. If the retrieval it returns true else it returns false.
            ///          All primary C++ data types including std::string is supported as key. The value C++ primary datatype.
            ///          This function is a wrapper on top of the previous function which returns the byte vector.
            template<typename Key, typename Value>
            bool get(Key const& key, Value& value) {
                bool ret = false;
                ByteVctorType vec;
                const bool rc = get(key, vec);
                if (true == rc) {
                    from_byte_vec(vec, value);
                    ret = true;
                }
                return ret;
            }

            /// @fn del
            /// @param key The key
            /// @details Delete the value corresponding to key. If delete is success then returns true else returns false.
            ///          All primary C++ data types including std::string is supported as key.
            template<typename Key>
            bool del(Key const& key) {
                const auto key_vec = to_byte_vec(key);
                const auto rc = unqlite_kv_delete(_db, key_vec.data(), key_vec.size());
                const bool ret = rc == UNQLITE_OK ? true : false;
                return ret;
            }

            KVIterator<> begin() {
                return KVIterator<>(_db);
            }

            KVIterator<> end() {
                return KVIterator<>();
            }
        };
    }
}

