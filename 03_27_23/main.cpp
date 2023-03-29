#include "sqlite3.h"
#include <iostream>
#include <string>
#include <ctime>
#include <climits>

int callback(void *, int, char **, char **);
void viewAssignmentsByProject(sqlite3 *);
void resetStream();

int main()
{
    sqlite3 *db;
    int rc;
    rc = sqlite3_open_v2("ConstructCo.db", &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 0;
    }
    else
    {
        std::cout << "Database opened successfully." << std::endl;
    }
    std::string query = "select * from employee"; //"select emp_num as 'Employee Number', emp_fname || ' ' || emp_lname as 'Employee Name' from employee";
    char *err;
    rc = sqlite3_exec(db, query.c_str(), callback, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error - select callback: " << err << std::endl;
        std::cout << query << std::endl;
        sqlite3_free(err);
    }
    query = "delete from employee where emp_lname = 'Brown' and emp_fname = 'Charlie'";
    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error - delete: " << err << std::endl;
        std::cout << query << std::endl;
        sqlite3_free(err);
    }

    std::string lname, fname, mi;
    int job, years;
    char formatDate[80];
    time_t currentDate = time(NULL);
    strftime(formatDate, 80, "%F", localtime(&currentDate)); // for date and time "%F %T"
    std::string hiredate(formatDate);
    lname = "Brown";
    fname = "Charlie";
    job = 504;
    years = 0;

    query = "insert into employee(emp_lname, emp_fname, emp_hiredate, job_code, emp_years) ";
    query += "values ('";
    query += lname + "', '";
    query += fname + "', '";
    query += hiredate + "', ";
    query += std::to_string(job) + ", ";
    query += std::to_string(years) + ");";

    rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error - insert: " << err << std::endl;
        std::cout << query << std::endl;
        sqlite3_free(err);
    }
    else
    {
        int emp_num = sqlite3_last_insert_rowid(db);
        std::cout << fname << " " << lname << " inserted into the database as employee number " << emp_num << std::endl;
    }
    viewAssignmentsByProject(db);
    sqlite3_close(db);
    return 0;
}

int callback(void *extData, int numCols, char **values, char **colNames)
{
    for (int i = 0; i < numCols; i++)
    {
        std::cout << colNames[i] << ": ";
        if (values[i] != NULL)
            std::cout << values[i];
        std::cout << std::endl;
    }
    std::cout << std::endl;
    return SQLITE_OK;
}

void viewAssignmentsByProject(sqlite3 *db)
{
    std::string query;
    sqlite3_stmt *result;
    std::string strLastError;
    query = "select proj_num, proj_name from project";
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(result);
        std::cout << "There was and error with the project query: " << strLastError << std::endl;
        std::cout << query << std::endl;
        return;
    }
    int columnCount = sqlite3_column_count(result);
    int i, choice;
    std::cout << std::left;
    std::cout << "Please choose the project you want to see the assignments for:" << std::endl;
    i = 1;
    do
    {
        rc = sqlite3_step(result);
        if (rc == SQLITE_DONE)
            break;
        std::cout << i << ". " << sqlite3_column_text(result, 0);
        std::cout << " - " << sqlite3_column_text(result, 1);
        std::cout << std::endl;
        i++;
    } while (rc == SQLITE_ROW);
    std::cin >> choice;
    while (!std::cin || choice < 1 || choice >= i)
    {
        if (!std::cin)
            resetStream();
        std::cout << "That is not a valid choice! Try again!" << std::endl;
        std::cin >> choice;
    }
    sqlite3_reset(result);
    for (int j = 0; j < choice; j++)
    {
        sqlite3_step(result);
    }
    int proj_num = sqlite3_column_int(result, 0);
    // std::string projNum = reinterpret_cast<const char*>(sqlite3_column_text(result, 0));
    sqlite3_finalize(result);
    query = "select sum(assign_hours) as 'Total Hours', sum(assign_charge) as 'Total Charges' ";
    query += "from assignment ";
    query += "where proj_num = " + std::to_string(proj_num);
    query += " group by proj_num";
    std::cout << query << std::endl;
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(result);
        std::cout << "There was and error with the assignment query: " << strLastError << std::endl;
        std::cout << query << std::endl;
        return;
    }
    columnCount = sqlite3_column_count(result);
    rc = sqlite3_step(result);
    while (rc == SQLITE_ROW)
    {
        for (int i = 0; i < columnCount; i++)
        {
            std::cout << sqlite3_column_name(result, i) << " - ";
            if (sqlite3_column_type(result, i) != SQLITE_NULL)
            {
                std::cout << sqlite3_column_text(result, i);
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        rc = sqlite3_step(result);
    }
    sqlite3_finalize(result);
}
void resetStream()
{
    std::cin.clear();
    std::cin.ignore(INT_MAX, '\n');
    std::cout << "You entered something that is not a number." << std::endl;
}
