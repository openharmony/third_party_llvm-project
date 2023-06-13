namespace OHOS
{
    class RefCounter
    {
    public:
        int i = 1;
    };
    class WeakRefCounter
    {
    public:
        WeakRefCounter(){}
        WeakRefCounter(RefCounter counter){}
        int i = 1;
    };
    class RefBase
    {
    public:
        RefBase();
        RefCounter *GetRefCounter() const { return refs_; }
        ~RefBase()
        {
            delete refs_;
        }

    private:
        RefCounter *refs_ = new RefCounter();
    };

    template <typename T>
    class sptr
    {
    public:
        sptr() {}
        sptr(T *other) : refs_(other){};
        sptr(const sptr<T> &other) : refs_(other.GetRefPtr()) {}
        sptr(sptr<T> &&other) : refs_(other.GetRefPtr()) {}
        template <typename O>
        sptr(const sptr<O> &other) : refs_(other.GetRefPtr()){}
        ~sptr() { 
            delete refs_; 
        }
        sptr<T> &operator=(sptr<T> &&other)
        {
            refs_ = other.GetRefPtr();
            return *this;
        }
        inline void ForceSetRefPtr(T *other) { refs_ = other; }
        inline operator T *() const { return refs_; }
        inline T &operator*() const { 
            return *refs_; // expected-warning {{Returning null reference}}
        }
        inline T *operator->() const { return refs_; }
        inline bool operator!() const { return refs_ == nullptr; }
        sptr<T> &operator=(T *other)
        {
            refs_ = other;
            return *this;
        }
        sptr<T> &operator=(const sptr<T> &other)
        {
            refs_ = other.GetRefPtr();
            return *this;
        }
        bool operator==(const T *other) const { return refs_ == other; }
        inline bool operator!=(const T *other) const { return !operator==(other); }

        bool operator==(const sptr<T> &other) const { return refs_ == other.GetRefPtr(); }
        inline bool operator!=(const sptr<T> &other) const { return !operator==(other); }
        inline T *GetRefPtr() const { return refs_; }

    private:
        T *refs_ = nullptr;
    };

    template <typename T>
    class wptr
    {
        template <typename O>
        friend class wptr;

    public:
        wptr() {}
        wptr(T *other) : refs_(new WeakRefCounter()) {}

        wptr(const wptr<T> &other) : refs_(new WeakRefCounter()) {}
        wptr(const sptr<T> &other) : refs_(new WeakRefCounter(other.GetRefPtr()->GetRefCounter())) {}
        template <typename O>
        wptr(const wptr<O> &other) : refs_(new WeakRefCounter()) {}
        template <typename O>
        wptr(const sptr<O> &other) : refs_(new WeakRefCounter(other.GetRefPtr()->GetRefCounter())) {}
        ~wptr() { delete refs_; }
        wptr<T> &operator=(T *other);
        template <typename O>
        wptr<T> &operator=(O *other);
        wptr<T> &operator=(const wptr<T> &other);
        wptr<T> &operator=(const sptr<T> &other);
        template <typename O>
        wptr<T> &operator=(const wptr<O> &other);
        template <typename O>
        wptr<T> &operator=(const sptr<O> &other);
        inline T &operator*() const
        {
            return *GetRefPtr();
        }
        inline T *operator->() const
        {
            return reinterpret_cast<T *>(refs_);
        }
        bool operator==(const T *other) const;
        inline bool operator!=(const T *other) const
        {
            return !operator==(other);
        };
        bool operator==(const wptr<T> &other) const;
        inline bool operator!=(const wptr<T> &other) const
        {
            return !operator==(other);
        }
        bool operator==(const sptr<T> &other) const;
        inline bool operator!=(const sptr<T> &other) const
        {
            return !operator==(other);
        }
        T *GetRefPtr() const;
        int GetRefCounter() const
        {
            return refs_->i;
        }

    private:
        WeakRefCounter *refs_ = nullptr;
    };

}