#pragma once

#include "blib/bun/SimpleOID.hpp"
#include "blib/utils/MD5.hpp"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <bitset>

namespace blib {
  namespace bun {
    /////////////////////////////////////////////////
    /// @class PRef
    /// @brief The persistent reference holder.
    ///        This is the primary object holder.
    /////////////////////////////////////////////////
    template<typename T>
    class PRef {
    private:
      enum class FlagsE : std::uint8_t {
        kDirty = 0
      };

      typedef SimpleOID OidType;
      std::unique_ptr<T> _obj;
      std::bitset<4> _flags;
      std::string _md5;

    public:
      typedef T ObjType;
      typedef PRef<T> SelfType;
      OidType oid;

    public:
      PRef() = default;
      PRef( PRef const& in_other ) : oid( in_other.oid ) {
        load( oid );
      }

      PRef( PRef& in_other ) : oid( in_other.oid ), _flags( in_other._flags ), _md5( in_other._md5 ), _obj( in_other._obj.release() ) {
      }

      PRef( ObjType* in_obj ) :_obj( in_obj ) {
      }

      PRef( OidType const& in_oid ) : oid( in_oid ) {
        load( oid );
      }

      PRef( OidType const& in_oid, ObjType* in_obj ) : oid( in_oid ), _obj( in_obj ) {
      }

      void reset( ObjType* in_obj ) {
        _obj.reset( in_obj );
        _flags.reset();
        _md5 = "";
      }

      ObjType* release() {
        oid.high = 0;
        oid.low = 0;
        _flags.reset();
        _md5 = "";
        return _obj.release();
      }

      ~PRef() = default;

      decltype(*_obj) operator*() {
        return *_obj;
      }

      T* operator->() {
        return _obj.get();
      }

      bool dirty() {
        const auto md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        if (md5 != _md5) {
          _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] = 1;
        }

        return _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] ? true : false;
      }

      OidType persist() {
        if (_md5.empty()) {
          oid = BunHelper<ObjType>::persistObj( _obj.get() );
        }
        else {
          BunHelper<ObjType>::updateObj( _obj.get(), oid );
        }
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
        return oid;
      }

      OidType save() {
        return persist();
      }

      void del() {
        BunHelper<ObjType>::deleteObj( oid );
        _md5.clear();
        oid.clear();
        _flags.reset();
      }

      PRef& operator=( ObjType* in_obj ) {
        reset( in_obj );
        return *this;
      }

      PRef& operator=( PRef& in_other ) {
        copyFrom( in_other );
        return *this;
      }

      bool operator==( PRef const& in_other ) {
        return oid == in_other.oid;
      }

      bool operator!=( PRef const& in_other ) {
        return oid != in_other.oid;
      }

      std::string toJson() const {
        return BunHelper<T>::objToJson( _obj.get(), oid );
      }

    private:
      void load( OidType const& in_oid ) {
        oid = in_oid;
        _obj = BunHelper<ObjType>::getObj( oid );
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
      }

      void copyFrom( PRef& in_other ) {
        oid = in_other.oid;
        _md5 = in_other._md5;
        _flags = in_other._flags;
        _obj = in_other._obj;
      }
    };
  }
}