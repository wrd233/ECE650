#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "query_funcs.h"

using namespace std;

static std::string wrapQuotes(const string& input) {
    string res = "'" + input + "'";
    return res;
}


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
    string SQL = "SELECT * FROM PLAYER WHERE";
    vector<string> whereStatement;
    if (use_mpg != 0) {
        whereStatement.push_back("MPG <= " + std::to_string(max_mpg) + " AND " + "MPG >= " + std::to_string(min_mpg));
    }
    if (use_ppg != 0) {
        whereStatement.push_back("PPG <= " + std::to_string(max_ppg) + " AND " + "PPG >= " + std::to_string(min_ppg));
    }
    if (use_rpg != 0) {
        whereStatement.push_back("RPG <= " + std::to_string(max_rpg) + " AND " + "RPG >= " + std::to_string(min_rpg));
    }
    if (use_apg != 0) {
        whereStatement.push_back("APG <= " + std::to_string(max_apg) + " AND " + "APG >= " + std::to_string(min_apg));
    }
    if (use_spg != 0) {
        whereStatement.push_back("SPG <= " + std::to_string(max_spg) + " AND " + "SPG >= " + std::to_string(min_spg));
    }
    if (use_bpg != 0) {
        whereStatement.push_back("BPG <= " + std::to_string(max_bpg) + " AND " + "BPG >= " + std::to_string(min_bpg));
    }
    if (whereStatement.empty()) {
        // 此时每个都无效，直接返回
        return;
    }

    // 组装SQL
    SQL += " " + whereStatement[0];
    for (size_t i = 1; i < whereStatement.size(); i++) {
        SQL += " AND " + whereStatement[i];
    }
    SQL += ";";

    // cout<<"query1: "<<SQL<<endl;
    
    nontransaction N(*C);
    result res;
    res = N.exec(SQL);
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG" << endl;
    for (const auto& row : res) {
        cout << row[0].as<int>() << " " 
            << row[1].as<int>() << " " 
            << row[2].as<int>() << " "
            << row[3].as<string>() << " " 
            << row[4].as<string>() << " " 
            << row[5].as<int>() << " "
            << row[6].as<int>() << " " 
            << row[7].as<int>() << " " 
            << row[8].as<int>() << " "
            << fixed << setprecision(1) << row[9].as<double>() << " " 
            << row[10].as<double>()
            << endl;
    }
}


void query2(connection *C, string team_color)
{
    string SQL = "SELECT TEAM.NAME "
                "FROM TEAM, COLOR "
                "WHERE TEAM.COLOR_ID = COLOR.COLOR_ID AND "
                "COLOR.NAME = ";
    SQL += wrapQuotes(team_color) + ";";

    // cout<<"query2: "<<SQL<<endl;

    nontransaction N(*C);
    result res;
    res = N.exec(SQL);

    cout << "NAME" << endl;
    for(const auto& row : res){
        cout << row[0].c_str() << endl;
    }

}


void query3(connection *C, string team_name)
{
    string SQL = "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM "
                "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
                "TEAM.NAME = ";
    SQL += wrapQuotes(team_name) + " ORDER BY PPG DESC;";        

    // cout<<"query3: "<<SQL<<endl;
    
    nontransaction N(*C);
    result res;
    res = N.exec(SQL);

    cout << "FIRST_NAME LAST_NAME" << endl;
    for (const auto& row : res) {
        cout << row[0].c_str() << " " << row[1].c_str() << endl;
    }
}


void query4(connection *C, string team_state, string team_color)
{
    string SQL = "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, STATE, COLOR, TEAM "
                "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
                "TEAM.COLOR_ID = COLOR.COLOR_ID AND "
                "TEAM.STATE_ID = STATE.STATE_ID AND ";
    SQL += "STATE.NAME = " + wrapQuotes(team_state) + " AND ";
    SQL += "COLOR.NAME = " + wrapQuotes(team_color) + ";";

    // cout<<"query4: "<<SQL<<endl;

    nontransaction N(*C);
    result res;
    res = N.exec(SQL);

    cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl;
    for (const auto& row : res) {
        cout << row[0].c_str() << " " << row[1].c_str() << " " << row[2].c_str() << endl;
    }
}


void query5(connection *C, int num_wins)
{
    string SQL = "SELECT FIRST_NAME, LAST_NAME, NAME, WINS FROM PLAYER, TEAM WHERE "
                "PLAYER.TEAM_ID = TEAM.TEAM_ID AND ";
    SQL += "WINS > " + to_string(num_wins) + ";";

    // cout<<"query5: "<<SQL<<endl;

    nontransaction N(*C);
    result res;
    res = N.exec(SQL);

    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
    for (const auto& row : res) {
        cout << row[0].c_str() << " " << row[1].c_str() << " " << row[2].c_str() << " " << row[3].c_str() << endl;
    }
}

void executeOneSQL(connection *C, string SQL)
{
    work W(*C);
    W.exec(SQL);
    W.commit();
}

void executeSQLStatements(connection *C, const string &statement){
    stringstream stateBuffer(statement);
    stringstream buffer;
    string line;

    while (getline(stateBuffer, line)) {
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

}

void dropTable(connection *C, string tableName)
{
    string sql = "DROP TABLE IF EXISTS " + tableName + " CASCADE;";
    executeOneSQL(C,sql);
    // cout << "Drop table[" + tableName + "]" << endl;  
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
