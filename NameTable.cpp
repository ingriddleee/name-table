// NameTable.cpp

#include "NameTable.h"
#include <string>
#include <vector>
#include <functional>
using namespace std;

class HashMap {
public:
    HashMap() : table{TABLE_SIZE, std::vector<Value>{}} {} //make vector of TABLE_SIZE and have each hold an empty vector
    ~HashMap() {}
    int get(const string& id) const {
        size_t hashValue = hash(id);
        if (table[hashValue].empty()) return -1; //nothing to return

        for (int i = table[hashValue].size()-1; i >= 0; i--) {
            //return the last one (most recent scope) -- scope doesn't matter since it's just the most recent one 
            if (table[hashValue][i].IDstring == id) {
                return table[hashValue][i].lineNum;
            }
        }

        return -1; //bad collision and were none
    }

    //overloaded get function for verifying that we didn't declare the same thing in a scope 
    int getScope(const string& id) const {
        size_t hashValue = hash(id);
        if (table[hashValue].empty()) return -1; //nothing to return

        for (int i = table[hashValue].size()-1; i >= 0; i--) {
            //return the last one (most recent scope)
            if (table[hashValue][i].IDstring == id) return table[hashValue][i].scope; 
        }

        return -1; //bad collision and were none
    }

    void set(const string& id, const int& value, const int scope) {
        size_t hashValue = hash(id);
        table[hashValue].push_back(Value(id, value, scope));
    }

    bool remove(const string& id, const int scope) {
        size_t hashValue = hash(id); //1. hash it 
        if (table[hashValue].empty()) return false; //2. if nothing there it's false
        //otherwise, make sure they match then remove

        for (int i = table[hashValue].size() - 1; i >= 0; i--) {
            //remove if it's the right one 
            if (table[hashValue][i].scope == scope && table[hashValue][i].IDstring == id) {
                table[hashValue].erase(table[hashValue].begin() + i);
                return true;
            }
        }

        return false; //wrong scope, called remove wrong
    }

private:
    const size_t TABLE_SIZE = 20000;
    std::hash<std::string> hash_fn; //the hash function we use for the map
    size_t hash(const string& str) const{
        std::size_t str_hash = hash_fn(str);
        return str_hash % TABLE_SIZE;
    }

    struct Value {
        Value(string id, int line, int sc) : IDstring(id), lineNum(line), scope(sc) {}
        string IDstring;
        int lineNum;
        int scope;
    };

    // hash table
    vector < vector<Value> > table;

}; 

// This class does the real work of the implementation.
class NameTableImpl
{
public:
    NameTableImpl() : scopes{ 1, std::vector<string>{} } {} //initalize the general scope
    void enterScope();
    bool exitScope();
    bool declare(const string& id, int lineNum);
    int find(const string& id) const;
private:
    HashMap map; //holds a map of Values
    vector< vector<string> > scopes; //vector of vectors holding the new variables in each scope
};

void NameTableImpl::enterScope()
{
    vector<string> temp; //allocate new vector for the new scope
    scopes.push_back(temp); //push back to scopes vector
}

bool NameTableImpl::exitScope()
{
    //if scopes is empty return false
    if (scopes.size() == 1) return false; //only the initial scope

    //1. delete all the line values from that scope
    vector<string> curScope = scopes.back();
    for (int i = 0; i < (curScope).size(); i++) map.remove(curScope[i], scopes.size()-1);
    
    //2. delete the scope from the scopes vector
    scopes.pop_back(); //pop it 
    return true;
}

bool NameTableImpl::declare(const string& id, int lineNum)
{
    if (id.empty()) return false;

    //Check for another declaration in the same scope.
    if (map.getScope(id) == scopes.size() - 1) return false;

    //else add to: 1) current scope vector 2) the map
    scopes.back().push_back(id);

    map.set(id, lineNum, scopes.size()-1);
    return true;
}

int NameTableImpl::find(const string& id) const
{
    if (id.empty()) return -1;
    return map.get(id);
}

//*********** NameTable functions **************

// For the most part, these functions simply delegate to NameTableImpl's
// functions.

NameTable::NameTable()
{
    m_impl = new NameTableImpl;
}

NameTable::~NameTable()
{
    delete m_impl;
}

void NameTable::enterScope()
{
    m_impl->enterScope();
}

bool NameTable::exitScope()
{
    return m_impl->exitScope();
}

bool NameTable::declare(const string& id, int lineNum)
{
    return m_impl->declare(id, lineNum);
}

int NameTable::find(const string& id) const
{
    return m_impl->find(id);
}
