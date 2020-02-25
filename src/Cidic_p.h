#include <vector>


typedef int CidicStrIndex;
typedef int CidicStrAttrItem;

struct _CidicStrAttr
{
    int attr_count;
    CidicStrAttrItem* attrItems;
};
typedef struct _CidicStrAttr CidicStrAttr;

struct _CidicEntry
{
    int source_count;
    int target_count;
    CidicStrIndex* source;
    CidicStrIndex* target;
};
typedef struct _CidicEntry CidicEntry;

class CidicPrivate
{
public:
    std::vector<const char*> stringList;
    std::vector<CidicStrAttr> stringAttributeList;
    std::vector<CidicEntry> entryList;

    void initDict();

    static bool parseLangEntryFromFile(const char* line,
                                       CidicEntry& entry);
    static int parseLangEntryItems(const char* entry,
                                   int length,
                                   CidicStrIndex*& index);
    static bool parseLangStringFromFile(const char* line,
                                        char*& string,
                                        CidicStrAttr& attribute);
};
