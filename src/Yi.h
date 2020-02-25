#include <list>


typedef struct _Yitem Yitem;

class Cidic;

class Yi
{
public:
    Yi();
    ~Yi();
    bool loadDict(Cidic* cidic);
    bool parse(const char* source, int length = 0);
    char* toString();
    char* translate(const char* source, int length = 0);

private:
    Cidic* dict;
    Yitem* head;

    Yitem* createNode();
    void deleteNode(Yitem* node);
    void deleteTree(Yitem* node);
    void resetNode(Yitem* node);
    void insertNode(Yitem* node, Yitem* lastNode);
    void insertNodeAsChild(Yitem* node, Yitem* parent);
    void insertNodeAsParent(Yitem* node, Yitem* child);
    void insertNodeAsSibling(Yitem* node, Yitem* sibling);

    std::list<char*>::iterator
         nodeToStrings(const Yitem* node,
                       std::list<char*>* list,
                       std::list<char*>::iterator index);
};
