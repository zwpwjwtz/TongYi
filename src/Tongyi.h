#include "Cidic.h"


class TongYiPrivate;

class Tongyi
{
public:
    Tongyi();
    ~Tongyi();
    bool loadDict(Cidic* cidic);
    bool parse(const char* source);
    char* toString();
    char* translate(const char *source);

protected:
    TongYiPrivate* d_ptr;
};
