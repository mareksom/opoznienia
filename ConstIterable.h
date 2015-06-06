#ifndef _CONST_ITERABLE_
#define _CONST_ITERABLE_

template<typename T>
class Iterator
{
public:
	typedef typename T::const_iterator const_iterator;

	Iterator(const_iterator it) : it(it) { }
	Iterator(const Iterator & it) : it(it.it) { }
	Iterator & operator ++ () { ++it; return *this; }
	Iterator operator ++ (int) { Iterator tmp(*this); operator++(); return tmp; }
	bool operator == (const Iterator & it) { return this->it == it.it; }
	bool operator != (const Iterator & it) { return this->it != it.it; }
	auto operator * () -> decltype(*(this->it)) { return *it; }
		
private:
	const_iterator it;
};

template<typename T>
class ConstIterable
{
public:

	ConstIterable(const T & t) : iterable(t) { }

	Iterator<T> begin() const
	{
		return Iterator<T>(iterable.begin());
	}

	Iterator<T> end() const
	{
		return Iterator<T>(iterable.end());
	}

private:
	const T & iterable;
};

#endif
