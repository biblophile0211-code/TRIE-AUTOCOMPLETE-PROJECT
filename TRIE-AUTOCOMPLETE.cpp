#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <functional>
#include <cctype>
using namespace std;

struct TrieNode
{
    unordered_map<char, TrieNode*> children;
    bool isEnd = false;
    int  freq  = 0;
};


class Trie
{
public:
    TrieNode* root;
    int nodeCount = 1;
    int wordCount = 0;

    Trie()
{ 
    root = new TrieNode();
}

    // INSERT
    string insert(string word)
{
        for (char& c : word) c = tolower(c);
        if (word.empty()) return R"({"ok":false,"msg":"empty word"})";
        for (char c : word)
            if (!isalpha(c)) return R"({"ok":false,"msg":"letters only"})";

        TrieNode* node = root;
        for (char c : word)
            {
            if (!node->children.count(c))
            {
                node->children[c] = new TrieNode();
                nodeCount++;
            }
            node = node->children[c];
        }
        if (!node->isEnd)
        {
            node->isEnd = true;
            node->freq  = 1;
            wordCount++;
            return R"({"ok":true,"added":true})";
        }
        node->freq++;
        return R"({"ok":true,"added":false})";
    }

    // DELETE
    bool deleteWord(string word)
{
        for (char& c : word) c = tolower(c);
        bool found = false;
        function<bool(TrieNode*, int)> del = [&](TrieNode* node, int i) -> bool {
            if (i == (int)word.size()) {
                if (!node->isEnd) return false;
                node->isEnd = false;
                node->freq  = 0;
                found = true;
                wordCount--;
                return node->children.empty();
            }
            char c = word[i];
            if (!node->children.count(c)) return false;
            bool shouldDel = del(node->children[c], i + 1);
            if (shouldDel) {
                delete node->children[c];
                node->children.erase(c);
                nodeCount--;
                return node->children.empty() && !node->isEnd;
            }
            return false;
        };
        del(root, 0);
        return found;
    }

    // SEARCH / AUTOCOMPLETE
    struct Result { string word; int freq; };

    vector<Result> search(string prefix, int limit = 8)
{
        for (char& c : prefix) c = tolower(c);
        vector<Result> results;
        TrieNode* node = root;
        for (char c : prefix) {
            if (!node->children.count(c)) return results;
            node = node->children[c];
        }
        function<void(TrieNode*, string)> dfs = [&](TrieNode* n, string path)
        {
            if ((int)results.size() >= 50) return;
            if (n->isEnd) results.push_back({path, n->freq});
            for (auto& p : n->children) {
    char ch = p.first;
    TrieNode* child = p.second;
    dfs(child, path + ch);
}
        };
        dfs(node, prefix);
        sort(results.begin(), results.end(), [](const Result& a, const Result& b)
            {
            return a.freq != b.freq ? a.freq > b.freq : a.word < b.word;
        });
        if ((int)results.size() > limit) results.resize(limit);
        return results;
    }

    // GET ALL WORDS
    vector<Result> getAllWords()
{
        vector<Result> words;
        function<void(TrieNode*, string)> dfs = [&](TrieNode* n, string path)
        {
            if (n->isEnd) words.push_back({path, n->freq});
            for (auto& p : n->children)
                {
    char ch = p.first;
    TrieNode* child = p.second;
    dfs(child, path + ch);
}
        };
        dfs(root, "");
        sort(words.begin(), words.end(), [](const Result& a, const Result& b)
            {
            return a.word < b.word;
        });
        return words;
    }

    // STATS
    string getStats() {
        ostringstream oss;
        oss << "{\"words\":" << wordCount << ",\"nodes\":" << nodeCount << "}";
        return oss.str();
    }
};

string escapeJson(const string& s)
{
    string r;
    for (char c : s)
        {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else r += c;
    }
    return r;
}

string resultsToJson(const vector<Trie::Result>& v)
{
    ostringstream oss;
    oss << "[";
    for (int i = 0; i < (int)v.size(); i++)
        {
        if (i) oss << ",";
        oss << "{\"word\":\"" << escapeJson(v[i].word)
            << "\",\"freq\":" << v[i].freq << "}";
    }
    oss << "]";
    return oss.str();
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Trie trie;

    // Pre-load default words
    vector<string> defaults = {
        "algorithm","array","autocomplete","binary","branch","bubble",
        "compiler","complexity","constant","data","delete","dijkstra",
        "dynamic","edge","function","graph","greedy","hash","heap",
        "index","insert","integer","leaf","linear","linked","list",
        "logarithm","loop","merge","node","object","overflow","pointer",
        "prefix","program","queue","quicksort","recursion","search",
        "segment","sort","stack","string","structure","suffix",
        "traverse","tree","trie","variable","vertex"
    };
    for (auto& w : defaults) trie.insert(w);

    string line;
    while (getline(cin, line))
        {
        if (line.empty()) { cout << "{}\n"; cout.flush(); continue; }

        istringstream iss(line);
        string cmd; iss >> cmd;
        string arg; getline(iss, arg);
        if (!arg.empty() && arg[0] == ' ') arg = arg.substr(1);

        if (cmd == "INSERT")
        {
            cout << trie.insert(arg) << "\n";
        }
        else if (cmd == "DELETE") 
        {
            bool ok = trie.deleteWord(arg);
            cout << (ok ? R"({"ok":true})" : R"({"ok":false,"msg":"word not found"})") << "\n";
        }
        else if (cmd == "SEARCH")
        {
            auto r = trie.search(arg);
            cout << resultsToJson(r) << "\n";
        }
        else if (cmd == "ALL") 
        {
            auto r = trie.getAllWords();
            cout << resultsToJson(r) << "\n";
        }
        else if (cmd == "STATS") 
        {
            cout << trie.getStats() << "\n";
        }
        else 
        {
            cout << R"({"error":"unknown command"})" << "\n";
        }
        cout.flush();
    }
    return 0;
}
