#include <cstdlib>
#include <cstring>
#include "Tongyi.h"
#include "Tongyi_p.h"

#define TONGYI_OUTPUT_SEPARATOR_YI      "."


Tongyi::Tongyi()
{
    d_ptr = new TongYiPrivate;
}

Tongyi::~Tongyi()
{
    delete d_ptr;
}

bool Tongyi::loadDict(Cidic *cidic)
{
    if (cidic == nullptr)
        return false;
    else
    {
        d_ptr->dict = cidic;
        return true;
    }
}

bool Tongyi::parse(const char *source)
{
    if (d_ptr->dict == nullptr)
        return false;

    Yi* next = new Yi;
    next->loadDict(d_ptr->dict);
    while (next->parse(source))
    {
        d_ptr->yiList.push_back(next);
        next = new Yi;
    }
    delete next;

    if (d_ptr->yiList.size() > 0)
        return true;
    else
        return false;
}

char* Tongyi::toString()
{
    int bufferLength, outputLength = 1;
    char* buffer;
    char* output = NULL;

    for (int i=0; i<d_ptr->yiList.size(); i++)
    {
        // Get the translated text
        buffer = d_ptr->yiList[0]->toString();
        if (!buffer)
            continue;

        // Allocate more memory for the output string
        bufferLength = strlen(buffer);
        output = (char*)(realloc(output,
                                 outputLength +
                                 bufferLength +
                                 strlen(TONGYI_OUTPUT_SEPARATOR_YI)));
        if (!output)
            break;

        // Append the translated text
        strcpy(&(output[outputLength - 1]), buffer);
        outputLength += bufferLength;

        // Append a separator
        strcpy(&(output[outputLength]), TONGYI_OUTPUT_SEPARATOR_YI);
        outputLength += strlen(TONGYI_OUTPUT_SEPARATOR_YI);
    }
    return output;
}

char* Tongyi::translate(const char *source)
{
    if (parse(source))
        return toString();
    else
        return nullptr;
}


TongYiPrivate::TongYiPrivate()
{
    dict = nullptr;
}

TongYiPrivate::~TongYiPrivate()
{
    if (yiList.size() > 0)
    {
        std::vector<Yi*>::const_iterator i;
        for (i = yiList.cbegin(); i != yiList.cend(); i++)
            delete *i;
    }
}
