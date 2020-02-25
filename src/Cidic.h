#ifndef CIDIC_H
#define CIDIC_H

#include "cidic_string_attr.h"

#define CIDIC_FILE_COMMENT_PREFIX "//"
#define CIDIC_FILE_FEATURE_PREFIX "//#"
#define CIDIC_FILE_ENTRY_SEPERATOR "\n"
#define CIDIC_FILE_LANG_SEPERATOR "|"
#define CIDIC_FILE_ITEM_SEPERATOR ";"
#define CIDIC_FILE_ITEM_MAX 32

#define CIDIC_STR_RELATION_UNKNOWN 0
#define CIDIC_STR_RELATION_PARALLEL 1
#define CIDIC_STR_RELATION_CHILD 2
#define CIDIC_STR_RELATION_PARENT 3


typedef int CidicEntryIndex;
typedef int CidicLangType;
typedef int CidicStrRelation;

typedef struct _CidicStrAttr CidicStrAttr;

struct CidicQueryResult
{
    CidicEntryIndex entry;
    const CidicStrAttr* matchedAttribute;
};

class CidicPrivate;

class Cidic
{

public:
    Cidic();
    ~Cidic();

    bool loadDictFile(const char* filePath);
    CidicQueryResult lookUp(const char* string, int length, CidicLangType lang);
    CidicStrRelation predictRelation(const CidicStrAttr *attribute1,
                                     const CidicStrAttr *attribute2);
    const char* toString(CidicEntryIndex index, CidicLangType lang);
    void unloadDict();

protected:
    CidicPrivate* d_ptr;
};

#endif // CIDIC_H
