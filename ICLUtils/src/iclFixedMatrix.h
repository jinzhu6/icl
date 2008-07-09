#ifndef ICL_FIXED_MATRIX_H
#define ICL_FIXED_MATRIX_H
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>

#include <iclException.h>
#include <iclDynMatrix.h>
#include <iclClippedCast.h>

namespace icl{

  /// FixedMatrix base struct defining datamode enum
  struct FixedMatrixBase{
    /// mode enum used for the FixedMatrix(T *data) constructor
    enum dataMode{
      deepcopy,     //!< given data pointer is copied deeply
      shallowcopy,  //!< given data pointer is used (ownership is not passed)
      takeownership //!< given data pointer is used (ownership is passed) 
    };

    /// Optimized copy function template (for N>30 using std::copy, otherwise a simple loop is used)
    template<class SrcIterator, class DstIterator, unsigned int N>
    static void optimized_copy(SrcIterator srcBegin, SrcIterator srcEnd, DstIterator dstBegin){
      if(N>30){
        std::copy(srcBegin,srcEnd,dstBegin);
      }else{
        while(srcBegin != srcEnd){
          //          *dstBegin++ = *srcBegin++;
          *dstBegin =
          *srcBegin;

          dstBegin++;
          srcBegin++;
        }
      }
    }
  };
    
  /** \cond */
  /// Forward Declaration fo FixedMatrixPart struct
  template<class T,unsigned int COLS,unsigned int ROWS> class FixedMatrix;
  /** \endcond */
  

  /// Utility struct for FixedMatrix sub-parts
  /** FixedMatrix member functions row and col return an instance of this
      utility structure. FixedMatrixPart instances wrap a range of Iterators
      (begin- and end-Iterator) of template parameter type. Once created, 
      FixedMatrixParts can not be setup with different Iterators. All further 
      Assignments will only copy the ranged data represented by the source
      and destination iterator pairs.\\
      To avoid problems with ranges of different size, FixedMatrixPart instances
      are 'templated' to the range size (template parameter N)
  **/
  template<class T,unsigned int N, class Iterator>
  class FixedMatrixPart : public FixedMatrixBase{

    public:
    /// Start iterator 
    Iterator begin;
    
    /// End iterator
    Iterator end;
  
    /// Creates a new FixedMatrixPart instance with given Iterator Pair
    FixedMatrixPart(Iterator begin, Iterator end):begin(begin),end(end){}
    
    /// Assignment with a new value (all data in range will be assigned with that value)
    FixedMatrixPart& operator=(const T &value){
      std::fill(begin,end,value);
      return *this;
    }
    /// Assignment with another (identical) instance of FixedMatrixPart
    /** Internally std::copy is used */
    FixedMatrixPart& operator=(const FixedMatrixPart &other){
      FixedMatrixBase::optimized_copy<Iterator,Iterator,N>(other.begin,other.end,begin);
      return *this;
    }
    
    /// Assignment with another (compatible) instance of FixedMatrixPart
    /** For compatibility. Iterator type and destination type may differ but
        RangeSize must be equal */
    template<class OtherIterator, class OtherT>
    FixedMatrixPart& operator=(const FixedMatrixPart<OtherT,N,OtherIterator> &other){
      std::transform(other.begin,other.end,begin,clipped_cast<OtherT,T>);
      return *this;
    }
    
    /// Assignment with a compatible FixedMatrix instance (FixedMatrix DIM must be euqal to Part-size)
    /** DIM equality is forced by Argument template parameters <...,COLS,N/COLS> */
    template<unsigned int COLS>
    FixedMatrixPart& operator=(const FixedMatrix<T,COLS,N/COLS> &m);

    /// Assignment with a FixedMatrix instance (FixedMatrix DIM must be euqal to Part-size)
    /** DIM equality is forced by Argument template parameters <...,COLS,N/COLS> */
    template<class T2, unsigned int COLS>
    FixedMatrixPart& operator=(const FixedMatrix<T2,COLS,N/COLS> &m);
    
  };

