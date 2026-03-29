#include <string>
#include <unordered_map>
#include <vector>


class SymbolTable {
private:
    struct SymbolInfo {
        std::string name;
        double value;
        size_t stringIndex;
    };
    
    std::unordered_map<std::string, size_t> symbolMap;
    std::vector<SymbolInfo> symbols;
    std::vector<std::string> stringPool;
    
public:
    size_t addSymbol(const std::string& name, double value = 0.0);
    bool setValue(const std::string& name, double value);
    bool getValue(const std::string& name, double& value) const;
    bool exists(const std::string& name) const;
    void clear();
    size_t getSymbolCount() const;
};


size_t SymbolTable::addSymbol(const std::string& name, double value) {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        symbols[it->second].value = value;
        return it->second;
    }
    
    size_t stringIndex = stringPool.size();
    stringPool.push_back(name);
    
    size_t symbolIndex = symbols.size();
    symbols.push_back({name, value, stringIndex});
    symbolMap[name] = symbolIndex;
    
    return symbolIndex;
}

bool SymbolTable::setValue(const std::string& name, double value) {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        symbols[it->second].value = value;
        return true;
    }
    return false;
}

bool SymbolTable::getValue(const std::string& name, double& value) const {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        value = symbols[it->second].value;
        return true;
    }
    return false;
}

bool SymbolTable::exists(const std::string& name) const {
    return symbolMap.find(name) != symbolMap.end();
}

void SymbolTable::clear() {
    symbolMap.clear();
    symbols.clear();
    stringPool.clear();
}

size_t SymbolTable::getSymbolCount() const {
    return symbols.size();
}