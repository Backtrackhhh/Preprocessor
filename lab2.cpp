#include <iostream>
#include <fstream>
#include <string> 
#include <vector>
#include <map>
#include <stack>
#include <regex>

using namespace std;

class Node {
private:
    string value;
    string argu;
public:
    Node(string value,string argu);
    string get_value();
    string get_argu();
};

class Preprocessor {
private:
    string pre_code;
    string new_code;
    string sign;
    int which_cmd(const string &line);
    void handle_include(string line);
    static string include_file(const string& path);
    string replace_define(string line);
    map<string,Node> defines;
    void handle_common(string line);
    void handle_define(string line);
    void handle_endif(string line);
    void handle_else(string line);
    void handle_if(string line);
    void handle_ifdef(string line);
    void handle_ifndef(string line);
    void handle_undef(string line);
    void handle(const string& line, const struct handle_struct g_handle_methods[]);
    void handles(const string &line);
    stack<bool> status; //use stack to store infomation about nested if-else
public:
    explicit Preprocessor(string code);
    ~Preprocessor();
    string preprocessor();
};



void split_string(string &string1, vector<string> &container, string &signs);
string trim(string &s);
bool start_with(const string& target, string line);

string get_unprocessed_code(int number);

void put_processed_code(int number, string code);

void run_test(int test_case_number);

 
 string Node::get_argu() {
    return argu;
}

string Node::get_value() {
    return value;
}

Node::Node(string value,string argu){
    this->value = move(value);
    this->argu = move(argu);
}


string replaceSingleSlashToDoubleSlahes(const string &strWithSingleSlash) {
    //将单斜杠路径(/)同一替换成双斜杠(//)路径分隔符
    string mTempStr = strWithSingleSlash;
    for (unsigned int i = 0; i < mTempStr.length(); i++) {
        if (mTempStr[i] == '\\') {
            mTempStr.replace(i, 1, "\\\\");
            ++i;
        }
    }
    return mTempStr;
}


string trim(string &s) {
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
    return s;
}

bool start_with(const string& target, string line){
    if (target == "#") {
        regex regex2("^" + target);
        return regex_search(line, regex2);
    }
    trim(line);
    string pattern("^\\b" + target + "\\b");
    regex regex1(pattern);
    return regex_search(line, regex1);
}

