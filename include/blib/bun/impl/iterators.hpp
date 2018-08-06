#pragma once
#include <boost/iterator/iterator_facade.hpp>

namespace blib {
	namespace bun {
		namespace __private {
			template<typename ValueType, typename WhereType, typename RefType>
			class ObjectIterator :
				public boost::iterator_facade<
				ObjectIterator<ValueType, WhereType, RefType>,
				RefType<ValueType>,
				boost::forward_traversal_tag
				> {};
		}
	}
}
