#include "Cidic.h"
#include "Cidic_p.h"
#include <algorithm>
#include <cstring>
#include <fstream>

#define CIDIC_FILE_MAX_BUFFER   2048


Cidic::Cidic()
{
    d_ptr = new CidicPrivate();
}

Cidic::~Cidic()
{
    unloadDict();
}

/*
CidicStrAttrItem Cidic::getStringBasicType(const char *string,
                                           CidicEntryIndex index)
{
    CidicStrAttrItem attribute = 0;
    int stringIndex = 0;

    if (string == nullptr)
        return attribute;

    if (index == 0)
    {
        // Search for the first matched string
        std::vector<const char*>::iterator i;
        int length = strlen(string);
        for (i = stringList.begin(); i != stringList.end(); i++)
        {
            if (strncmp(string, *i, length) == 0)
            break;
        }

        if (i != stringList.end())
            stringIndex = i - stringList.begin() + 1;
    }
    else
    {
        // Search string index in specified entry
        const CidicEntry& entry = entryList[index];
        int i;
        for (i = 0; i < entry.source_count; i++);
        {
            if (strcmp(stringList[entry.source[i]], string) == 0)
            {
                stringIndex = entry.source[i];
                break;
            }
        }
        if (stringIndex == 0)
        {
            for (i = 0; i < entry.target_count; i++);
            {
                if (strcmp(stringList[entry.target[i]], string) == 0)
                {
                    stringIndex = entry.target[i];
                    break;
                }
            }
        }
    }

    if (stringIndex > 0)
        attribute = stringAttributeList[stringIndex].attrItems[0];

    return attribute;
}
*/


bool Cidic::loadDictFile(const char *filePath)
{
    std::ifstream file;
    file.open(filePath);
    if (!file.is_open())
        return false;

    unloadDict();
    d_ptr->initDict();

    char* line = new char[CIDIC_FILE_MAX_BUFFER + 1];

    // First, extract language strings and their attributes from file
    char* langString = nullptr;
    CidicStrAttr strAttr;
    while (!file.eof())
    {
        // Here we assume entry seperator is '\n',
        // so we use getline() instead of get()
        file.getline(line, CIDIC_FILE_MAX_BUFFER);

        if (strlen(line) < 1)
            break;
        else if (strstr(line, CIDIC_FILE_COMMENT_PREFIX) ||
                 strstr(line, CIDIC_FILE_FEATURE_PREFIX))
            continue;

        if (CidicPrivate::parseLangStringFromFile(line, langString, strAttr))
        {
            d_ptr->stringList.push_back(langString);
            d_ptr->stringAttributeList.push_back(strAttr);
        }
    }

    // Then, extract language entries from file
    CidicEntry entry;
    while (!file.eof())
    {
        file.getline(line, CIDIC_FILE_MAX_BUFFER);

        if (strstr(line, CIDIC_FILE_COMMENT_PREFIX) ||
            strstr(line, CIDIC_FILE_FEATURE_PREFIX))
            continue;

        if (CidicPrivate::parseLangEntryFromFile(line, entry))
            d_ptr->entryList.push_back(entry);
    }

    file.close();
    delete line;
    return true;
}

