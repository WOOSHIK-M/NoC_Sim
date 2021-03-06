#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <typeinfo>
#include <iomanip>

#include "config_utils.hpp"

Configuration *Configuration::theConfig = 0;


Configuration::Configuration()
{
    theConfig = this;
    _config_file = 0;
}


void Configuration::AddStrField(string const & field, string const & value)
{
    _str_map[field] = value;
}


void Configuration::Assign(string const & field, string const & value)
{
    map<string, string>::const_iterator match;

    match = _str_map.find(field);
    
    if (match != _str_map.end()) {
        _str_map[field] = value;
    } else {
        ParseError("Unknown string field: " + field);
    }
}


void Configuration::Assign(string const & field, int value)
{
    map<string, int>::const_iterator match;
    
    match = _int_map.find(field);
    if (match != _int_map.end()) {
        _int_map[field] = value;
    } else {
        ParseError("Unknown int field: " + field);
    }
}


void Configuration::Assign(string const & field, double value)
{
    map<string, double>::const_iterator match;

    match = _float_map.find(field);
    if (match != _float_map.end()) {
        _float_map[field] = value;
    } else {
        ParseError("Unknown float field: " + field);
    }
}


string Configuration::GetStr(string const & field) const
{
    map<string, string>::const_iterator match;

    match = _str_map.find(field);
    if(match != _str_map.end()) {
        return match->second;
    } else {
        ParseError("Unknown string field: " + field);
        exit(-1);
    }
}


int Configuration::GetInt(string const & field) const
{
    map<string, int>::const_iterator match;

    match = _int_map.find(field);
    if(match != _int_map.end()) {
        return match->second;
    } else {
        ParseError("Unknown integer field: " + field);
        exit(-1);
    }
}


double Configuration::GetFloat(string const & field) const
{  
    map<string,double>::const_iterator match;

    match = _float_map.find(field);
    if(match != _float_map.end()) {
        return match->second;
    } else {
        ParseError("Unknown double field: " + field);
        exit(-1);
    }
}


void Configuration::ParseError(string const & msg, unsigned int lineno) const
{
    if(lineno) {
        cerr << "Parse error on line " << lineno << " : " << msg << endl;
    } else {
        cerr << "Parse error : " << msg << endl;
    }


    exit( -1 );
}


void ParseArgs(Configuration * cf, string const & filepath)
{   
    ifstream rf;
    rf.open(filepath);
    
    if (!rf.is_open()) {
        cerr << "Invalid Config File ... [" << filepath << "]" << endl;
        exit(-1);
    } else {
        cerr << endl<< setw(50) << setfill('#') << " " << endl;
        cerr << "=> BEGIN Configuration File : [" << filepath << "]" << endl;
        cerr << setw(50) << setfill('#') << " " << endl << endl;

        string line;
        int typeflag = 0;
        while (getline(rf, line)) {
            cerr << line << endl;

            // strip spaces and empty lines
            line.erase(remove(line.begin(), line.end(), ' '), line.end());
            line.erase(remove(line.begin(), line.end(), '\t'), line.end());
            line.erase(remove(line.begin(), line.end(), '\n'), line.end());

            // ignore comment lines
            if (line[0] == '#' || line.empty())
                continue;

            // seek for '='
            int iter = 0;
            while (iter < line.size() && line[iter] != '=' ) {
                iter++;
            }
            if (iter >= line.size() - 1)
                continue;

            // the string before '=' is key, and the string after '=' is value
            string key = line.substr(0, iter);
            string value = line.substr(iter + 1, line.size() - iter - 1);
            
            // update params
            if (value[typeflag] > 47 && value[typeflag] < 58) {     // ASCII Code (Number)
                string::iterator it;
                string ptr(".");
                if (value.find(ptr) == string::npos) {
                    cf->Assign(key, stoi(value));           // int value
                } else {
                    cf->Assign(key, stof(value));           // float value
                }
            } else {
                cf->Assign(key, value);                     // string value
            }
        }
        rf.close();
    }
}


void ParseLUTs(vector<vector<int>> & lut, string const & filepath)
{   
    ifstream rf;
    rf.open(filepath);

    if(!rf.is_open()) {
		cerr << "Invalid LUT filename: ." << filepath << endl;
		exit(-1);
	} else {
        cerr << endl<< setw(50) << setfill('#') << " " << endl;
        cerr << "=> BEGIN READ Look Up Table : [" << filepath << "]" << endl;
        cerr << setw(50) << setfill('#') << " " << endl << endl;

        for (int i=0;i<(int)lut[0].size();i++) {
            string line;
            getline(rf,line);
            istringstream line_in(line);
            for (int j=0;j<(int)lut.size();j++) {
                double temp;
                line_in >> temp;
                if(bool(line_in)) {
                    lut[j][i] = static_cast<int>(temp) - 1;
                }
            }
        }

        cerr << "Parsing Finished ... " << endl;
    }

    // ===== Print Parsing result =====
    // for (int i=0;i<(int)lut.size();i++) {
    //     for (int j=0;j<(int)lut[0].size();j++){
    //         cerr << lut[i][j] << " ";
    //     }
    //     cerr << endl;   
    // }
    // exit(-1);
}
