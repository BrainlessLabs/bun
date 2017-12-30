#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file PRef.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief PRef is the reference to the persistence object. This handles all
///        the storage of the data.
/// @details This will be specialized for each class to store data.
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/SimpleOID.hpp"
//#include "blib/bun/BunHelper.hpp"
#include "blib/bun/PRefHelper.hpp"
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
        /// @details This is the primary object holder.
        ///          Anything assigned to this can be stored in the database.
        /////////////////////////////////////////////////
        template<typename T>
        class PRef {
        private:
            enum class FlagsE : std::uint8_t {
                kDirty = 0
            };

            /// @typedef OidType = SimpleOID
            typedef SimpleOID OidType;
            /// @var std::unique_ptr<T> _obj
            /// @brief Stores the object. The unique pointer is specialized
            ///        for that object type.
            std::unique_ptr<T> _obj;
            /// @var  std::bitset<4> _flags
            std::bitset<4> _flags;
            /// @var std::string _md5
            /// @brief Holds the MD5 sum of this object.
            std::string _md5;

        public:
            typedef T ObjType;
            typedef PRef<T> SelfType;
            /// @var OidType oid
            /// @brief Holds the OID for this object.
            ///        Each object will have an unique OID.
            ///        This will distinguish them from other object.
            OidType oid;

        public:
            PRef() = default;

            PRef(PRef const &in_other) : oid(in_other.oid) {
                load(oid);
            }

            PRef(PRef &in_other) : oid(in_other.oid), _flags(in_other._flags), _md5(in_other._md5),
                                   _obj(in_other._obj.release()) {
            }

            PRef(ObjType *in_obj) : _obj(in_obj) {
            }

            PRef(OidType const &in_oid) : oid(in_oid) {
                load(oid);
            }

            PRef(OidType const &in_oid, ObjType *in_obj) : oid(in_oid), _obj(in_obj) {
            }

            /// @fm reset
            /// @brief Resets the current PRef and assigns another object to it.
            /// @param in_obj The other object to assign it to.
            void reset(ObjType *in_obj) {
                _obj.reset(in_obj);
                _flags.reset();
                _md5 = "";
            }

            /// @fn release
            /// @brief Reset the PRef to hold nothing.
            ///        The oid is set to 0 and the object released.
            ObjType *release() {
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

            T *operator->() {
                return _obj.get();
            }

            /// @fn dirty
            /// @brief Returns true if the object is changed from last commit
            ///        else false.
            /// @details The MD5 of the object is taken from the last commit.
            ///          If the MD5 are different then it returns true else
            ///          it returns false.
            bool dirty() {
                const auto md5 = blib::bun::__private::PRefHelper<ObjType>::md5(_obj.get(), oid);
                if (md5 != _md5) {
                    _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] = 1;
                }

                return _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] ? true : false;
            }

            /// @fn persist
            /// @brief Commits the object in database
            /// @details Commits the object in database and also updates the
            ///          MD5 of the object. If this function is not called
            ///          the object is not going to be updated in database.
            /// @return OidType Returns the OID of the persisted object.
            OidType persist() {
                if (_md5.empty()) {
                    oid = blib::bun::__private::PRefHelper<ObjType>::persistObj(_obj.get());
                } else {
					blib::bun::__private::PRefHelper<ObjType>::updateObj(_obj.get(), oid);
                }
                _md5 = blib::bun::__private::PRefHelper<ObjType>::md5(_obj.get(), oid);
                _flags.reset();
                return oid;
            }

            /// @fn save
            /// @brief Does same as persist. Calls persist internally.
            OidType save() {
                return persist();
            }

            /// @fn del
            /// @brief Delets the persistent object.
            ///        Clears the MD5 and the flags.
            void del() {
				blib::bun::__private::PRefHelper<ObjType>::deleteObj(oid);
                _md5.clear();
                oid.clear();
                _flags.reset();
            }

            PRef &operator=(ObjType *in_obj) {
                reset(in_obj);
                return *this;
            }

            PRef &operator=(PRef &in_other) {
                copyFrom(in_other);
                return *this;
            }

            bool operator==(PRef const &in_other) {
                return oid == in_other.oid;
            }

            bool operator!=(PRef const &in_other) {
                return oid != in_other.oid;
            }

            /// @fn toJson
            /// @brief Returns a JSON representation of the object.
            std::string toJson() const {
                return blib::bun::__private::PRefHelper<T>::objToJson(_obj.get(), oid);
            }

        private:
            /// @fn oad(OidType const &in_oid)
            /// @brief Loads an object from Database.
            /// @param in_oid A valid OID for the object.
            void load(OidType const &in_oid) {
                oid = in_oid;
                _obj = blib::bun::__private::PRefHelper<ObjType>::getObj(oid);
                _md5 = blib::bun::__private::PRefHelper<ObjType>::md5(_obj.get(), oid);
                _flags.reset();
            }

            /// @fn copyFrom(PRef &in_other)
            /// @brief Create a exact clone of another PRef
            void copyFrom(PRef &in_other) {
                oid = in_other.oid;
                _md5 = in_other._md5;
                _flags = in_other._flags;
                _obj = in_other._obj;
            }
        };
    }
}
