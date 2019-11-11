#pragma once

// 环形缓冲区（先进先出）
// Size必须为2的幂

template<typename Elem, unsigned long Size> class CFifo
{
    // 检测大小是否是2的幂
    static_assert((Size == 1) || !(Size & (Size - 1)));

public:
    CFifo() : _begin(0), _end(0) {}

    bool          empty() const { return _begin == _end; }
    bool          full() const { return _begin == (_end ^ Size); }
    unsigned long size() const { return _end >= _begin ? _end - _begin : _begin - _end + Size; }
    unsigned long capacity() const { return Size; }

    struct Iterator  // 前向随机迭代器
    {
        CFifo *       cfifo;
        unsigned long cur;

        Elem &    operator*() { return cfifo->elems[cur & (Size - 1)]; }

        // 线性访问
        Iterator &operator++()
        {
            cur = cfifo->incr(cur);
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            cur          = cfifo->incr(cur);
            return tmp;
        }
        bool operator==(const Iterator &it) { return cur == it.cur; }
        bool operator!=(const Iterator &it) { return cur != it.cur; }

        // 随机访问
        Elem &    operator[](unsigned long n) { return *(*this + n); }
        Iterator &operator+=(unsigned long n)
        {
            cur = cfifo->incr(cur + n - 1);
            return *this;
        }
        friend Iterator operator+(const Iterator &it, unsigned long n)
        {
            Iterator tmp = it;
            return tmp += n;
        }
        friend Iterator operator+(unsigned long n, const Iterator &it)
        {
            Iterator tmp = it;
            return tmp += n;
        }
    };
    Iterator begin() { return Iterator {this, _begin}; }
    Iterator end() { return Iterator {this, _end}; }

    Elem &   head() { return elems[_begin & (Size - 1)]; }
    Elem &   tail() { return elems[(_end - 1) & (Size - 1)]; }
    Elem &   operator[](unsigned long n) { return begin()[n]; }

    template<bool Overwrite = false> bool put(const Elem &e)
    {
        bool isFull = full();
        if (!Overwrite && isFull)
            return false;

        elems[_end & (Size - 1)] = e;

        if (isFull)
            _begin = incr(_begin);

        _end = incr(_end);

        return true;
    }

    Elem get()
    {
        Elem &e = head();
        _begin  = incr(_begin);
        return e;
    }

private:
    Elem elems[Size];

    // 序号范围为[0, 2 * Size - 1], 超出Size部分表示满的情况
    // 这样就不需要浪费一位缓冲区空间来表示满的状态
    unsigned long _begin;
    unsigned long _end;

    int incr(int i) { return (i + 1) & (2 * Size - 1); }
};