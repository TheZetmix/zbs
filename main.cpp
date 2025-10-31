#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include "./utils.cpp"
using namespace std;
namespace fs = filesystem;

class FileConfig {
public:
    string           name;
    vector<fs::path> dependencies;
    vector<string>   linked;
    fs::path         output;
    string           compiler = "g++"; // g++ by default
    vector<string>   compiler_options;
};

class bsconfigParser {
public:
    vector<FileConfig> file_configs;
    
    bool is_section(string name) {
        return !name.empty() && name.front() == '[' && name.back() == ']';
    }
    
    string extract_config_name(string line) {
        auto start = line.find(' ') + 1;
        auto end   = line.find(']', start);
        return line.substr(start, end - start);
    }
    
    bool pkg_config_lib_installed(string name) {
        // "!" because pkg-config returns 0 if lib exists and 1 if not
        return !system(string("pkg-config --exists " + name).c_str());
    }
    
    void parse_file() {
        ifstream file("./.zbsconfig");
        string line;
        string current_section;
        
        FileConfig current_config;
        
        while (getline(file, line)) {
            // skip empty lines
            if (line.empty()) continue;
            
            // update current config name
            if (is_section(line) && line.rfind("[config", 0) == 0) {
                this->file_configs.push_back(current_config);
                current_config = {};
                current_config.name = extract_config_name(line);
            }
            
            // if default section
            if (is_section(line)) {
                current_section = line;
                continue;
            }
            
            if (current_section == "[output]") {
                current_config.output = line;
            } else if (current_section == "[sources]") {
                current_config.dependencies.push_back(fs::path(line));
            } else if (current_section == "[link]") {
                if (pkg_config_lib_installed(line))
                    current_config.linked.push_back(line);
                else zbs::error(string("lib \"" + line + "\" " + "(in " + current_config.name + ") not found, did you install it?"));
            } else if (current_section == "[compiler]") {
                current_config.compiler = line;
            } else if (current_section == "[flags]") {
                current_config.compiler_options.push_back(line);
            }
        }
        
        if (!current_config.name.empty())
            this->file_configs.push_back(current_config);
        
        // delete first blank element
        file_configs.erase(file_configs.begin());
    }
};

int main(int argc, char** argv) {
    // check if .bsconfig exists
    if (!fs::exists("./.zbsconfig")) {
        zbs::error("\".zbsconfig\" not found in current directory");
    }
    
    bsconfigParser parser;
    parser.parse_file();
    
    // generate build commands and execute them
    for (auto i : parser.file_configs) {
        string generated_cmd = "";
        
        generated_cmd += i.compiler;
        
        generated_cmd += " -o ";
        generated_cmd += i.output;
        
        for (auto dep : i.dependencies) {
            generated_cmd += " " + dep.string();
        }
        
        for (auto link : i.linked) {
            generated_cmd += " -l" + link;
        }
        
        zbs::log(generated_cmd);
        system(generated_cmd.c_str());
    }
    
    return 0;
}