CidicQueryResult Cidic::lookUp(const char *string,
                               int length,
                               CidicLangType lang)
{
    // Currently, Cidic takes account of only two languages:
    // lang = 1: looking up for "source" language
    // lang = 2: looking up for "target" language

    auto& stringList = d_ptr->stringList;
    auto& entryList = d_ptr->entryList;

    CidicQueryResult result;
    result.entry = 0;
    result.matchedAttribute = nullptr;

    std::vector<const char*>::iterator i;
    for (i = stringList.begin(); i != stringList.end(); i++)
    {
        if (*i &&
            length == strlen(*i) &&
            strncmp(string, *i, length) == 0)
            break;
    }
    if (i == stringList.end())
        return result;

    CidicEntryIndex entryIndex = 0;
    std::vector<CidicEntry>::const_iterator j;
    CidicStrIndex strIndex = i - stringList.begin();
    int k;
    bool found = false;
    if (lang == 1)
    {
        for (j = entryList.cbegin(); j != entryList.cend(); j++)
        {
            for (k = 0; k < (*j).source_count; k++)
            {
                if ((*j).source[k] == strIndex)
                {
                    entryIndex = j - entryList.begin();
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }
    else if (lang == 2)
    {
        for (j = entryList.cbegin(); j != entryList.cend(); j++)
        {
            for (k = 0; k < (*j).target_count; k++)
            {
                if ((*j).target[k] == strIndex)
                {
                    entryIndex = j - entryList.begin();
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }

    result.entry = entryIndex;
    result.matchedAttribute = &d_ptr->stringAttributeList[strIndex];
    return result;
}



CidicStrRelation Cidic::predictRelation(const CidicStrAttr* attribute1,
                                        const CidicStrAttr* attribute2)
{
    // TODO: Predict the relationship of two entries by their attributes
    if (attribute1 == nullptr)
    {
        if (attribute2 == nullptr)
            return CIDIC_STR_RELATION_PARALLEL;
        else
            return CIDIC_STR_RELATION_PARENT;
    }
    else
    {
        if (attribute2 == nullptr)
            return CIDIC_STR_RELATION_CHILD;
        else
            return CIDIC_STR_RELATION_PARALLEL;
    }
}

const char* Cidic::toString(CidicEntryIndex index, CidicLangType lang)
{
    if (d_ptr->entryList.size() < (unsigned int)index)
        return nullptr;

    CidicStrIndex strIndex = 0;
    if (lang == 1)
    {
        if (d_ptr->entryList[index].source_count > 0 )
            strIndex = d_ptr->entryList[index].source[0];
    }
    else if (lang == 2)
    {
        if (d_ptr->entryList[index].target_count > 0 )
            strIndex = d_ptr->entryList[index].target[0];
    }

    if (strIndex > 0)
        return d_ptr->stringList[strIndex];
    else
        return nullptr;
}

void Cidic::unloadDict()
{
    // Clear all items in the language entry list
    std::vector<CidicEntry>::iterator i;
    for (i = d_ptr->entryList.begin(); i != d_ptr->entryList.end(); i++)
    {
        delete (*i).source;
        delete (*i).target;
    }

    // Clear all items in the string list
    std::vector<const char*>::iterator j;
    for (j = d_ptr->stringList.begin(); j != d_ptr->stringList.end(); j++)
    {
        delete *j;
    }

    // Clear all items in the string attribute list
    std::vector<CidicStrAttr>::iterator k;
    for (k = d_ptr->stringAttributeList.begin();
         k != d_ptr->stringAttributeList.end();
         k++)
    {
        if ((*k).attr_count > 0)
            delete (*k).attrItems;
    }
}

void CidicPrivate::initDict()
{
    CidicEntry entry;
    entry.source = nullptr;
    entry.source_count = 0;
    entry.target = nullptr;
    entry.target_count = 0;
    entryList.push_back(entry);

    stringList.push_back(nullptr);

    CidicStrAttr strAttr;
    strAttr.attrItems = nullptr;
    strAttr.attr_count = 0;
    stringAttributeList.push_back(strAttr);
}

bool CidicPrivate::parseLangEntryFromFile(const char *line, CidicEntry &entry)
{
    const char* p = strstr(line, CIDIC_FILE_LANG_SEPERATOR);
    if (p == nullptr)
        return false;

    entry.source_count =
            CidicPrivate::parseLangEntryItems(line,
                                              p - line,
                                              entry.source);
    entry.target_count =
            CidicPrivate::parseLangEntryItems(p + 1,
                                              strlen(line) - (p - line) - 1,
                                              entry.target);

    return true;
}

int CidicPrivate::parseLangEntryItems(const char* entry,
                                      int length,
                                      CidicStrIndex *&index)
{
    int count = 0;
    const char *p = entry, *p2;
    char* buffer[CIDIC_FILE_ITEM_MAX];

    do
    {
        p2 = strstr(p, CIDIC_FILE_ITEM_SEPERATOR);
        if (p2 == nullptr)
            p2 = entry + length;

        buffer[count] = new char[p2 - p + 1];
        strncpy(buffer[count++], entry, p2 - p);
        p = p2 + 1;
    }while (p - entry <= length);

    if (count > 0)
    {
        index = new int[count];
        for (int i = 0; i < count; i++)
            index[i] = strtol(buffer[i], nullptr, 10);
    }

    return count;
}

bool CidicPrivate::parseLangStringFromFile(const char* line,
                                           char*& string,
                                           CidicStrAttr& attribute)
{
    const int length = strlen(line);
    string = nullptr;

    if (length < 1)
        return false;

    // Search for the first seperator
    const char *p, *p2;
    p = strstr(line, CIDIC_FILE_ITEM_SEPERATOR);
    if (p == nullptr)
        p = p + length;

    // Extract the first item as language string
    string = new char[p - line + 1];
    strncpy(string, line, p - line);
    p++;

    // Extract following items as string properties
    int attrItem_count = 0;
    char attrItem[sizeof(CidicStrAttrItem)];
    CidicStrAttrItem attrItems[CIDIC_FILE_ITEM_MAX];
    memset(attrItems, 0, sizeof(CidicStrAttrItem) * CIDIC_FILE_ITEM_MAX);
    while (p - line <= length)
    {
        p2 = strstr(p, CIDIC_FILE_ITEM_SEPERATOR);
        if (p2 == nullptr)
            p2 = line + length + 1;

        if ((unsigned)(p2 - p) > sizeof(CidicStrAttrItem))
            strncpy(attrItem, p, sizeof(CidicStrAttrItem));
        else
            strncpy(attrItem, p, p2 - p);
        memcpy(&attrItems[attrItem_count], attrItem, sizeof(CidicStrAttrItem));

        attrItem_count++;
        p = p2 + 1;
    }

    attribute.attr_count = attrItem_count;
    attribute.attrItems = new CidicStrAttrItem[attrItem_count];
    for (int i = 0; i < attrItem_count; i++)
        attribute.attrItems[i] = attrItems[i];

    return true;
}