  /// Powerful and highly flexible Matrix class implementation
  /** By using fixed template parameters as Matrix dimensions,
      specializations to e.g. row or column vectors, are also
      as performant as possible.
  */
  template<class T,unsigned int COLS,unsigned int ROWS>
  class FixedMatrix : public FixedMatrixBase{
    public:

    /// returning a reference to a null matrix
    static const FixedMatrix &null(){
      static FixedMatrix null_matrix(T(0));
      return null_matrix;
    }

    /// count of matrix elements (COLS x ROWS)
    static const unsigned int DIM = COLS*ROWS;
    
    private:
    
    /// internal data storage
    T *m_data;
    
    /// flag to indicate whether current data pointer is owned and must be freed in the desturctor
    bool m_ownData;

    public:
    
    /// Default constructor 
    /** New data is created internally but elements are not initialized */
    FixedMatrix():m_data(new T[DIM]),m_ownData(true){}
    
    /// Create Matrix and initialize elements with given value
    FixedMatrix(const T &initValue):m_data(new T[DIM]),m_ownData(true){
      std::fill(begin(),end(),initValue);
    }

    /// Create matrix with given data pointer
    /** @param srcData source data pointer to use
        @param mode specifies what to do with given data pointer
                    @see FixedMatrixBase::dataMode
    */
    FixedMatrix(T *srcdata, dataMode mode):m_ownData(true){
      switch(mode){
        case deepcopy:
          m_data = new T[DIM];
          FixedMatrixBase::optimized_copy<T*,T*,DIM>(srcdata,srcdata+dim(),begin());
          //std::copy(srcdata,srcdata+dim(),begin());
          break;
        case shallowcopy:
          m_data = srcdata;
          m_ownData = false;
          break;
        case takeownership:
          m_data = srcdata;
          break;
      }
    }
    
    /// Create matrix with given data pointer (const version)
    /** As given data pointer is const, no shallow pointer copies are
        allowed here 
        @params srcdata const source data pointer copied deeply
    */
    FixedMatrix(const T *srcdata):m_data(new T[DIM]),m_ownData(true){
      FixedMatrixBase::optimized_copy<T*,T*,DIM>(srcdata,srcdata+dim(),begin());
      //std::copy(srcdata,srcdata+dim(),begin());
    }
    
    /** \cond */
#define A1(N) const T& v ## N
#define A2(N1,N2) A1(N1),A1(N2)
#define A3(N1,N2,N3) A1(N1),A2(N2,N3)
#define A4(N1,N2,N3,N4) A2(N1,N2),A2(N3,N4)

#define B1(N) if(COLS*ROWS>N) m_data[N] = v ## N
#define B2(N1,N2) B1(N1);B1(N2)
#define B3(N1,N2,N3) B1(N1);B2(N2,N3)
#define B4(N1,N2,N3,N4) B2(N1,N2);B2(N3,N4)

#define FM_INIT m_data(new T[DIM]),m_ownData(true)

    FixedMatrix(A2(0,1)):FM_INIT{ B2(0,1); }
    FixedMatrix(A3(0,1,2)):FM_INIT{ B3(0,1,2); }
    FixedMatrix(A4(0,1,2,3)):FM_INIT{ B4(0,1,2,3); }

    FixedMatrix(A4(0,1,2,3),A1(4)):FM_INIT{ B4(0,1,2,3);B1(4); }
    FixedMatrix(A4(0,1,2,3),A2(4,5)):FM_INIT{ B4(0,1,2,3);B2(4,5); }    
    FixedMatrix(A4(0,1,2,3),A3(4,5,6)):FM_INIT{ B4(0,1,2,3);B3(4,5,6); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7); }

    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A1(8)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B1(8); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A2(8,9)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B2(8,9); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A3(8,9,10)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B3(8,9,10); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A4(8,9,10,11)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B4(8,9,10,11); }

    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A4(8,9,10,11),A1(12)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B4(8,9,10,11);B1(12); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A4(8,9,10,11),A2(12,13)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B4(8,9,10,11);B2(12,13); }
    FixedMatrix(A4(0,1,2,3),A4(4,5,6,7),A4(8,9,10,11),A3(12,13,14)):FM_INIT{ B4(0,1,2,3);B4(4,5,6,7);B4(8,9,10,11);B3(12,13,14); }

    /** \endcond */

    /// Create matrix with given initializer elements (16 values max)
    /** Although there are no default parameters, this function can be called with any count of arguments (max. 16).
        This is achieved by heavy efficiently by C-Macro based overloading of the FixedMatrix constructor. To keep
        FixedMatrix documentation clean of all overloaded constructor versions, they have been excluded from 
        documentation explicitly. */
    
    FixedMatrix(const T& v0,const T& v1,const T& v2,const T& v3,
                const T& v4,const T& v5,const T& v6,const T& v7,  
                const T& v8,const T& v9,const T& v10,const T& v11,  
                const T& v12,const T& v13,const T& v14,const T& v15):FM_INIT{
      B4(0,1,2,3);B4(4,5,6,7);B4(8,9,10,11);B4(12,13,14,15);
    } 
    /** \cond */
