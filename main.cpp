#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include "./utils.cpp"
#include "./thirdparty/ccflags.h"
using namespace std;
namespace fs = filesystem;

class FileConfig {
public:
    string           name;
    vector<fs::path> dependencies;
    vector<fs::path> include_dirs;
    vector<string>   linked;
    fs::path         output;
    string           compiler = "g++"; // g++ by default
    vector<string>   compiler_options;
    vector<string>   defines;
    vector<string>   cmds;
    bool             execute = true;   // by default, we execute config
    bool             autogen = true;
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
            
            // start new file config section
            if (is_section(line) && line.rfind("[config", 0) == 0) {
                this->file_configs.push_back(current_config);
                current_config = {};
                current_config.name = extract_config_name(line);
            }
            
            // if default section
            if (is_section(line)) {
                current_section = line;
                
                // single line sections
                if (current_section == "[no-execute]") {
                    current_config.execute = false;
                } else if (current_section == "[no-autogen]") {
                    current_config.autogen = false;
                }
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
            } else if (current_section == "[defines]") {
                current_config.defines.push_back(line);
            } else if (current_section == "[cmds]") {
                current_config.cmds.push_back(line);
            } else if (current_section == "[include]") {
                current_config.include_dirs.push_back(line);
            }
        }
        
        if (!current_config.name.empty())
            this->file_configs.push_back(current_config);
        
        // delete first blank element
        file_configs.erase(file_configs.begin());
    }
};

void generate_and_execute(FileConfig cfg) {
    string generated_cmd = "";
    
    if (cfg.autogen) {
        generated_cmd += cfg.compiler;
        
        generated_cmd += " -o ";
        generated_cmd += cfg.output;
    }
    
    for (auto dep : cfg.dependencies) {
        generated_cmd += " " + dep.string();
    }
    
    for (auto link : cfg.linked) {
        generated_cmd += " -l" + link;
    }
    
    for (auto flag : cfg.compiler_options) {
        generated_cmd += " " + flag;
    }
    
    for (auto define : cfg.defines) {
        generated_cmd += " " + define;
    }
    
    for (auto include : cfg.include_dirs) {
        generated_cmd += " -I" + include.string();
    }
    
    if (!generated_cmd.empty()) {
        zbs::log(generated_cmd);
        system(generated_cmd.c_str());
    }
    
    for (auto cmd : cfg.cmds) {
        if (!cmd.empty()) {
            zbs::log(cmd);
            system(cmd.c_str());
        }
    }
}

int main(int argc, char** argv) {
    // setup flags
    cc_setargs(argc, argv);
    cc_set_minimum_flags(0);
    
    // check if .bsconfig exists
    if (!fs::exists("./.zbsconfig")) {
        zbs::error("\".zbsconfig\" not found in current directory");
    }
    
    bsconfigParser parser;
    parser.parse_file();
    
    // parse flags
    if (cc_argexp("-c", "--config")) { // execute single file config by name
        for (auto i : parser.file_configs) {
            if (i.name == string(cc_getargexp("-c", "--config"))) {
                generate_and_execute(i);
                goto quit;
            }
        }
        zbs::error(string("config " + string(cc_getargexp("-c", "--config")) + " not found"));
    }
    
    // generate build commands and execute them
    for (auto i : parser.file_configs) {
        if (i.execute)
            generate_and_execute(i);
    }
    
 quit:
    
    return 0;
}
