#pragma once

/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
// Author: BrainlessLibraries

#include <memory>
#include <vector>
#include <queue>
#include <stack>
#include <functional>
#include <boost/iterator/iterator_facade.hpp>

namespace blib {
  namespace container {
    namespace tree {
      //=====================================================================
      // Node Iterators implementations
      namespace _private {
        // Left to right iterator for child nodes
        template<typename NodeType>
        class child_node_ltor_iterator :
          public boost::iterator_facade < child_node_ltor_iterator<NodeType>, NodeType, boost::forward_traversal_tag > {
        public:
          typedef NodeType Node;
          typedef Node& NodeRef;
          typedef Node const& ConstNodeRef;
          typedef child_node_ltor_iterator<NodeType> SelfType;

        private:
          typedef typename Node::ChildrenContainerType ChildrenContainerType;
          typedef typename ChildrenContainerType::iterator ItrType;

          friend class IteratorUtility;
        private:
          ItrType _it;
          ItrType _end;

        public:
          child_node_ltor_iterator( ) {
            _it = _end;
          }

          explicit child_node_ltor_iterator( ItrType& aBegin, ItrType& aEnd ) {
            _it = aBegin;
            _end = aEnd;
          }

        private:
          friend class boost::iterator_core_access;

          bool equal( SelfType const& aOther ) const {
            return aOther._it == _it;
          }

          NodeRef dereference( ) const {
            return *_it;
          }

          void increment( ) {
            ++_it;
          }

          ItrType itr( ) {
            return _it;
          }
        };

        // Right to left iterator for child nodes
        template<typename NodeType>
        class child_node_rtol_iterator :
          public boost::iterator_facade < child_node_rtol_iterator<NodeType>, NodeType, boost::forward_traversal_tag > {
        public:
          typedef NodeType Node;
          typedef Node& NodeRef;
          typedef Node const& ConstNodeRef;
          typedef child_node_rtol_iterator<NodeType> SelfType;

        private:
          typedef typename Node::ChildrenContainerType ChildrenContainerType;
          typedef typename ChildrenContainerType::reverse_iterator ItrType;

          friend class IteratorUtility;
        private:
          ItrType _it;
          ItrType _end;

        public:
          child_node_rtol_iterator( ) {
            _it = _end;
          }

          explicit child_node_rtol_iterator( ItrType& aBegin, ItrType& aEnd ) {
            _it = aBegin;
            _end = aEnd;
          }

        private:
          friend class boost::iterator_core_access;

          bool equal( SelfType const& aOther ) const {
            return aOther._it == _it;
          }

          NodeRef dereference( ) const {
            return *_it;
          }

          void increment( ) {
            ++_it;
          }

          ItrType itr( ) {
            return _it;
          }
        };

        class IteratorUtility {
        public:
          template<typename Iterator>
          static typename Iterator::ItrType itr( Iterator aItr ) {
            return aItr.itr( );
          }
        };


        //=====================================================================
        // Node Handle Implementation
        template<class NodeType>
        class NodeHandleImpl {
        public:
          typedef NodeType Node;
          typedef NodeHandleImpl<Node> SelfType;

        private:
          Node const* _handle;
          friend class NodeUtility;

        private:
          Node const* const pointer( ) const {
            return const_cast< Node const* const >( _handle );
          }

        public:
          NodeHandleImpl( Node const* const aPtr = nullptr ) :
            _handle( const_cast< Node const* >( aPtr ) ) {}

          NodeHandleImpl( SelfType const& aNode ) :
            _handle( aNode._handle ) {}

          NodeHandleImpl( Node const& aNode ) :
            _handle( aNode.handle( )._handle ) {}

          ~NodeHandleImpl( ) {
            _handle = nullptr;
          }

          bool operator==( SelfType const& aNode ) const {
            return aNode._handle == _handle;
          }

          bool operator==( Node const& aNode ) const {
            return aNode.handle( )._handle == _handle;
          }

