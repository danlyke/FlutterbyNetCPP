#ifndef COCOAARRAY_H_INCLUDED
#define COCOAARRAY_H_INCLUDED
#include "fby_core_unix_propertyhelper.h"
#include <vector>
#include <boost/smart_ptr.hpp>
#include <algorithm>

namespace FbyHelpers
{

	template <typename C> class UnixArray;
	
	template <typename C> class UnixArrayHelper
	{
    private:
        FBYUNCOPYABLE(UnixArrayHelper);
	private:
		UnixArray<C> *helped;
		friend class UnixArray<C>;	
	public:
		int getCount() const;
	public:

		UnixArrayHelper(UnixArray<C> *helped) : helped(helped),
												  Count(this, &UnixArrayHelper<C>::getCount),
												  Length(this, &UnixArrayHelper<C>::getCount)
		{
#ifdef COCOAARRAYDEBUG
			printf("Created UnixArrayHelper %lx\n", (unsigned long)this);
			printf("   for UnixArray %lx\n", (unsigned long)helped);
#endif // ifdef COCOAARRAYDEBUG
		}

		~UnixArrayHelper()
		{
#ifdef COCOAARRAYDEBUG
			printf("Destroyed UnixArrayHelper %lx\n", (unsigned long)this);
			printf("   for UnixArray %lx\n", (unsigned long)helped);
#endif // ifdef COCOAARRAYDEBUG
			memset(this, 0, sizeof(*this));
		}

	public:
		PropertyHelper<UnixArrayHelper<C>,int> Count;
		PropertyHelper<UnixArrayHelper<C>,int> Length;
		void Add(C obj);
		bool Contains(C obj);
		void RemoveAt(int idx);
		void Clear();
		void RemoveRange(int idx, int count);
		void Remove(const C &obj);
		void AddRange(UnixArray<C> array);
		void AddRange(boost::intrusive_ptr<UnixArray<C> > array);
		
		void CopyTo(UnixArray<C> target, int index)
		{
			while (index < target.elems->size())
			{
				if (!(index < helped->elems->size()))
					break;
				C v = (*(helped->elems))[index];
				(*(target.elems))[index] = v;
				++index;
			}
		}

		void Reverse()
		{
			std::reverse( helped->elems->begin(), helped->elems->end() );
		}


		
	};
	
	class UnixArrayBase
	{
	public:
		UnixArrayBase()
		{
#ifdef COCOAARRAYDEBUG
			printf("Created UnixArrayBase %lx\n", (unsigned long)this);
#endif // ifdef COCOAARRAYDEBUG
		}
		virtual ~UnixArrayBase()
		{
#ifdef COCOAARRAYDEBUG
			printf("Destroyed UnixArrayBase %lx\n", (unsigned long)this);
#endif // ifdef COCOAARRAYDEBUG
			memset(this, 0, sizeof(*this));
		}
		virtual bool IsNull() const = 0;
	};

	inline bool IsNull(const UnixArrayBase &a)
	{
		return a.IsNull();
	}

	template <typename C> class UnixArray : public UnixArrayBase
	{
	private:
		UnixArrayHelper<C> helper;
		boost::shared_ptr<std::vector<C> > elems;
	public:
		UnixArray(int n = 0)
			: helper(this),
			elems(new std::vector<C>(n))
			{
#ifdef COCOAARRAYDEBUG
				printf("Created UnixArray %lx\n", (unsigned long)this);
#endif // ifdef COCOAARRAYDEBUG

			}

		UnixArray(const UnixArray<C> &rhs)
			: helper(this),
			elems(rhs.elems)
			{
#ifdef COCOAARRAYDEBUG
				printf("Created UnixArray %lx from %lx\n", (unsigned long)this, (unsigned long)&rhs);
#endif // ifdef COCOAARRAYDEBUG
			}

		virtual ~UnixArray()
		{
#ifdef COCOAARRAYDEBUG
			printf("Destroyed UnixArray %lx\n", (unsigned long)this);
#endif // ifdef COCOAARRAYDEBUG
			elems = boost::shared_ptr<std::vector<C> >();
		}

		UnixArray<C> & operator =(const UnixArray<C> &rhs)
		{
#ifdef COCOAARRAYDEBUG
			printf("Copied UnixArray %lx from %lx\n", (unsigned long)this, (unsigned long)&rhs);
#endif // ifdef COCOAARRAYDEBUG
			elems = rhs.elems;
			helper.helped = this;
			return *this;
		}


		friend class UnixArrayHelper<C>;

		C Get(int i)
		{
			return (*elems)[i];
		}
		
		void Set(int i, const C& v)
		{
			(*elems)[i] = v;
		}

		C & operator [](int i)
		{
			return (*elems)[i];
		}

		UnixArrayHelper<C> *operator->()
		{
			return &helper;
		}

		bool IsNull() const
		{
			return elems->size() == 0;
		}
	};
	
	template <typename C> bool ArrayIsNull(const UnixArray<C> &obj)
	{
		return obj.IsNull();
	}

	template <typename C> inline void UnixArrayHelper<C>::Add(C obj)
	{
		helped->elems->push_back(obj);
	}
	
	template <typename C> inline bool UnixArrayHelper<C>::Contains(C obj)
	{
		return std::find(helped->elems->begin(), helped->elems->end(), obj) != helped->elems->end();
	}
	
	template <typename C> inline int UnixArrayHelper<C>::getCount() const
	{
		return helped->elems->size();
	}
	
	template <typename C> inline void UnixArrayHelper<C>::RemoveAt(int idx)
	{
		helped->elems->erase(helped->elems->begin() + idx);
	}
	
	template <typename C> inline void UnixArrayHelper<C>::Clear()
	{
		helped->elems->clear();
	}
	
	template <typename C> inline void UnixArrayHelper<C>::RemoveRange(int idx, int count)
	{
		helped->elems->erase(helped->elems->begin() + idx, helped->elems->begin() + idx + count);
	}
	
	template <typename C> inline void UnixArrayHelper<C>::Remove(const C &obj)
{	
		helped->elems->erase(find(helped->elems->begin(), helped->elems->end(), obj));
	}
	
	template <typename C> inline void UnixArrayHelper<C>::AddRange(UnixArray<C> array)
	{
		for (int i = 0; i < array->Count; ++i)
		{
			Add(array[i]);
		}
	}
	
	template <typename C> inline void UnixArrayHelper<C>::AddRange(boost::intrusive_ptr<UnixArray<C> > array)
	{
		printf("AddRange boost\n");
		for (int i = 0; i < (*array)->Count; ++i)
		{
			printf("  Adding element %d: %d\n", i, (*array)[i]);
			Add((*array)[i]);
		}
	}
	
	
	
} // end of namespace FbyHelpers
#endif // #ifndef COCOAARRAY_H_INCLUDED



