#ifndef ACCESSOR_H_
#define ACCESSOR_H_

// T    getFoo ()             const { return foo; }
// void setFoo (const T &foo)       { this->foo=foo; }
#define value_reader(type, capitalName, name) type get ## capitalName () const           { return name;     }
#define value_writer(type, capitalName, name) void set ## capitalName (const type &name) { this->name=name; }
#define value_accessor(type, capitalName, name) \
        value_reader (type, capitalName, name) \
        value_writer (type, capitalName, name)

// bool isFoo ()          const { return foo; }
// void setFoo (bool foo)       { this->foo=foo; }
#define bool_reader(capitalName, name) bool is  ## capitalName () const    { return name;     }
#define bool_writer(capitalName, name) void set ## capitalName (bool name) { this->name=name; }
#define bool_accessor(capitalName, name) \
        bool_reader (capitalName, name) \
        bool_writer (capitalName, name)

// T    *getFoo ()             const { return foo; }
// void  setFoo (const T *foo)       { this->foo=foo; }

#endif

