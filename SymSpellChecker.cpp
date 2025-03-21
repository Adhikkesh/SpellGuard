#include <QApplication>
#include <QMainWindow>
#include <QWebEngineView>
#include <QWebChannel>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <set>
#include <string>
#include <chrono>
#include <iomanip>
#include <cctype>
#include <thread>
#include <mutex>

using namespace std;

struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord;
    int frequency;  // Store word frequency

    TrieNode() : isEndOfWord(false), frequency(0) {}
};

#include <queue>  // Already present, but ensure it’s there for priority_queue

struct Suggestion {
    string word;
    int frequency;
    int editDistance;
    
    Suggestion(const string& w, int f, int d) : word(w), frequency(f), editDistance(d) {}
    
    bool operator<(const Suggestion& other) const {
        if (editDistance != other.editDistance) {
            return editDistance > other.editDistance; // Lower distance is better
        }
        return frequency < other.frequency; // Higher frequency is better
    }
};

std::vector<std::string> generateEdits(const std::string& word, int maxDistance);
int levenshteinDistance(const std::string& s1, const std::string& s2);

// Trie Class
class Trie {
    public:
    TrieNode* root;
    unordered_map<string, int> wordFrequency; // Store word frequencies separately

    Trie() {
        root = new TrieNode();
    }

    ~Trie() {
        deleteTrie(root);
    }

    void insert(const string &word, int frequency = 1) {
        TrieNode *node = root;
        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
        }
        node->isEndOfWord = true;
        node->frequency = frequency;
        
        // Also store in the frequency map
        wordFrequency[word] = frequency;
    }

    bool search(const string &word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                return false;
            }
            node = node->children[ch];
        }
        return node->isEndOfWord;
    }

    int getFrequency(const string &word) {
        // Check if word exists in frequency map
        auto it = wordFrequency.find(word);
        if (it != wordFrequency.end()) {
            return it->second;
        }
        
        // If not in map but in trie, return trie frequency
        TrieNode *node = root;
        for (char c : word) {
            if (node->children.find(c) == node->children.end()) {
                return 0; // Word not found
            }
            node = node->children[c];
        }
        return (node->isEndOfWord) ? node->frequency : 0;
    }

    // Get word count in dictionary
    size_t wordCount() const {
        return wordFrequency.size();
    }

    vector<pair<string, int>> getWordsWithPrefix(const string &prefix, int limit = 10) {
        vector<pair<string, int>> results;
        TrieNode* node = root;
        
        // Navigate to the node representing the prefix
        for (char ch : prefix) {
            if (node->children.find(ch) == node->children.end()) {
                return results; // Prefix not found
            }
            node = node->children[ch];
        }
        
        // Perform DFS to find all words with this prefix
        findAllWordsFromNode(node, prefix, results, limit);
    
        sort(results.begin(), results.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
             return a.second > b.second; 
        });
        
        return results;
    }

    
private:
    void findAllWordsFromNode(TrieNode* node, string prefix, vector<pair<string, int>>& results, int limit) {
        if (!node) return;
        if (results.size() >= static_cast<size_t>(limit)) return;

        if (node->isEndOfWord) {
            results.push_back({prefix, node->frequency});
        }

        for (const auto& child : node->children) {
            findAllWordsFromNode(child.second, prefix + child.first, results, limit);
        }
    }

    void deleteTrie(TrieNode* node) {
        for (auto& pair : node->children) {
            deleteTrie(pair.second);
        }
        delete node;
    }
};

// Progress display class
class ProgressBar {
    private:
        int width;
        char completeChar;
        char incompleteChar;
        
    public:
        ProgressBar(int w = 50, char complete = '=', char incomplete = ' ') 
            : width(w), completeChar(complete), incompleteChar(incomplete) {}
            
        void display(float progress) {
            int barWidth = width - 10; // Leave space for percentage
            
            cout << "\r[";
            int pos = barWidth * progress;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) cout << completeChar;
                else cout << incompleteChar;
            }
            
            cout << "] " << fixed << setprecision(1) << (progress * 100.0) << "%" << flush;
        }
        
        void complete() {
            display(1.0);
            cout << endl;
        }
};