          SelfType& operator=( SelfType const& aNode ) {
            _handle = aNode._handle;
            return *this;
          }

          operator bool( ) const {
            bool ret = false;
            if ( _handle ) {
              ret = true;
            }
            return ret;
          }
        };

        //=====================================================================
        // Node Utility
        class NodeUtility {
        public:
          template<typename NodeType>
          static NodeType const * const getNodeInternal( NodeHandleImpl<NodeType> const& aNodeHandle ) {
            return const_cast< NodeType const * const >( aNodeHandle._handle );
          }
        };

        //=====================================================================
        // Tree Iterators
        //=====================================================================
        //=====================================================================
        // PreOrder Tree Iterator
        template<typename NodeType>
        class pre_order_iterator :
          public boost::iterator_facade < pre_order_iterator<NodeType>, NodeType, boost::forward_traversal_tag > {
        private:
          typedef NodeType Node;
          typedef typename Node::ValueType ValueType;
          typedef typename Node::ValueRef ValueRef;
          typedef typename Node::ConstValueRef ConstValueRef;
          typedef typename Node::NodeRef NodeRef;
          typedef typename Node::ConstNodeRef ConstNodeRef;
          typedef typename Node::NodeHandle NodeHandle;
          typedef typename Node::NodeAllocator NodeAllocator;
          typedef typename Node::DataAllocator DataAllocator;
          typedef typename Node::child_node_ltor_iterator child_node_ltor_iterator;
          typedef std::reference_wrapper<Node> NodeRefWrapper;
          typedef std::stack<NodeRefWrapper, std::vector<NodeRefWrapper>> Stack;
          typedef pre_order_iterator<Node> SelfType;

        private:
          friend class boost::iterator_core_access;

          std::shared_ptr<Stack> _stack;
          std::shared_ptr<Node> _cur;

        public:
          pre_order_iterator( ) {}

          pre_order_iterator( NodeRef aRoot ) {
            _stack = std::make_shared<Stack>( );
            _cur = std::make_shared<Node>( aRoot );
            stack( ).push( aRoot );
          }

          pre_order_iterator( pre_order_iterator const& aOther ) {
            _stack = aOther._stack;
            _cur = aOther._cur;
          }

        private:
          NodeRef dereference( ) const {
            return cur( );
          }

          bool equal( SelfType const& aOther ) const {
            bool ret = false;

            if ( aOther._cur == _cur ) {
              ret = true;
            }

            //if ( !stack( ).empty( ) && ( stack( ).size( ) == aOther.stack( ).size( ) ) ) {
            //  if ( top( ) == aOther.top( ) ) {
            //    ret = true;
            //  }
            //}

            return ret;
          }

          //iterativePreorder( node )
          //  parentStack = empty stack
          //  while ( not parentStack.isEmpty( ) or node != null )
          //    if ( node != null )
          //      visit( node )
          //      if ( node.right != null ) parentStack.push( node.right )
          //        node = node.left
          //      else
          //      node = parentStack.pop( )
          void increment( ) {
            if ( stack( ).empty( ) ) {
              _cur.reset( );
              return;
            }

            // _cur already points to the top, so just pop it
            stack( ).pop( );
            // Right child is pushed before left child to make sure that left subtree is processed first.
            for ( auto it = cur( ).child_node_rtol_begin( );
                  it != cur( ).child_node_rtol_end( );
                  ++it ) {
              stack( ).push( *it );
            }

            // If the stack is not empty then only assign 
            if ( !stack( ).empty( ) ) {
              cur( top( ) );
            }
            else {
              _cur.reset( );
            }
          }

          NodeRef top( ) {
            return stack( ).top( );
          }

          Stack& stack( ) {
            return *_stack;
          }

          NodeRef cur( ) const {
            return *_cur;
          }

          void cur( ConstNodeRef aNode ) const {
            *_cur = aNode;
          }
        };// PreOrder Tree Iterator End