void split_string(string &s, vector<string> &v, string &c) {   //split given string with sign 
    string::size_type pos1, pos2;
    pos2 = s.find_first_of(c);
    pos1 = 0;
    while (string::npos != pos2) {
        if ((pos2 - pos1) != 0u)
            v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
        pos2 = s.find_first_of(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}



int main() {
    for (int test_case_number = 1; test_case_number <= 2; test_case_number++) {
        run_test(test_case_number);
    }
    return 0;
}

void run_test(int test_case_number) {
    string raw_code = get_unprocessed_code(test_case_number);
    Preprocessor prep(raw_code);
    string processed_code = prep.preprocessor();
    put_processed_code(test_case_number, processed_code);
}

string get_unprocessed_code(int number) {
    string filename = "test/test" + to_string(number) + ".cpp";
    string file;
    ifstream is(filename);
    if (!is.is_open()) {
        cout << "Broken input file.";
    } else {
        string line;
        while (getline(is, line)) {
            file.append(line).push_back('\n');
        }
        is.close();
    }
    return file;
}

void put_processed_code(int number, string code) {
    string filename = "test/test" + to_string(number) + ".out.cpp";
    ofstream os(filename);
    if (!os.is_open()) {
        cout << "Broken output file.";
    } else {
        os << code;
        os.close();
    }
}


string cmds[8] = {//precmd
        "define",  //1
        "undef",   //2
        "if",      //3
        "ifdef",   //4
        "ifndef",  //5
        "else",    //6
        "endif",   //7
        "include"  //8
};

int Preprocessor::which_cmd(const string &line) {
    int i = 0;
    for(const string& cmd: cmds){
        i++;
        if(start_with("#",line) && start_with(cmd,line.substr(line.find_first_of('#') + 1))){
            return i;
        }
    }
    return 0;//replace define
}

//constructor function
Preprocessor::Preprocessor(string code) {
    this->pre_code = move(code);
    this->new_code = "";
    this->sign = " \t\n\r\f#";
    this->status.push(true);
}

void Preprocessor::handle_include(string line) {
    if(!status.top()){
        return;
    } 
    if(line.find('<') != string::npos || line.find(".h") == string::npos){
        new_code.append(line).push_back('\n');
        return;
    }
    string path("test/");
    int start_index = line.find_first_of('"');
    int end_index = line.find_first_of('"',start_index + 1);
    path.append(line.substr(start_index + 1,end_index - start_index - 1));
    string file;
    ifstream is(path);
    string line1;
    while (getline(is, line1)) {
        file.append(line1).push_back('\n');
    }
    is.close();
    new_code.append(file);
}

//There maybe two types of define
string Preprocessor::replace_define(string line) {
    string tmp = line;
    for (auto & define : defines) {
        string key = define.first;
        Node node = define.second;
        if (!node.get_argu().empty()) {
            int pos1;
            if ((pos1 = tmp.find(key)) != string::npos) {
                int pos2 = tmp.find_first_of('(', pos1);
                int pos3 = tmp.find_first_of(')', pos2);
                string arg = tmp.substr(pos2 + 1, pos3 - pos2 - 1);
                string tmpArg = node.get_argu();
                string value = node.get_value();
                regex regex1("\"#" + tmpArg);
                value = regex_replace(value, regex1, arg + "\"");
                regex regex2(tmpArg);
                value = regex_replace(value, regex2, arg);
                if(value.find("##") != string::npos){
                    int pos = value.find("##");
                    string start = value.substr(0,pos);
                    string end = value.substr(pos + 2);
                    value = trim(start) + trim(end);
                }
                int index = tmp.find(key);
                int index2 = tmp.find(')', index);
                tmp.replace(index, index2 - index + 1, "");
                tmp.insert(index, value);
            }
        } else if(!node.get_value().empty()){
            regex re("\\b" + key + "\\b");
            tmp = regex_replace(tmp, re, node.get_value());
        }

    }
    return tmp;

}

void Preprocessor::handle_define(string line) {
    if(!status.top()){
        return;
    }
    trim(line);

    if(line.find('(') == string::npos || line.find_first_of(')') == line.size() - 1){
        vector<string> strings;
        split_string(line,strings,sign);
        string value = line.substr(line.find_first_of(strings[1]) + strings[1].length());
        Node node(trim(value),"");//no arguments
        defines.insert(make_pair(strings[1],node));

    }else{
        string sign1("()");
        vector<string> strings;
        split_string(line,strings,sign1);
        string value;
        value = strings[2];
        if(strings.size() >= 4){
            value = strings[3];
        }
        Node node(value,trim(strings[1]));
        vector<string> strings1;
        split_string(strings[0],strings1,sign);
        defines.insert(make_pair(strings1[1],node));
    }
}


void Preprocessor::handle_endif(string line) {
    status.pop();
}

void Preprocessor::handle_if(string line) {
    vector<string> strings;
    split_string(line,strings,sign);
    status.push(replace_define(strings[1]) == "1" && status.top());
}

void Preprocessor::handle_else(string line) {
    bool latest_status = status.top();
    status.pop();
    status.push((!latest_status) && status.top());
}

void Preprocessor::handle_ifdef(string line) {
    vector<string> strings;
    split_string(line,strings,sign);
    status.push(defines.count(strings[1]) != 0  && status.top());
}

void Preprocessor::handle_ifndef(string line) {
    vector<string> strings;
    split_string(line,strings,sign);
    status.push(defines.count(strings[1]) == 0 && status.top());
}


void Preprocessor::handle_undef(string line) {
    if(!status.top()){
        return;
    }
    vector<string> strings;
    split_string(line,strings,sign);
    defines.erase(strings[1]);
}

//replace define
void Preprocessor::handle_common(string line) {
    if(!status.top()){
        return;
    }
    new_code.append(replace_define(std::move(line))).push_back('\n');
}

//handles function calls defferent types of handle

void Preprocessor::handles(const string &line) {
    int type = which_cmd(line);
    switch (type) {
        case 0:
            handle_common(line);
            break;
        case 1:
            handle_define(line);
            break;
        case 2:
            handle_undef(line);
            break;
        case 3:
            handle_if(line);
            break;
        case 4:
            handle_ifdef(line);
            break;
        case 5:
            handle_ifndef(line);
            break;
        case 6:
            handle_else(line);
            break;
        case 7:
            handle_endif(line);
            break;
        case 8:
            handle_include(line);
            break;
        default:
            break;
    }
}


string Preprocessor::preprocessor() {
    vector<string> lines;
    string sign = "\n";
    split_string(pre_code, lines, sign); 
    for (auto line :lines) {
        handles(line);
    }
    return new_code;
}

Preprocessor::~Preprocessor() = default;