class Trie;
vector<string> correctSpelling(Trie &trie, const string &inputWord, int maxResults) {
    // If the word exists in dictionary, return it as is
    if (trie.search(inputWord)) {
        return {inputWord};
    }
    
    // Priority queue to store suggestions sorted by edit distance and frequency
    priority_queue<Suggestion> suggestions;
    
    // Generate edits
    vector<string> edits = generateEdits(inputWord, 2);
    
    // Check each edit in the trie
    for (const string& edit : edits) {
        if (trie.search(edit)) {
            int frequency = trie.getFrequency(edit);
            int distance = levenshteinDistance(inputWord, edit);
            suggestions.push(Suggestion(edit, frequency, distance));
        }
    }
    
    // Extract top suggestions
    vector<string> results;
    while (!suggestions.empty() && results.size() < static_cast<size_t>(maxResults)) {
        results.push_back(suggestions.top().word);
        suggestions.pop();
    }
    
    // If no suggestions found
    if (results.empty()) {
        results.push_back("No suggestion");
    }
    
    return results;
}


vector<string> getSuggestions(Trie &trie, const string &partialInput, int maxResults = 5) {
    vector<string> suggestions;
    
    // First get exact prefix matches
    auto prefixMatches = trie.getWordsWithPrefix(partialInput, maxResults);
    for (const auto& match : prefixMatches) {
        suggestions.push_back(match.first);
    }
    
    // If we don't have enough suggestions, add spelling corrections
    if (suggestions.size() < static_cast<size_t>(maxResults) && partialInput.length() >= 2) {
        auto corrections = correctSpelling(trie, partialInput, maxResults - suggestions.size());
        
        // Only add corrections that aren't already in suggestions
        for (const auto& correction : corrections) {
            if (correction != "No suggestion") {
                if (find(suggestions.begin(), suggestions.end(), correction) == suggestions.end()) {
                    suggestions.push_back(correction);
                }
            }
        }
    }
    
    return suggestions;
}

// String utilities
namespace StringUtils {
    string toLower(const string& s) {
        string result = s;
        transform(result.begin(), result.end(), result.begin(), 
                  [](unsigned char c) { return tolower(c); });
        return result;
    }
    
    string trim(const string& s) {
        auto start = s.begin();
        while (start != s.end() && isspace(*start)) {
            start++;
        }
        
        auto end = s.end();
        do {
            end--;
        } while (end > start && isspace(*end));
        
        return string(start, end + 1);
    }
    
    bool isAlphaOnly(const string& s) {
        return all_of(s.begin(), s.end(), [](char c) { return isalpha(c); });
    }
}

// Load valid words from a text file
bool loadValidWords(const string &filename, Trie &trie) {
    ifstream file(filename);
    string word;

    if (!file.is_open()) {
        cerr << "Error opening valid words file: " << filename << endl;
        return false;
    }

    cout << "Loading valid words from " << filename << "..." << endl;
    
    // Count lines for progress
    size_t lineCount = 0;
    size_t totalLines = 0;
    
    ifstream countFile(filename);
    string line;
    while (getline(countFile, line)) {
        totalLines++;
    }
    
    ProgressBar progressBar;
    
    // Reset file to beginning
    file.clear();
    file.seekg(0);
    
    // Now load words
    int wordCount = 0;
    while (getline(file, word)) {
        // Trim whitespace
        word = StringUtils::trim(word);
        word = StringUtils::toLower(word);
        
        if (!word.empty() && StringUtils::isAlphaOnly(word)) {
            trie.insert(word, 1); // Default frequency of 1
            wordCount++;
        }
        
        lineCount++;
        if (lineCount % 1000 == 0 || lineCount == totalLines) {
            progressBar.display(static_cast<float>(lineCount) / totalLines);
        }
    }
    
    progressBar.complete();
    cout << "Valid words loaded: " << wordCount << " words processed." << endl;
    return true;
}