        //=====================================================================
        // PostOrder Tree Iterator 2Stacks
        //Traverse the left subtree by recursively calling the post - order function.
        //  Traverse the right subtree by recursively calling the post - order function.
        //  Display the data part of root element( or current element ).
        template<typename NodeType>
        class post_order_2stack_iterator :
          public boost::iterator_facade < post_order_2stack_iterator<NodeType>, NodeType, boost::forward_traversal_tag > {
        private:
          typedef NodeType Node;
          typedef typename Node::ValueType ValueType;
          typedef typename Node::ValueRef ValueRef;
          typedef typename Node::ConstValueRef ConstValueRef;
          typedef typename Node::NodeRef NodeRef;
          typedef typename Node::ConstNodeRef ConstNodeRef;
          typedef typename Node::NodeHandle NodeHandle;
          typedef typename Node::NodeAllocator NodeAllocator;
          typedef typename Node::DataAllocator DataAllocator;
          typedef typename Node::child_node_ltor_iterator child_node_ltor_iterator;
          typedef std::reference_wrapper<Node> NodeRefWrapper;
          typedef std::stack<NodeRefWrapper, std::vector<NodeRefWrapper>> Stack;
          typedef post_order_2stack_iterator<Node> SelfType;

        private:
          friend class boost::iterator_core_access;

          std::shared_ptr<Stack> _stack1;
          std::shared_ptr<Stack> _stack2;
          std::shared_ptr<Node> _cur;

        public:
          post_order_2stack_iterator( ) {}

          post_order_2stack_iterator( NodeRef aRoot ) {
            _stack1 = std::make_shared<Stack>( );
            _stack2 = std::make_shared<Stack>( );
            _cur = std::make_shared<Node>( aRoot );
            stack1( ).push( aRoot );
            createSecondStack( );
          }

          post_order_2stack_iterator( post_order_2stack_iterator const& aOther ) {
            _stack1 = aOther._stack1;
            _stack2 = aOther._stack2;
            _cur = aOther._cur;
          }

        private:
          NodeRef dereference( ) const {
            return cur( );
          }

          bool equal( SelfType const& aOther ) const {
            bool ret = false;
            if ( aOther._cur == _cur ) {
              ret = true;
            }
            return ret;
          }

          // Browse the second stack
          void increment( ) {
            if ( stack2( ).empty( ) ) {
              _cur.reset( );
            }
            else {
              cur( top( ) );
              stack2( ).pop( );
            }
          }

          // http://www.geeksforgeeks.org/iterative-postorder-traversal-using-stack/
          //1. Push root to first stack.
          //2. Loop while first stack is not empty
          //  2.1 Pop a node from first stack and push it to second stack
          //  2.2 Push left and right children of the popped node to first stack
          //3. Print contents of second stack
          void createSecondStack( ) {
            // Populate the second stack
            while ( !stack1( ).empty( ) ) {
              NodeRef node = stack1( ).top( );
              stack1( ).pop( );
              for ( auto& n : node ) {
                stack1( ).push( n );
              }
              stack2( ).push( node );
            }

            //Free stack1, we dont need it further
            _stack1.reset( );
            if ( !stack2( ).empty( ) ) {
              cur( top( ) );
              stack2( ).pop( );
            }
          }

          NodeRef top( ) {
            return stack2( ).top( );
          }

          Stack& stack1( ) {
            return *_stack1;
          }

          Stack& stack2( ) {
            return *_stack2;
          }

          NodeRef cur( ) const {
            return *_cur;
          }

          void cur( NodeRef aNode ) const {
            *_cur = aNode;
          }
        };
        // PostOrder Tree Iterator 2Stacks End

