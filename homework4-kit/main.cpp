#include <iostream>
#include <pqxx/pqxx>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=acc_bball user=postgres password=passw0rd");
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
  executeSQLFile(C, "table.sql");
  insertFromFile(C, "color.txt", "color", {"NAME"});
  insertFromFile(C, "state.txt", "state", {"NAME"});
  insertFromFile(C, "team.txt", "team", {"NAME", "STATE_ID", "COLOR_ID", "WINS", "LOSSES"});
  insertFromFile(C, "player.txt", "player", {"TEAM_ID", "UNIFORM_NUM", "FIRST_NAME", "LAST_NAME", "MPG", "PPG", "RPG", "APG", "SPG", "BPG"});


  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


