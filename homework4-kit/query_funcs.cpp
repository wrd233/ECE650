#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "query_funcs.h"

using namespace std;


void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    stringstream SQL;
    SQL << "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) VALUES (";
    SQL << team_id << ", " 
        << jersey_num << ", " 
        << escapeSingleQuotes(first_name) << ", "
        << escapeSingleQuotes(last_name) << ", " 
        << mpg << ", " 
        << ppg << ", " 
        << rpg << ", " 
        << apg << ", " 
        << spg << ", " 
        << bpg << ");";

    executeOneSQL(C, SQL.str());

}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    stringstream SQL;
    SQL << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES (";
    SQL << escapeSingleQuotes(name) << ", " 
        << state_id << ", " 
        << color_id << ", " 
        << wins << ", "
        << losses << ");";
    executeOneSQL(C, SQL.str());
}


void add_state(connection *C, string name)
{
    stringstream SQL;
    SQL << "INSERT INTO STATE (NAME) VALUES (";
    SQL << escapeSingleQuotes(name) << ");";
    executeOneSQL(C, SQL.str());
}


void add_color(connection *C, string name)
{
    stringstream SQL;
    SQL << "INSERT INTO STATE (NAME) VALUES (";
    SQL << escapeSingleQuotes(name) << ");";
    executeOneSQL(C, SQL.str());
}


void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
}


void query2(connection *C, string team_color)
{
}


void query3(connection *C, string team_name)
{
}


void query4(connection *C, string team_state, string team_color)
{
}


void query5(connection *C, int num_wins)
{
}

void executeOneSQL(connection *C, string SQL)
{
    work W(*C);
    W.exec(SQL);
    W.commit();
}

void executeSQLFile(connection *C, string fileName)
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << fileName << endl;
        return;
    }

    stringstream buffer;
    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        buffer << line;

        // 读到分号为一句SQL
        if (line.find(';') != string::npos) {
            executeOneSQL(C, buffer.str());
            buffer.str("");
        }
    }

    file.close();
}

void dropTable(connection *C, string tableName)
{
    string sql = "DROP TABLE IF EXISTS " + tableName + " CASCADE;";
    executeOneSQL(C,sql);
    cout << "Drop table[" + tableName + "]" << endl;  
}

void insertFromFile(connection *C, const string &fileName, const string &tableName, const vector<string> &columns) 
{
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << fileName << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string seqNum; // 第一个元素是序号
        getline(ss, seqNum, ' '); // 丢弃第一个元素
        vector<string> values;
        string value;
        while (getline(ss, value, ' ')) {
            // 将单引号替换为两个单引号
            values.push_back(escapeSingleQuotes(value));
        }

        if (values.size() != columns.size()) {
            cerr << "列元素数量不匹配" << endl;
            return;
        }

        stringstream SQL;
        SQL << "INSERT INTO " << tableName << " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            SQL << columns[i];
            if (i != columns.size() - 1) {
                SQL << ", ";
            }
        }
        SQL << ") VALUES (";
        for (size_t i = 0; i < values.size(); ++i) {
            SQL << "'" << values[i] << "'";
            if (i != values.size() - 1) {
                SQL << ", ";
            }
        }
        SQL << ");";

        // Execute the INSERT statement
        executeOneSQL(C, SQL.str());
    }
    file.close();
}

string escapeSingleQuotes(const string& str) {
    string escapedStr;
    for (char ch : str) {
        if (ch == '\'') {
            escapedStr += "''";
        } else {
            escapedStr += ch;
        }
    }
    return escapedStr;
}