        //=====================================================================
        // LevelOrder Tree Iterator
        template<typename NodeType>
        class level_order_iterator :
          public boost::iterator_facade < level_order_iterator<NodeType>, NodeType, boost::forward_traversal_tag > {
        private:
          typedef NodeType Node;
          typedef typename Node::ValueType ValueType;
          typedef typename Node::ValueRef ValueRef;
          typedef typename Node::ConstValueRef ConstValueRef;
          typedef typename Node::NodeRef NodeRef;
          typedef typename Node::ConstNodeRef ConstNodeRef;
          typedef typename Node::NodeHandle NodeHandle;
          typedef typename Node::NodeAllocator NodeAllocator;
          typedef typename Node::DataAllocator DataAllocator;
          typedef typename Node::child_node_ltor_iterator child_node_ltor_iterator;
          typedef std::reference_wrapper<Node> NodeRefWrapper;
          typedef std::queue<NodeRefWrapper> Queue;
          typedef level_order_iterator<Node> SelfType;

        private:
          friend class boost::iterator_core_access;

          std::shared_ptr<Queue> _queue;
          std::shared_ptr<Node> _cur;

        public:
          level_order_iterator( ) {}

          level_order_iterator( NodeRef aRoot ) {
            _queue = std::make_shared<Queue>( );
            _cur = std::make_shared<Node>( aRoot );
            queue( ).push( aRoot );
          }

          level_order_iterator( level_order_iterator const& aOther ) {
            _queue = aOther._queue;
            _cur = aOther._cur;
          }

        private:
          NodeRef dereference( ) const {
            return cur( );
          }

          bool equal( SelfType const& aOther ) const {
            bool ret = false;
            if ( aOther._cur == _cur ) {
              ret = true;
            }
            return ret;
          }

          //1) Create an empty queue q
          //2) temp_node = root /*start from root*/
          //3) Loop while temp_node is not NULL
          //  a) print temp_node->data.
          //  b) Enqueue temp_node’s children( first left then right children ) to q
          //  c) Dequeue a node from q and assign it’s value to temp_node
          void increment( ) {
            if ( !_cur ) {
              return;
            }

            if ( queue( ).empty( ) ) {
              _cur.reset( );
            }
            else {
              // Pop it, _cur already contains it
              queue( ).pop( );
              for ( auto& n : cur( ) ) {
                queue( ).push( n );
              }
              // Push only if there is something in the queue
              if ( !queue( ).empty( ) ) {
                cur( queue( ).front( ) );
              }
              else {
                _cur.reset( );
              }
            }
          }

          Queue& queue( ) {
            return *_queue;
          }

          NodeRef cur( ) const {
            return *_cur;
          }

          void cur( ConstNodeRef aNode ) const {
            *_cur = aNode;
          }
        };// LevelOrder Tree Iterator End
      } // _private

      //=====================================================================
      // Node Handle
      template<class NodeType>
      class NodeHandle :
        public _private::NodeHandleImpl < NodeType > {
      private:
        typedef NodeHandleImpl<NodeType> BaseType;

      public:
        NodeHandle( Node const* const aPtr = nullptr ) :
          BaseType( aPtr ) {}

        NodeHandle( SelfType const& aNode ) :
          BaseType( aNode ) {}

        NodeHandle( Node const& aNode ) :
          BaseType( aNode ) {}
      };

      //=====================================================================
      // Tree Node
      //=====================================================================
      template<
        typename NodeDataType,
        typename DataAlloc = std::allocator<NodeDataType>,
        template<typename>class NodeAlloc = std::allocator>
      class Node {
      public:
        typedef NodeDataType ValueType;
        typedef ValueType& ValueRef;
        typedef ValueType const& ConstValueRef;
        typedef Node<NodeDataType, DataAlloc, NodeAlloc> NodeType;
        typedef NodeType SelfType;
        typedef NodeType& NodeRef;
        typedef NodeType const& ConstNodeRef;
        typedef _private::child_node_ltor_iterator<Node<NodeDataType>> child_node_ltor_iterator;
        typedef _private::child_node_rtol_iterator<Node<NodeDataType>> child_node_rtol_iterator;
        typedef NodeHandle<SelfType> NodeHandle;
        typedef NodeAlloc<SelfType> NodeAllocator;
        typedef DataAlloc DataAllocator;

