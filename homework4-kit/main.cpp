#include <iostream>
#include <pqxx/pqxx>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

const string SQLStatements =
    "CREATE TABLE STATE("
    "    STATE_ID SERIAL,"
    "    NAME VARCHAR(256),"
    "    PRIMARY KEY (STATE_ID)"
    ");"
    ""
    "CREATE TABLE COLOR("
    "    COLOR_ID SERIAL,"
    "    NAME VARCHAR(256),"
    "    PRIMARY KEY (COLOR_ID)"
    ");"
    ""
    "CREATE TABLE TEAM("
    "    TEAM_ID SERIAL,"
    "    NAME VARCHAR(256),"
    "    STATE_ID INT,"
    "    COLOR_ID INT,"
    "    WINS INT,"
    "    LOSSES INT,"
    "    PRIMARY KEY (TEAM_ID),"
    "    FOREIGN KEY (STATE_ID) REFERENCES STATE(STATE_ID) ON DELETE SET NULL ON UPDATE CASCADE,"
    "    FOREIGN KEY (COLOR_ID) REFERENCES COLOR(COLOR_ID) ON DELETE SET NULL ON UPDATE CASCADE"
    ");"
    ""
    "CREATE TABLE PLAYER("
    "    PLAYER_ID SERIAL,"
    "    TEAM_ID INT,"
    "    UNIFORM_NUM INT,"
    "    FIRST_NAME VARCHAR(256),"
    "    LAST_NAME VARCHAR(256),"
    "    MPG INT,"
    "    PPG INT,"
    "    RPG INT,"
    "    APG INT,"
    "    SPG DOUBLE PRECISION,"
    "    BPG DOUBLE PRECISION,"
    "    PRIMARY KEY (PLAYER_ID),"
    "    FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID) ON DELETE SET NULL ON UPDATE CASCADE"
    ");";


int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  dropTable(C, "PLAYER");
  dropTable(C, "TEAM");
  dropTable(C, "STATE");
  dropTable(C, "COLOR");
  executeSQLStatements(C,SQLStatements);
  insertFromFile(C, "color.txt", "color", {"NAME"});
  insertFromFile(C, "state.txt", "state", {"NAME"});
  insertFromFile(C, "team.txt", "team", {"NAME", "STATE_ID", "COLOR_ID", "WINS", "LOSSES"});
  insertFromFile(C, "player.txt", "player", {"TEAM_ID", "UNIFORM_NUM", "FIRST_NAME", "LAST_NAME", "MPG", "PPG", "RPG", "APG", "SPG", "BPG"});


  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