// Load frequency data from CSV
bool loadFrequencyData(const string &filename, Trie &trie) {
    ifstream file(filename);
    string line, word;
    int count;

    if (!file.is_open()) {
        cerr << "Error opening frequency file: " << filename << endl;
        return false;
    }

    cout << "Loading frequency data from " << filename << "..." << endl;
    
    // Count lines for progress
    size_t lineCount = 0;
    size_t totalLines = 0;
    
    ifstream countFile(filename);
    string countLine;
    while (getline(countFile, countLine)) {
        totalLines++;
    }
    
    ProgressBar progressBar;
    
    // Reset file to beginning
    file.clear();
    file.seekg(0);
    
    // Skip header if present
    getline(file, line);
    if (line != "word,count") {
        // If no header, process the first line
        stringstream ss(line);
        getline(ss, word, ',');
        word = StringUtils::toLower(word);
        ss >> count;
        if (!word.empty() && trie.search(word)) {
            trie.wordFrequency[word] = count;
        }
    }
    
    // Process the rest of the file
    int wordCount = 0;
    int updatedCount = 0;
    lineCount++;
    
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, word, ',');
        word = StringUtils::toLower(word);
        ss >> count;
        
        // Skip empty words
        if (word.empty()) continue;
        
        // Update frequency only if word exists in the valid words dictionary
        if (trie.search(word)) {
            trie.wordFrequency[word] = count;
            updatedCount++;
        }
        
        wordCount++;
        lineCount++;
        
        if (lineCount % 1000 == 0 || lineCount == totalLines) {
            progressBar.display(static_cast<float>(lineCount) / totalLines);
        }
    }
    
    progressBar.complete();
    cout << "Frequency data loaded: " << wordCount << " entries processed, " 
         << updatedCount << " frequencies updated." << endl;
    return true;
}

// Generate edits for SymSpell algorithm
vector<string> generateEdits(const string &word, int maxDistance = 2) {
    vector<string> edits;
    set<string> uniqueEdits; // To avoid duplicates
    
    // Handle empty word case
    if (word.empty()) return edits;
    
    // Deletions (remove one character)
    for (size_t i = 0; i < word.length(); i++) {
        string edit = word.substr(0, i) + word.substr(i + 1);
        if (uniqueEdits.insert(edit).second) {
            edits.push_back(edit);
        }
    }
    
    // Transpositions (swap adjacent characters)
    for (size_t i = 0; i < word.length() - 1; i++) {
        string edit = word;
        swap(edit[i], edit[i + 1]);
        if (uniqueEdits.insert(edit).second) {
            edits.push_back(edit);
        }
    }
    
    // Substitutions (change one character to another)
    for (size_t i = 0; i < word.length(); i++) {
        string base = word;
        for (char c = 'a'; c <= 'z'; c++) {
            if (word[i] != c) {
                base[i] = c;
                if (uniqueEdits.insert(base).second) {
                    edits.push_back(base);
                }
            }
        }
    }
    
    // Insertions (add one character)
    for (size_t i = 0; i <= word.length(); i++) {
        for (char c = 'a'; c <= 'z'; c++) {
            string edit = word.substr(0, i) + c + word.substr(i);
            if (uniqueEdits.insert(edit).second) {
                edits.push_back(edit);
            }
        }
    }
    
    // For edit distance 2, apply edits to the edits (more efficient implementation)
    if (maxDistance > 1) {
        size_t origSize = edits.size();
        for (size_t i = 0; i < origSize; i++) {
            // Only generate deletions for edit distance 2 (faster but still effective)
            for (size_t j = 0; j < edits[i].length(); j++) {
                string edit2 = edits[i].substr(0, j) + edits[i].substr(j + 1);
                if (uniqueEdits.insert(edit2).second) {
                    edits.push_back(edit2);
                }
            }
        }
    }
    
    return edits;
}


