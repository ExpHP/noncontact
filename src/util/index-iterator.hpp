
// Adapted from https://forum.kde.org/viewtopic.php?f=74&t=94120#p191501
// Functions that work with iterators typically have as a precondition that the iterators form a valid
//  range, so I removed checks where the comparison operators compared the containers.
// (besides, they technically turned <= and friends into a partial order)

/// A generic random access iterator for a container that defines operator[].
template<typename Value_t, typename Container_t>
class index_iterator : public std::iterator<std::random_access_iterator_tag, Value_t>
{
protected:
   Container_t* container_;
   ptrdiff_t    index_;

public:

   typedef typename std::iterator<std::random_access_iterator_tag, Value_t> base_iterator;
   typedef typename base_iterator::reference         reference;
   typedef typename base_iterator::pointer           pointer;
   typedef typename base_iterator::difference_type   difference_type;
   typedef typename base_iterator::iterator_category iterator_category;
   typedef typename base_iterator::value_type        value_type;


   index_iterator() : container_(nullptr), index_(0) { }
   index_iterator(Container_t& container, ptrdiff_t index) : container_(&container), index_(index) { }

   bool operator==(const index_iterator& other) { return index_ == other.index_; }
   bool operator!=(const index_iterator& other) { return index_ != other.index_; }

   Value_t&       operator*()       { return (*container_)[index_]; }
   Value_t const& operator*() const { return (*container_)[index_]; }
   
   Value_t*       operator->()       { return &((*container_)[index_]); }
   Value_t const* operator->() const { return &((*container_)[index_]); }

   index_iterator& operator++() { ++index_; return *this;}
   index_iterator operator++(int) { index_iterator prev(*this); operator++(); return prev;}

   index_iterator& operator--() { --index_; return *this;}
   index_iterator operator--(int) { index_iterator prev(*this); operator--(); return prev;}
   
   friend index_iterator operator+(const index_iterator& a, difference_type b) { index_iterator ret(a); ret += b; return ret; }
   friend index_iterator operator-(const index_iterator& a, difference_type b) { index_iterator ret(a); ret -= b; return ret; }
   friend index_iterator operator+(difference_type a, const index_iterator& b) { index_iterator ret(b); ret += a; return ret; }
   friend index_iterator operator-(difference_type a, const index_iterator& b) { index_iterator ret(b); ret -= a; return ret; }

   difference_type operator-(const index_iterator& other) const { return index_ - other.index_; }

   bool operator< (const index_iterator& other) { return index_ <  other.index_; }
   bool operator<=(const index_iterator& other) { return index_ <= other.index_; }
   bool operator> (const index_iterator& other) { return index_ >  other.index_; }
   bool operator>=(const index_iterator& other) { return index_ >= other.index_; }

   index_iterator& operator+=(difference_type b) { index_ += b; }
   index_iterator& operator-=(difference_type b) { index_ -= b; }

   // XXX shouldn't this be (*container_)[i + index_] ?
   Value_t&       operator[](difference_type i)       { return (*container_)[i]; }
   Value_t const& operator[](difference_type i) const { return (*container_)[i]; }
};

template<typename Value_t, typename Container_t>
inline index_iterator<Value_t, Container_t> index_begin(Container_t& container)
{
   return index_iterator<Value_t, Container_t>(container, 0);
}

template<typename Value_t, typename Container_t>
inline index_iterator<Value_t, Container_t> index_end(Container_t& container)
{
   return index_iterator<Value_t, Container_t>(container, container.size());
}

template<typename Value_t, typename Container_t>
inline index_iterator<const Value_t, const Container_t> index_begin(const Container_t& container)
{
   return index_iterator<const Value_t, const Container_t>(container, 0);
}

template<typename Value_t, typename Container_t>
inline index_iterator<const Value_t, const Container_t> index_end(const Container_t& container)
{
   return index_iterator<const Value_t, const Container_t>(container, container.size());
}