      private:
        friend class child_node_ltor_iterator;
        friend class child_node_rtol_iterator;
        typedef std::vector<NodeType, NodeAllocator> ChildrenContainerType;

      private:
        std::shared_ptr<ValueType> _data;
        NodeHandle _parent;
        std::shared_ptr<ChildrenContainerType> _children;

      private:
        void allocateChildren( ) {
          _children = std::make_shared<ChildrenContainerType>( );
        }

        NodeRef assign( ConstNodeRef aOther ) {
          _parent = aOther._parent;
          _data = aOther._data;
          _children = aOther._children;
          return *this;
        }

        ChildrenContainerType& children( ) {
          return *_children;
        }
      public:
        Node( NodeHandle const& aParent = NodeHandle( ) ) :
          _parent( aParent ) {
          allocateChildren( );
        }

        Node( ConstNodeRef aOther ) {
          assign( aOther );
        }

        Node( ConstValueRef aData, NodeHandle const& aParent ) :
          _parent( aParent ) {
          allocateChildren( );
          _data = std::allocate_shared<ValueType>( DataAllocator( ), aData );
        }

        ~Node( ) {
          //clear( );
        }

        NodeHandle& parent( ) const {
          return _parent;
        }

        void parent( NodeHandle const& aParent ) {
          _parent = aParent;
        }

        NodeHandle handle( ) const {
          NodeHandle ret( this );
          return ret;
        }

        ValueRef data( ) {
          return *_data;;
        }

        ConstValueRef data( ) const {
          return *_data;;
        }

        void data( ConstValueRef aData ) {
          _data = std::allocate_shared<ValueType>( DataAllocator( ), aData );
        }

        // Access the children by index.
        ValueRef operator[]( const std::size_t aIndex ) {
          return children( ).at( aIndex );
        }

        std::size_t numberOfChildren( ) const {
          std::size_t ret = 0;
          if ( hasChildren( ) ) {
            ret = children( ).size( );
          }
          return ret;
        }

        bool hasChildren( ) const {
          bool ret = false;
          if ( _children ) {
            if ( !_children->empty( ) ) {
              ret = true;
            }
          }
          return ret;
        }

        void addChild( ConstNodeRef aNode ) {
          children( ).push_back( aNode );
          children( ).back( ).parent( handle( ) );
        }

        void addChild( ConstValueRef aValue ) {
          const NodeType n( aValue, handle( ) );
          children( ).push_back( n );
        }

        // The iterator pos must be valid and dereferenceable. 
        // Thus the end() iterator (which is valid, but is not dereferencable) cannot be used as a value for pos.
        void removeChild( child_node_ltor_iterator const& aItr ) {
          children( ).erase( _private::IteratorUtility::itr( aItr ) );
        }

        std::size_t size( ) const {
          return children( ).size( );
        }

        // Return true when there is no data and there is no children
        bool empty( ) const {
          return !_data && ( !_children || children( ).empty( ) );
        }

        // Return false when there is no data in the node. Else return true
        operator bool( ) const {
          bool ret = false;
          if ( _data ) {
            ret = true;
          }
          return ret;
        }

        void clear( ) {
          children( ).clear( );
          _parent = nullptr;
        }

        bool operator==( ConstNodeRef aOther ) const {
          bool ret = false;
          if ( aOther._parent == _parent ) {
            if ( aOther._data == _data ) {
              if ( aOther._children == _children ) {
                ret = true;
              }
            }
          }

          return ret;
        }

        NodeRef operator=( ConstNodeRef aOther ) {
          return assign( aOther );
        }

        bool isLeaf( ) const {
          return children( ).empty( );
        }