// Function to calculate Levenshtein edit distance
int levenshteinDistance(const string& s1, const string& s2) {
    const size_t len1 = s1.size();
    const size_t len2 = s2.size();
    
    vector<vector<int>> d(len1 + 1, vector<int>(len2 + 1));
    
    for (size_t i = 0; i <= len1; i++) {
        d[i][0] = i;
    }
    
    for (size_t j = 0; j <= len2; j++) {
        d[0][j] = j;
    }
    
    for (size_t i = 1; i <= len1; i++) {
        for (size_t j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            
            d[i][j] = min({
                d[i - 1][j] + 1,      // deletion
                d[i][j - 1] + 1,      // insertion
                d[i - 1][j - 1] + cost // substitution
            });
            
            // Transposition
            if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                d[i][j] = min(d[i][j], d[i - 2][j - 2] + cost);
            }
        }
    }
    
    return d[len1][len2];
}

// Get the word at the cursor position in a text
string getCurrentWordAtCursor(const string& text, int cursorPos) {
    // Ensure valid cursor position
    if (cursorPos < 0) cursorPos = 0;
    if (cursorPos > text.length()) cursorPos = text.length();
    
    // Find the start of the current word
    int wordStart = cursorPos;
    while (wordStart > 0 && isalpha(text[wordStart - 1])) {
        wordStart--;
    }
    
    // Find the end of the current word
    int wordEnd = cursorPos;
    while (wordEnd < text.length() && isalpha(text[wordEnd])) {
        wordEnd++;
    }
    
    // Extract the word
    return text.substr(wordStart, wordEnd - wordStart);
}

// Replace the word at cursor with a suggestion
string replaceWordAtCursor(const string& text, int cursorPos, const string& suggestion) {
    // Ensure valid cursor position
    if (cursorPos < 0) cursorPos = 0;
    if (cursorPos > text.length()) cursorPos = text.length();
    
    // Find the start of the current word
    int wordStart = cursorPos;
    while (wordStart > 0 && isalpha(text[wordStart - 1])) {
        wordStart--;
    }
    
    // Find the end of the current word
    int wordEnd = cursorPos;
    while (wordEnd < text.length() && isalpha(text[wordEnd])) {
        wordEnd++;
    }
    
    // Construct new text with the replacement
    return text.substr(0, wordStart) + suggestion + text.substr(wordEnd);
}


// Example of accepting a suggestion in the UI
void acceptSuggestion(string& text, int& cursorPos, const string& suggestion) {
    // Get the current word length
    string currentWord = getCurrentWordAtCursor(text, cursorPos);
    int currentWordLength = currentWord.length();
    
    // Replace the current word with the suggestion
    text = replaceWordAtCursor(text, cursorPos, suggestion);
    
    // Update cursor position to end of the inserted suggestion
    cursorPos = cursorPos - currentWordLength + suggestion.length();
}

// Throttled suggestion generator to improve UI responsiveness
class ThrottledSuggestionGenerator {
    private:
        Trie& dictionary;
        string lastQuery;
        vector<string> lastResults;
        chrono::steady_clock::time_point lastQueryTime;
        mutex mtx;
        const chrono::milliseconds throttleTime{100}; // 100ms throttle
        
    public:
        ThrottledSuggestionGenerator(Trie& dict) : dictionary(dict) {}
        
        vector<string> getSuggestions(const string& query) {
            lock_guard<mutex> lock(mtx);
            
            auto now = chrono::steady_clock::now();
            
            // If same query or too soon since last query, return cached results
            if (query == lastQuery || 
                (now - lastQueryTime < throttleTime && query.find(lastQuery) == 0)) {
                return lastResults;
            }
            
            // Generate new suggestions
            lastQuery = query;
            lastQueryTime = now;
            lastResults = ::getSuggestions(dictionary, query);
            
            return lastResults;
        }
};
    
