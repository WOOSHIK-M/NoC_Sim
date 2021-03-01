#ifndef _CONFIG_UTILS_HPP_
#define _CONFIG_UTILS_HPP_

#include <string>
#include <map>
#include <cstdio>
#include <vector>

#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

using namespace std;

class Configuration {
    static Configuration * theConfig;
    FILE * _config_file;
    string _config_string;

protected:
    map<string, string> _str_map;
    map<string, int> _int_map;
    map<string, double> _float_map;

public:
    Configuration();

    void AddStrField(string const & field, string const & value);

    void Assign(string const & field, string const & value);
	void Assign(string const & field, int value);
	void Assign(string const & field, double value);

    string GetStr(string const & field) const;
    int GetInt(string const & field) const;
    double GetFloat(string const & field) const;

    void ParseError(string const & msg, unsigned int linene = 0) const;
};

void ParseArgs(Configuration * cf, string const & filepath);
void ParseLUTs(vector<vector<int>> & lut, string const & filepath);

#endif