#include <vector>
#include "Yi.h"


class TongYiPrivate
{
public:
    TongYiPrivate();
    ~TongYiPrivate();

    Cidic* dict;
    std::vector<Yi*> yiList;
};