        // To support range based for loop
        // Default iteration is left to right
        child_node_ltor_iterator begin( ) {
          child_node_ltor_iterator it( children( ).begin( ), children( ).end( ) );
          return it;
        }

        child_node_ltor_iterator end( ) {
          child_node_ltor_iterator it( children( ).end( ), children( ).end( ) );
          return it;
        }

        child_node_ltor_iterator child_node_ltor_begin( ) {
          child_node_ltor_iterator it( children( ).begin( ), children( ).end( ) );
          return it;
        }

        child_node_ltor_iterator child_node_ltor_end( ) {
          child_node_ltor_iterator it( children( ).end( ), children( ).end( ) );
          return it;
        }

        child_node_rtol_iterator child_node_rtol_begin( ) {
          child_node_rtol_iterator it( children( ).rbegin( ), children( ).rend( ) );
          return it;
        }

        child_node_rtol_iterator child_node_rtol_end( ) {
          child_node_rtol_iterator it( children( ).rend( ), children( ).rend( ) );
          return it;
        }
      };
      // Tree Node End


      //=====================================================================
      // NTree Definition
      //=====================================================================
      template<typename NodeType>
      class NTree {
      public:
        typedef NodeType Node;
        typedef typename Node::ValueType ValueType;
        typedef typename Node::ValueRef ValueRef;
        typedef typename Node::ConstValueRef ConstValueRef;
        typedef typename Node::NodeRef NodeRef;
        typedef typename Node::ConstNodeRef ConstNodeRef;
        typedef typename Node::NodeHandle NodeHandle;
        typedef typename Node::NodeAllocator NodeAllocator;
        typedef typename Node::DataAllocator DataAllocator;
        typedef typename Node::child_node_ltor_iterator child_node_ltor_iterator;
        typedef typename Node::child_node_rtol_iterator child_node_rtol_iterator;
        typedef _private::pre_order_iterator<Node> pre_order_iterator;
        typedef _private::post_order_2stack_iterator<Node> post_order_iterator;
        typedef _private::level_order_iterator<Node> level_order_iterator;
        typedef NTree<Node> SelfType;
        typedef std::shared_ptr<Node> NodeSharedPtr;

      private:
        Node _root;

      public:
        NTree( ) {}

        NTree( ConstNodeRef aNode ) {
          root( aNode );
        }

        ~NTree( ) {
          //clear( );
        }

        void root( ConstValueRef aVal ) {
          _root.data( aVal );
        }

        void root( ConstNodeRef aNode ) {
          _root = aNode;
        }

        NodeRef root( ) {
          return _root;
        }

        ConstNodeRef root( ) const {
          return _root;
        }

        bool isRoot( NodeHandle const& aNodeHandle ) const {
          bool ret = false;
          if ( aNodeHandle == _root.handle( ) ) {
            ret = true;
          }
          return ret;
        }

        bool empty( ) const {
          return !_root;
        }

        operator bool( ) const {
          return empty( );
        }

        void clear( ) {
          if ( _root ) {
            _root->clear( );
          }
          _root.reset( );
        }

        SelfType& operator=( SelfType const& aOther ) {
          _root = aOther._root;
          return *this;
        }

        bool operator==( SelfType const& aOther ) const {
          return aOther._root == _root;
        }

        pre_order_iterator pre_order_begin( ) {
          pre_order_iterator ret( _root );
          return ret;
        }

        pre_order_iterator pre_order_end( ) {
          pre_order_iterator ret;
          return ret;
        }

        post_order_iterator post_order_begin( ) {
          post_order_iterator ret( _root );
          return ret;
        }

        post_order_iterator post_order_end( ) {
          post_order_iterator ret;
          return ret;
        }

        level_order_iterator level_order_begin( ) {
          level_order_iterator ret( _root );
          return ret;
        }

        level_order_iterator level_order_end( ) {
          level_order_iterator ret;
          return ret;
        }
      };
      //=====================================================================
      // NTree End
    }
  }
}