class SpellCheckerBridge : public QObject {
    Q_OBJECT

private:
    Trie& dictionary;

public:
    SpellCheckerBridge(Trie& dict, QObject* parent = nullptr)
        : QObject(parent), dictionary(dict) {}

public slots:
    // Get suggestions for a word
    QJsonArray getSuggestions(const QString& word) {
        QJsonArray results;
        string stdWord = word.toLower().toStdString();
        auto suggestions = ::getSuggestions(dictionary, stdWord);
        for (const auto& suggestion : suggestions) {
            results.append(QString::fromStdString(suggestion));
        }
        return results;
    }

    // Check if a word is correctly spelled
    bool checkWord(const QString& word) {
        return dictionary.search(word.toLower().toStdString());
    }

    // Correct spelling
    QJsonArray correctSpelling(const QString& word, int maxResults = 5) {
        QJsonArray results;
        string stdWord = word.toLower().toStdString();
        auto corrections = ::correctSpelling(dictionary, stdWord, maxResults);
        for (const auto& correction : corrections) {
            results.append(QString::fromStdString(correction));
        }
        return results;
    }

    // Get dictionary stats
    QJsonObject getDictionaryStats() {
        QJsonObject stats;
        stats["wordCount"] = static_cast<int>(dictionary.wordCount());
        return stats;
    }

    // Check entire text (new function to support frontend "Check Spelling" button)
    QJsonObject checkText(const QString& text) {
        QJsonObject result;
        string stdText = text.toStdString();
        istringstream iss(stdText);
        string word;
        int checkedWords = 0;
        int errorCount = 0;
        QJsonArray misspelledWords;

        while (iss >> word) {
            checkedWords++;
            string cleanedWord = StringUtils::toLower(StringUtils::trim(word));
            if (!cleanedWord.empty() && StringUtils::isAlphaOnly(cleanedWord) && !dictionary.search(cleanedWord)) {
                errorCount++;
                misspelledWords.append(QString::fromStdString(cleanedWord));
            }
        }

        result["checkedWords"] = checkedWords;
        result["errorCount"] = errorCount;
        result["misspelledWords"] = misspelledWords;
        return result;
    }

    // Set autocorrect state (placeholder for future expansion)
    void setAutocorrect(bool enabled) {
        // Add logic here if you implement autocorrect in the backend
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Initialize dictionary
    Trie dictionary;
    string validWordsFile = "words_alpha.txt";
    string frequencyFile = "english_word_frequency.csv";

    if (argc > 1) validWordsFile = argv[1];
    if (argc > 2) frequencyFile = argv[2];

    cout << "Loading dictionary..." << endl;
    if (!loadValidWords(validWordsFile, dictionary)) {
        cerr << "Failed to load valid words. Exiting." << endl;
        return 1;
    }
    if (!loadFrequencyData(frequencyFile, dictionary)) {
        cout << "No frequency data loaded. Using default frequencies." << endl;
    }

    // Create main window
    QMainWindow window;
    window.setWindowTitle("SymSpell Checker");
    window.resize(1000, 700);

    // Create WebEngineView
    QWebEngineView* webView = new QWebEngineView();

    // Create WebChannel
    QWebChannel* channel = new QWebChannel();

    // Create bridge object with name "spellCheckerBackend" to match JavaScript
    SpellCheckerBridge* bridge = new SpellCheckerBridge(dictionary);
    channel->registerObject("spellCheckerBackend", bridge); // Match JavaScript object name
    webView->page()->setWebChannel(channel);

    // Load HTML content from file or string
    QFile htmlFile(":/html/index.html"); // Assuming HTML is in resources
    if (!htmlFile.open(QIODevice::ReadOnly)) {
        cerr << "Failed to load HTML file." << endl;
        webView->setHtml("<h1>Error: Could not load UI</h1>");
    } else {
        QString htmlContent = QString(htmlFile.readAll());
        webView->setHtml(htmlContent, QUrl("qrc:/html/index.html"));
    }

    // Set central widget and show
    window.setCentralWidget(webView);
    window.show();

    return app.exec();
}

#include "SymSpellChecker.moc"