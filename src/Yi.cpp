#include <list>
#include <cstring>
#include "Yi.h"
#include "Cidic.h"

struct _Yitem
{
    Yitem* parent;
    std::list<Yitem*> childList;
    CidicEntryIndex index;
    char* reference;
    const CidicStrAttr* refAttribute;
};

Yi::Yi()
{
    dict = nullptr;
    head = nullptr;
}

Yi::~Yi()
{
    if (head != nullptr)
    {
        deleteTree(head);
    }
}

void Yi::deleteNode(Yitem* node)
{
    if (node == nullptr)
        return;

    delete node->reference;
    delete node;
}

void Yi::deleteTree(Yitem* node)
{
    if (node == nullptr)
        return;

    if (node->childList.size() > 0)
    {
        std::list<Yitem*>::iterator i;
        for (i = node->childList.begin(); i != node->childList.end(); i++)
            deleteTree(*i);
    }
    deleteNode(node);
}

void Yi::resetNode(Yitem* node)
{
    if (node == nullptr)
        return;

    node->index = 0;
    node->refAttribute = nullptr;
    node->reference = nullptr;if (node->childList.size() > 0)
    {
        std::list<Yitem*>::iterator i;
        for (i = node->childList.begin(); i != node->childList.end(); i++)
            deleteTree(*i);
    }
}

void Yi::insertNode(Yitem* node, Yitem* lastNode)
{
    switch (dict->predictRelation(node->refAttribute, lastNode->refAttribute))
    {
        case CIDIC_STR_RELATION_PARALLEL:
            insertNodeAsSibling(node, lastNode);
            break;
        case CIDIC_STR_RELATION_CHILD:
            insertNodeAsChild(node, lastNode);
            break;
        case CIDIC_STR_RELATION_PARENT:
            insertNodeAsParent(node, lastNode);
            break;
        case CIDIC_STR_RELATION_UNKNOWN:
        default:
            insertNodeAsSibling(node, lastNode);
    }
}

void Yi::insertNodeAsChild(Yitem* node, Yitem* parent)
{
    parent->childList.push_back(node);
    node->parent = parent;
}

void Yi::insertNodeAsParent(Yitem* node, Yitem* child)
{
    if (child->parent != nullptr)
    {
        child->parent->childList.remove(child);
        child->parent->childList.push_back(node);
    }
    node->childList.push_back(child);
    node->parent = child->parent;
    child->parent = node;
}

void Yi::insertNodeAsSibling(Yitem* node, Yitem* sibling)
{
    if (sibling->parent == nullptr)
        return;
    sibling->parent->childList.push_back(node);
    node->parent = sibling->parent;
}

std::list<char*>::iterator
Yi::nodeToStrings(const Yitem* node,
                  std::list<char*>* list,
                  std::list<char*>::iterator index)
{
    // TODO: consider the nature of the target language when serializing

    // First insert child nodes
    std::list<char*>::iterator& newIndex = index;
    std::list<Yitem*>::const_iterator i = node->childList.cbegin();
    while (i != node->childList.cend())
    {
        newIndex = nodeToStrings(*i, list, newIndex);
        i++;
    }

    // Then get translated string of this node
    char* tempString = nullptr;
    const char* translated = dict->toString(node->index, 2);
    if (translated == nullptr)
    {
        // No translated string generated, use its reference instead
        if (node->reference != nullptr)
        {
            tempString = new char[strlen(node->reference) + 1];
            strcpy(tempString, node->reference);
        }
    }
    else
    {
        // Use translated string
        tempString = new char[strlen(translated) + 1];
        strcpy(tempString, translated);
    }

    // Insert this node
    if (tempString)
        list->insert(index, tempString);
//    if (i == node->childList.cbegin())
//    {
//        // No child node; insert at current position
//        list->insert(index, tempString);
//    }
//    else
//    {
//        // Insert after the child nodes
//        list->insert(index + std::distance(i, node->childList.cbegin()),
//                     tempString);
//    }

    return newIndex;
}

bool Yi::loadDict(Cidic* cidic)
{
    if (cidic == nullptr)
        return false;
    else
    {
        dict = cidic;
        return true;
    }
}

bool Yi::parse(const char* source, int length)
{
    if (dict == nullptr)
        return false;
    if (length <= 0)
        length = strlen(source);

    const char *p = source, *pLastString = source;
    int matchedLength;
    CidicQueryResult result;
    Yitem* tempItem = nullptr;
    Yitem* lastItem = new Yitem;
    resetNode(lastItem);
    while (true)
    {
        // Use greedy strategy: try to match as much content as possible
        result.entry = 0;
        matchedLength = length - (p - source);
        while (matchedLength > 0)
        {
            result = dict->lookUp(p, matchedLength, 1);
            if (result.entry > 0)
                break;
            else
                matchedLength--;
        }

        if (result.entry > 0 || p - source >= length)
        {
            if (pLastString < p)
            {
                // Add previously unparsed string to a node
                tempItem = new Yitem;
                tempItem->index = 0;
                tempItem->reference = new char[p - pLastString + 1];
                strncpy(tempItem->reference, pLastString, p - pLastString);
                tempItem->reference[p - pLastString] = '\0';
                tempItem->refAttribute = nullptr;
                insertNode(tempItem, lastItem);
                lastItem = tempItem;
            }

            if (p - source >= length)
                break;
        }

        if (result.entry > 0)
        {
            // Add parsed string to a node
            tempItem = new Yitem;
            tempItem->index = result.entry;
            tempItem->reference = new char[matchedLength + 1];
            strncpy(tempItem->reference, p, matchedLength);
            tempItem->reference[matchedLength] = '\0';
            tempItem->refAttribute = result.matchedAttribute;
            insertNode(tempItem, lastItem);
            lastItem = tempItem;
            p += matchedLength;
            pLastString = p;
        }
        else
        {
            // Add current char to unparsed string
            p++;
        }
    }

    if (tempItem != nullptr)
    {
        // Find the root of the constructed tree,
        // and let the head point to it
        while (lastItem->parent != nullptr)
            lastItem = lastItem->parent;
        head = lastItem;
        return true;
    }
    else
    {
        // No parsed entry, do not change the head
        delete lastItem;
        return false;
    }
}

char* Yi::toString()
{
    // Serialize the tree into a list of string
    std::list<char*> outputList;
    nodeToStrings(head, &outputList, outputList.begin());
    if (outputList.size() <= 0)
        return nullptr;

    // Estimate the total length of the output
    int length, totalLength = 0;
    std::list<char*>::const_iterator i;
    for (i = outputList.cbegin(); i != outputList.cend(); i++)
    {
        length = strlen(*i);
        totalLength += length;
    }

    // Concatenate strings in the list
    int p = 0;
    char* output = new char[totalLength + 1];
    for (i = outputList.cbegin(); i != outputList.cend(); i++)
    {
        length = strlen(*i);
        strncpy(&(output[p]), *i, length);
        p += length;
    }
    return output;
}

char* Yi::translate(const char* source, int length)
{
    if (!parse(source, length))
        return nullptr;
    else
        return toString();
}
