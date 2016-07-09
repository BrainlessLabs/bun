#pragma once

#include "blib/utils/Singleton.hpp"
#include <blib/hdr/sqlite_modern_cpp.hpp>

namespace blib {
  namespace bun {
    /////////////////////////////////////////////////
    /// @class Db
    /// @brief Database class. This is a singleton class.
    ///        This class is the backend that does the persist job.
    ///        This class should not be used directly.
    /////////////////////////////////////////////////
    struct Db : public ::blib::Singleton<Db> {
      using DbConnectionType = sqlite::database;
      std::unique_ptr<DbConnectionType> _db;
      bool _ok;
      std::string _fileName;

      bool ok() const {
        return _ok;
      }

      DbConnectionType& db() {
        return *_db;
      }

      bool connect( std::string const& aDbName ) {
        bool ret = true;
        try {
          _db.reset( new DbConnectionType( aDbName ) );
          _ok = true;
          _fileName = aDbName;
        }
        catch (...) {
          ret = false;
          _ok = false;
        }
        return ret;
      }
    };

    inline void dbConnect( std::string const& in_db ) {
      Db::i().connect( in_db );
    }
  }
}