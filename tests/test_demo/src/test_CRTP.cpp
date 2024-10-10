#include <iostream>
#include <chrono>
#include "ScopeProfiler.h"

class DynamicInterface {
public:
    virtual void tick(uint64_t n) = 0;
    virtual uint64_t getValue() = 0;
};

class DynamicImplementation : public DynamicInterface {
public:
    DynamicImplementation() : counter{0} {}
    void tick(uint64_t n) {counter += n;}
    uint64_t getValue() {return counter;}
private:
    uint64_t counter;
};

template <typename Implementation>
class CRTPInterface {
public:
    void tick(uint64_t n) {impl().tick(n);}
    uint64_t getValue() {return impl().getValue();}
private:
    Implementation& impl() {
        return *static_cast<Implementation*>(this);
    }
};

class CRTPImplementation : public CRTPInterface<CRTPImplementation> {
public:
    CRTPImplementation() : counter{0} {}
    void tick(uint64_t n) {counter += n;}
    uint64_t getValue() {return counter;}
private:
    uint64_t counter;
};

int main() {
    const unsigned N = 10'000'000;
    {
        ScopeProfiler _("DynamicInterface");
        DynamicImplementation dynObj;
        DynamicInterface* dynPtr = &dynObj;
        for (unsigned i = 0; i < N; ++i) {
            dynPtr->tick(i);
        }
    }
    {
        ScopeProfiler _("CRTPInterface");
        CRTPImplementation crtpObj;
        CRTPInterface<CRTPImplementation>* crtpPtr = &crtpObj;
        for (unsigned i = 0; i < N; ++i) {
            crtpPtr->tick(i);
        }
    }
    printScopeProfiler();
    return 0;
}