#ifndef TOPLIST_H
#define TOPLIST_H

/* toplist<N, T, comp_fn = std::less<T>> - a list of the top N elements
 * Adding a new element to the list will remove the 'smallest'
 * element past N.
 * Removing elements is not supported, but the list can be cleared.
 * A const iterator is provided for accessing the list elements.
 *
 * A custom comparison function for elements can be provided.
 */
template<uint16_t N, typename T, typename comp_fn>
class toplist
{
  uint16_t nelems; // #elements in the list
  T        elems[N]; // the actual saved elements

public:
  toplist() : nelems(0)
  {
  };

  void clear()
  {
    nelems = 0;
  }

  typedef T* iterator;
  typedef const T* const_iterator;

  void add(T elem)
  {
    if (nelems < N) {
      // Just add the element
      elems[nelems++] = elem;
    } else {
      // Replace smallest element
      T smallest = elem;
      for (uint16_t i = 0; i < nelems; i++) {
        if (comp_fn()(elems[i], smallest)) {
          T temp = elems[i];
          elems[i] = smallest;
          smallest = temp;
        }
      }
    }
  }

  const_iterator begin() {
    return &elems[0];
  }

  const_iterator end() {
    return &elems[nelems];
  }
};

#endif