#undef A1
#undef A2
#undef A3
#undef A4
#undef B1
#undef B2
#undef B3
#undef B4
#undef MF_INIT
    /** \endcond */

    // Range based constructor 
    //   template<class OtherT, class OtherIterator>
    //FixedMatrix(const RangedPart<OtherIterator> &r):m_data(new T[DIM]),m_ownData(true){
    //  std::copy(r.begin(),r.end(),begin());
    //} 
   
    /// Range based constructor for STL compatiblitiy 
    /** Range size must be compatible to the new matrix's dimension */
    template<class OtherIterator>
    FixedMatrix(OtherIterator begin, OtherIterator end):m_data(new T[DIM]),m_ownData(true){
      FixedMatrixBase::optimized_copy<OtherIterator,T*,DIM>(begin,end,begin());
      //      std::copy(begin,end,begin());
    }

    // Explicit Copy template based constructor (deep copy)
    FixedMatrix(const FixedMatrix &other):m_data(new T[DIM]),m_ownData(true){
      FixedMatrixBase::optimized_copy<const T*,T*,DIM>(other.begin(),other.end(),begin());
      //std::copy(other.begin(),other.end(),begin());
    }

    // Explicit Copy template based constructor (deep copy)
    template<class otherT>
    FixedMatrix(const FixedMatrix<otherT,COLS,ROWS> &other):m_data(new T[DIM]),m_ownData(true){
      std::transform(other.begin(),other.end(),begin(),clipped_cast<otherT,T>);
    }
    
    /// Assignment operator (with compatible data type) (deep copy)
    FixedMatrix &operator=(const FixedMatrix &other){
      if(this == &other) return *this;
      FixedMatrixBase::optimized_copy<const T*,T*,DIM>(other.begin(),other.end(),begin());
      //std::copy(other.begin(),other.end(),begin());
      return *this;
    }
    
    /// Assignment operator (with compatible data type) (deep copy)
    /** Internally using std::transform with icl::clipped_cast<otherT,T> */
    template<class otherT>
    FixedMatrix &operator=(const FixedMatrix<otherT,COLS,ROWS> &other){
      if(this == &other) return *this;
      std::transform(other.begin(),other.end(),begin(),clipped_cast<otherT,T>);
      return *this;
    }
    
    /// Assign all elements with given value
    FixedMatrix &operator=(const T &t){
      std::fill(begin(),end(),t);
      return *this;
    }

    /// Assign matrix elements with sup-part of another matrix (identical types)
    template<class Iterator>
    FixedMatrix &operator=(const FixedMatrixPart<T,DIM,Iterator> &r){ 
      //      std::copy(r.begin,r.end,begin());
      FixedMatrixBase::optimized_copy<Iterator,T*,DIM>(r.begin,r.end,begin());
      return *this;
    }

    /// Assign matrix elements with sup-part of another matrix (compatible types)
    template<class otherT, class Iterator>
    FixedMatrix &operator=(const FixedMatrixPart<otherT,DIM,Iterator> &r){ 
      std::transform(r.begin,r.end,begin(),clipped_cast<otherT,T>);
      return *this;
    }

    /// Matrix devision
    /** A/B is equal to A*inv(B)
        Only allowed form square matrices B
        @param m denominator for division expression
    */
    FixedMatrix operator/(const FixedMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return this->operator*(m.inv());
    }

    /// Matrix devision (inplace)
    FixedMatrix &operator/=(const FixedMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return *this = this->operator*(m.inv());
    }

    /// Multiply all elements by a scalar
    FixedMatrix operator*(T f) const{
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::multiplies<T>(),f));
      return d;
    }

    /// moved outside the class  Multiply all elements by a scalar (inplace)
    FixedMatrix &operator*=(T f){
      std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<T>(),f));
      return *this;
    }

    /// Divide all elements by a scalar
    FixedMatrix operator/(T f) const{
      return this->operator*(1/f);
    }

    /// Divide all elements by a scalar
    FixedMatrix &operator/=(T f){
      return this->operator*=(1/f);
    }

    /// Add a scalar to each element
    FixedMatrix operator+(const T &t){
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::plus<T>(),t));
      return d;
    }

    /// Add a scalar to each element (inplace)
    FixedMatrix &operator+=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::plus<T>(),t));
      return *this;
    }


    /// Substract a scalar from each element
    FixedMatrix operator-(const T &t){
      FixedMatrix d;
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::minus<T>(),t));
      return d;
    }

    /// Substract a scalar from each element (inplace)
    FixedMatrix &operator-=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::minus<T>(),t));
      return *this;
    }

    /// Element-wise matrix addition
    FixedMatrix operator+(const FixedMatrix &m){
      FixedMatrix d;
      std::transform(begin(),end(),m.begin(),d.begin(),std::plus<T>());
      return d;
    }

    /// Element-wise matrix addition (inplace)
    FixedMatrix &operator+=(const FixedMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::plus<T>());
      return *this;
    }

    /// Element-wise matrix subtraction
    FixedMatrix operator-(const FixedMatrix &m){
      FixedMatrix d;
      std::transform(begin(),end(),m.begin(),d.begin(),std::minus<T>());
      return d;
    }
    /// Element-wise matrix subtraction (inplace)
    FixedMatrix &operator-=(const FixedMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::minus<T>());
      return *this;
    }

    /// Prefix - operator 
    /** M + (-M) = 0;
    */
    FixedMatrix operator-() const {
      FixedMatrix cpy(*this);
      std::transform(cpy.begin(),cpy.end(),cpy.begin(),std::negate<T>());
      return cpy;
    }


    /// Element access operator
    T &operator()(unsigned int col,unsigned int row){
      return m_data[col+cols()*row];
    }

    /// Element access operator (const)
    const T &operator() (unsigned int col,unsigned int row) const{
      return m_data[col+cols()*row];
    }

    /// Element access index save (with exception if index is invalid)
    T &at(unsigned int col,unsigned int row) throw (InvalidIndexException){
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }

    /// Element access index save (with exception if index is invalid) (const)
    const T &at(unsigned int col,unsigned int row) const throw (InvalidIndexException){
      return const_cast<FixedMatrix*>(this)->at(col,row);
    }
    
    /// linear data view element access
    /** This function is very useful e.g. for derived classes
        FixedRowVector and FixedColVector */
    T &operator[](unsigned int idx){
      return m_data[idx];
    }
    /// linear data view element access (const)
    /** This function is very useful e.g. for derived classes
        FixedRowVector and FixedColVector */
    const T &operator[](unsigned int idx) const{
      return m_data[idx];
    }
    

    /// iterator type
    typedef T* iterator;
    
    /// const iterator type
    typedef const T* const_iterator;

    /// row_iterator
    typedef T* row_iterator;

    /// const row_iterator
    typedef const T* const_row_iterator;
  
    /// compatibility-function returns template parameter ROWS
    static unsigned int rows(){ return ROWS; }
  
    /// compatibility-function returns template parameter COLS
    static unsigned int cols(){ return COLS; }

    /// return internal data pointer
    T *data() { return m_data; }

    /// return internal data pointer (const)
    const T *data() const { return m_data; }

    /// return static member variable DIM (COLS*ROWS)
    static unsigned int dim() { return DIM; }
  
    /// internal struct for row-wise iteration with stride=COLS
    struct col_iterator : public std::iterator<std::random_access_iterator_tag,T>{

      /// just for compatibility with STL
      typedef unsigned int difference_type;
      
      /// wrapped data pointer (held shallowly)
      mutable T *p;
      
      /// the stride is equal to parent Matrix' classes COLS template parameter
      static const unsigned int STRIDE = COLS;
      
      /// Constructor
      col_iterator(T *col_begin):p(col_begin){}
      
      /// prefix increment operator
      col_iterator &operator++(){
        p+=STRIDE;
        return *this;
      }
      /// prefix increment operator (const)
      const col_iterator &operator++() const{
        p+=STRIDE;
        return *this;
      }
      /// postfix increment operator
      col_iterator operator++(int){
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      /// postfix increment operator (const)
      const col_iterator operator++(int) const{
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /// prefix decrement operator
      col_iterator &operator--(){
        p-=STRIDE;
        return *this;
      }

      /// prefix decrement operator (const)
      const col_iterator &operator--() const{
        p-=STRIDE;
        return *this;
      }

      /// postfix decrement operator
      col_iterator operator--(int){
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// postfix decrement operator (const)
      const col_iterator operator--(int) const{
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// jump next n elements (inplace)
      col_iterator &operator+=(difference_type n){
        p += n * STRIDE;
        return *this;
      }

      /// jump next n elements (inplace) (const)
      const col_iterator &operator+=(difference_type n) const{
        p += n * STRIDE;
        return *this;
      }


      /// jump backward next n elements (inplace)
      col_iterator &operator-=(difference_type n){
        p -= n * STRIDE;
        return *this;
      }

      /// jump backward next n elements (inplace) (const)
      const col_iterator &operator-=(difference_type n) const{
        p -= n * STRIDE;
        return *this;
      }


      /// jump next n elements
      col_iterator operator+(difference_type n) {
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump next n elements (const)
      const col_iterator operator+(difference_type n) const{
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }

      /// jump backward next n elements
      col_iterator operator-(difference_type n) {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      /// jump backward next n elements (const)
      const col_iterator operator-(difference_type n) const {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }

      /// steps between two iterators ... (todo: check!)
      difference_type operator-(const col_iterator &i) const{
        return (p-i.p)/STRIDE;
      }
      
      /// Dereference operator
      T &operator*(){
        return *p;
      }

      /// const Dereference operator
      const T &operator*() const{
        return *p;
      }

      /// comparison operator ==
      bool operator==(const col_iterator &i) const{ return p == i.p; }

      /// comparison operator !=
      bool operator!=(const col_iterator &i) const{ return p != i.p; }

      /// comparison operator <
      bool operator<(const col_iterator &i) const{ return p < i.p; }

      /// comparison operator <=
      bool operator<=(const col_iterator &i) const{ return p <= i.p; }

      /// comparison operator >=
      bool operator>=(const col_iterator &i) const{ return p >= i.p; }

      /// comparison operator >
      bool operator>(const col_iterator &i) const{ return p > i.p; }

    };
  
    // const column operator typedef
    typedef const col_iterator const_col_iterator;

    /// returns an iterator to first element iterating over each element (row-major order)
    iterator begin() { return m_data; }

    /// returns an iterator after the last element
    iterator end() { return m_data+dim(); }
  
    /// returns an iterator to first element iterating over each element (row-major order) (const)
    const_iterator begin() const { return m_data; }

    /// returns an iterator after the last element (const)
    const_iterator end() const { return m_data+dim(); }
  
    /// returns an iterator iterating over a certain column 
    col_iterator col_begin(unsigned int col) { return col_iterator(m_data+col); }

    /// row end iterator
    col_iterator col_end(unsigned int col) { return col_iterator(m_data+col+dim()); }
  
    /// returns an iterator iterating over a certain column (const)
    const_col_iterator col_begin(unsigned int col) const { return col_iterator(m_data+col); }

    /// row end iterator const
    const_col_iterator col_end(unsigned int col) const { return col_iterator(m_data+col+dim()); }

    /// returns an iterator iterating over a certain row
    row_iterator row_begin(unsigned int row) { return m_data+row*cols(); }

    /// row end iterator
    row_iterator row_end(unsigned int row) { return m_data+(row+1)*cols(); }

    /// returns an iterator iterating over a certain row (const)
    const_row_iterator row_begin(unsigned int row) const { return m_data+row*cols(); }

    /// row end iterator (const)
    const_row_iterator row_end(unsigned int row) const { return m_data+(row+1)*cols(); }


    /// Matrix multiplication (essential)
    /** matrices multiplication A*B is only valid if cols(A) is equal to rows(B).
        Resulting matrix has dimensions cols(B) x rows(A)
        @param m right operator in matrix multiplication
    */
    template<unsigned int MCOLS>
    FixedMatrix<T,MCOLS,ROWS> operator*(const FixedMatrix<T,MCOLS,COLS> &m) const{
      FixedMatrix<T,MCOLS,ROWS> d;
      
      // for(unsigned int c=0;c<MCOLS;++c){
      //   for(unsigned int r=0;r<ROWS;++r){
      
      
      // for(unsigned int c=0;c<MCOLS;++c){
      //   for(unsigned int r=0;r<ROWS;++r){
      //     d(c,r) = std::inner_product(m.col_begin(c),m.col_end(c),row_begin(r),T(0));
      //   }
      // }
      return d;
    }
    /**          __MCOLS__
                |         |
                |         |
              COLS        |
                |         |
                |_________|
                
       --COLS--  __________
       |      | |         |
       |      | |         |
     ROWS     | |         |
       |      | |         |
       |______| |_________|

    */


    /// invert the matrix (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    /** This function internally uses an instance of DynMatrix<T> */
    FixedMatrix inv() const throw (InvalidMatrixDimensionException,SingularMatrixException){
      DynMatrix<T> m(COLS,ROWS,m_data,false);
      DynMatrix<T> mi = m.inv();
      m.set_data(0);
      FixedMatrix r;
      FixedMatrixBase::optimized_copy<T*,T*,DIM>(mi.begin(),mi.end(),begin());
      //      std::copy(mi.begin(),mi.end(),r.begin());      
      return r;
    }
    
    /// calculate matrix determinant
    T det() const throw(InvalidMatrixDimensionException){
      DynMatrix<T> m(COLS,ROWS,m_data,false);
      return m.det();
    }
  
    /// returns matrix's transposed
    FixedMatrix<T,ROWS,COLS> transp() const{
      FixedMatrix<T,ROWS,COLS> d;
      for(unsigned int i=0;i<cols();++i){
        FixedMatrixBase::optimized_copy<const_col_iterator, typename FixedMatrix<T,ROWS,COLS>::row_iterator,DIM>(col_begin(i),col_end(i),d.row_begin(i));
        //        std::copy(col_begin(i),col_end(i),d.row_begin(i));
      }
      return d;
    }

    /// returns a matrix row-reference  iterator pair
    FixedMatrixPart<T,COLS,row_iterator> row(unsigned int idx){
      return FixedMatrixPart<T,COLS,row_iterator>(row_begin(idx),row_end(idx));
    }

    /// returns a matrix row-reference  iterator pair (const)
    FixedMatrixPart<T,COLS,const_row_iterator> row(unsigned int idx) const{
      return FixedMatrixPart<T,COLS,const_row_iterator>(row_begin(idx),row_end(idx));
    }

    /// returns a matrix col-reference  iterator pair
    FixedMatrixPart<T,ROWS,col_iterator> col(unsigned int idx){
      return FixedMatrixPart<T,ROWS,col_iterator>(col_begin(idx),col_end(idx));
    }

    /// returns a matrix col-reference  iterator pair (const)
    FixedMatrixPart<T,ROWS,const_col_iterator> col(unsigned int idx) const{
      return FixedMatrixPart<T,ROWS,const_col_iterator>(col_begin(idx),col_end(idx));
    }
    
    /// create identity matrix 
    /** if matrix is not a spare one, upper left square matrix
        is initialized with the fitting identity matrix and other
        elements are initialized with 0
    */
    static FixedMatrix<T,ROWS,COLS> id(){
      FixedMatrix<T,ROWS,COLS> m(T(0));
      for(unsigned int i=0;i<ROWS && i<COLS;++i){
        m(i,i) = 1;
      }
      return m;
    }

    /// Calculates the length of the matrix data vector
    inline double length(T norm=2) const{ 
      double sumSquares = 0;
      for(unsigned int i=0;i<DIM;++i){
        sumSquares += ::pow((*this)[i],(double)norm);
      }
      return ::pow( sumSquares, 1.0/norm);
    }
    
  };

 

  /// Matrix multiplication (inplace)
  /** inplace matrix multiplication does only work for squared source and 
      destination matrices of identical size */
  template<class T, unsigned int M_ROWS_AND_COLS,unsigned int V_COLS>
  inline FixedMatrix<T,V_COLS,M_ROWS_AND_COLS> &operator*=(FixedMatrix<T,V_COLS,M_ROWS_AND_COLS> &v,
                                                           const FixedMatrix<T,M_ROWS_AND_COLS,M_ROWS_AND_COLS> &m){
    return v = (m*v);
  } 



  /// put the matrix into a std::ostream (human readable)
  template<class T, unsigned int COLS, unsigned int ROWS>
  inline std::ostream &operator<<(std::ostream &s,const FixedMatrix<T,COLS,ROWS> &m){
    for(unsigned int i=0;i<m.rows();++i){
      s << "| ";
      std::copy(m.row_begin(i),m.row_end(i),std::ostream_iterator<T>(s," "));
      s << "|" << std::endl;
    }
    return s;
  }

  /// creates a 2D rotation matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,2,2> create_rot_2D(T angle);

  /// creates a 2D homogen matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,3,3> create_hom_3x3(T angle, T dx=0, T dy=0, T v0=0, T v1=0);

  /// creates a 2D homogen matrix with translation part only (defined for float and double)
  template<class T>
  inline FixedMatrix<T,3,3> create_hom_3x3_trans(T dx, T dy){
    FixedMatrix<T,3,3> m = FixedMatrix<T,3,3>::id();
    m(2,0)=dx;
    m(2,1)=dy;
    return m;
  }

  
  /// creates a 3D rotation matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,3,3> create_rot_3D(T rx,T ry,T rz);

  /// creates a 3D homogen matrix (defined for float and double)
  template<class T>
  FixedMatrix<T,4,4> create_hom_4x4(T rx, T ry, T rz, T dx=0, T dy=0, T dz=0, T v0=0, T v1=0, T v2=0);


  /// creates a 3D homogen matrix with translation part only (defined for float and double)
  template<class T>
  inline FixedMatrix<T,4,4> create_hom_4x4_trans(T dx, T dy, T dz){
    FixedMatrix<T,4,4> m = FixedMatrix<T,4,4>::id();
    m(3,0)=dx;
    m(3,1)=dy;
    m(3,2)=dy;
    return m;
  }

  /// calculate the trace of a square matrix
  template<class T, unsigned int ROWS_AND_COLS>
  FixedMatrix<T,1,ROWS_AND_COLS> trace(const FixedMatrix<T,ROWS_AND_COLS,ROWS_AND_COLS> &m){
    FixedMatrix<T,1,ROWS_AND_COLS> t;
    for(unsigned int i=0;i<ROWS_AND_COLS;++i){
      t[i] = m(i,i);
    }
    return t;
  }


  /** \cond  declared and documented above */
  template<class T,unsigned int N, class Iterator> template<unsigned int COLS>
  inline FixedMatrixPart<T,N,Iterator>& FixedMatrixPart<T,N,Iterator>::operator=(const FixedMatrix<T,COLS,N/COLS> &m){
    FixedMatrixBase::optimized_copy<const T*,Iterator,N>(m.begin(),m.end(),begin);
    //std::copy(m.begin(),m.end(),begin);
    return *this;
  }
  template<class T,unsigned int N, class Iterator> template<class T2, unsigned int COLS>
  inline FixedMatrixPart<T,N,Iterator>& FixedMatrixPart<T,N,Iterator>::operator=(const FixedMatrix<T2,COLS,N/COLS> &m){
    std::transform(m.begin(),m.end(),begin,clipped_cast<T2,T>);
    return *this;
  }
  /** \endcond */


}

#endif
