#include <iostream>
#include <string>
#include <iomanip>
#include <climits>
#include <ctime>
#include <cctype>
#include <regex>
#include "sqlite3.h"

void printMainMenu();
void viewInvoice(sqlite3 *);
void viewCustomer(sqlite3 *);
void addInvoice(sqlite3 *);
int mainMenu();
int rollback(sqlite3 *, std::string);
int commit(sqlite3 *);
int beginTransaction(sqlite3 *);

int main()
{
    int choice;
    sqlite3 *db;
    int rc = sqlite3_open_v2("SaleCo.db", &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error in connection: " << sqlite3_errmsg(db);
        return 0;
    }
    std::cout << "Welcome to SaleCo" << std::endl;
    choice = mainMenu();
    while (true)
    {
        switch (choice)
        {
        case 1:
            viewInvoice(db);
            break;
        case 2:
            viewCustomer(db);
            break;
        case 3:
            addInvoice(db);
            break;
        case 4:
            // addCustomer(mydb);
            break;
        case -1:
        {
            // don't forget to close.
            sqlite3_close(db);
            return 0;
        }
        default:
            std::cout << "That is not a valid choice." << std::endl;
        }
        std::cout << "\n\n";
        choice = mainMenu();
    }

    return 0;
}

void printMainMenu()
{
    std::cout << "Please choose an option (enter -1 to quit):  " << std::endl;
    std::cout << "1. View an invoice" << std::endl;
    std::cout << "2. View Customer Information" << std::endl;
    std::cout << "3. Add an invoice" << std::endl;
    std::cout << "4. Add a Customer" << std::endl;
    std::cout << "Enter Choice: ";
}

int mainMenu()
{
    int choice = 0;

    printMainMenu();
    std::cin >> choice;
    while ((!std::cin || choice < 1 || choice > 4) && choice != -1)
    {
        if (!std::cin)
        {
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
        }
        std::cout << "That is not a valid choice." << std::endl
                  << std::endl;
        printMainMenu();
        std::cin >> choice;
    }
    return choice;
}

int rollback(sqlite3 *db, std::string query)
{
    std::string error = sqlite3_errmsg(db);
    std::cout << "There was an error: " << error << std::endl;
    std::cout << query << std::endl;
    char *err;
    std::string rollbackQuery = "rollback";
    int rc = sqlite3_exec(db, rollbackQuery.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error on rollback: " << err << std::endl;
        sqlite3_free(err);
    }
    return rc;
}

int commit(sqlite3 *db)
{
    char *err;
    std::string commitQuery = "commit";
    int rc = sqlite3_exec(db, commitQuery.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error on commit: " << err << std::endl;
        sqlite3_free(err);
    }
    return rc;
}

int beginTransaction(sqlite3 *db)
{
    char *err;
    std::string query = "begin transaction";
    int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error on begin transaction: " << err << std::endl;
        sqlite3_free(err);
    }
    return rc;
}

void viewInvoice(sqlite3 *db)
{
    std::string query = "SELECT INVOICE.INV_NUMBER, INVOICE.INV_DATE, CUSTOMER.CUS_FNAME, CUSTOMER.CUS_LNAME ";
    query += "FROM INVOICE JOIN CUSTOMER ON INVOICE.CUS_CODE = CUSTOMER.CUS_CODE;";
    sqlite3_stmt *pRes;
    std::string m_strLastError;
    std::string query2;
    std::string inv_number;
    std::string inv_date;
    std::string cus_fname, cus_lname;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        m_strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        std::cout << "There was an error: " << m_strLastError << std::endl;
        return;
    }
    else
    {
        std::cout << "Please choose the invoice you want to see:" << std::endl;
        int columnCount = sqlite3_column_count(pRes);
        int i = 1, choice;
        sqlite3_stmt *pRes2;
        std::cout << std::left;
        while (sqlite3_step(pRes) == SQLITE_ROW)
        {
            std::cout << i << ". " << sqlite3_column_text(pRes, 0);
            std::cout << std::endl;
            i++;
        }

        // Fixed a bug where going outside the bounds of the choices would NOT
        // stop the program from proceeding and would crash with a segmentation fault.
        while (!(std::cin >> choice) || choice < 1 || choice > (i - 1))
        {
            if (!std::cin)
            {
                std::cin.clear();
                std::cin.ignore(INT_MAX, '\n');
            }
            std::cout << "That is not a valid choice! Try again!" << std::endl;
        }

        sqlite3_reset(pRes);
        for (int i = 0; i < choice; i++)
            sqlite3_step(pRes);
        inv_number = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
        inv_date = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 1));
        cus_fname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));
        cus_lname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 3));
        sqlite3_finalize(pRes);
        query2 = "SELECT PRODUCT.P_DESCRIPT as Product ,LINE.LINE_PRICE as Price, LINE.LINE_UNITS as Units ";
        query2 += "FROM LINE ";
        query2 += "JOIN PRODUCT on line.P_CODE = PRODUCT.P_CODE  ";
        query2 += "WHERE LINE.INV_NUMBER = '" + inv_number + "';";

        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
        {
            m_strLastError = sqlite3_errmsg(db);
            sqlite3_finalize(pRes2);
            std::cout << "There was an error: " << m_strLastError << std::endl;
            return;
        }
        else
        {
            std::cout << "Invoice #: " << inv_number << std::endl;
            std::cout << "Invoice Date: " << inv_date << std::endl;
            std::cout << "Customer: " << cus_fname << " " << cus_lname << std::endl;
            columnCount = sqlite3_column_count(pRes2);
            std::cout << std::left;
            std::cout << "|" << std::setw(83) << std::setfill('-') << "-"
                      << "|" << std::endl;
            std::cout << std::setfill(' ');
            for (int i = 0; i < columnCount; i++)
            {
                std::cout << "|" << std::setw(27) << sqlite3_column_name(pRes2, i);
            }
            std::cout << "|" << std::endl;
            std::cout << "|" << std::setw(83) << std::setfill('-') << "-"
                      << "|" << std::endl;
            std::cout << std::setfill(' ');

            while (sqlite3_step(pRes2) == SQLITE_ROW)
            {
                for (int i = 0; i < columnCount; i++)
                {
                    if (sqlite3_column_type(pRes2, i) != SQLITE_NULL)
                        std::cout << "|" << std::setw(27) << sqlite3_column_text(pRes2, i);
                    else
                        std::cout << "|" << std::setw(27) << " ";
                }
                std::cout << "|" << std::endl;
                std::cout << "|" << std::setw(83) << std::setfill('-') << "-"
                          << "|" << std::endl;
                std::cout << std::setfill(' ');
            }
            sqlite3_finalize(pRes2);
        }
    }
}

void viewCustomer(sqlite3 *db)
{

    std::string cusID, firstName, lastName, initial, phoneNum, areaCode, balance;
    balance = "$";

    // Note I decided to use query std::string concatenation to simplify the programatic process
    // And also grab the customer code by itself, for use in the second query.
    std::string query = "SELECT CUS_CODE || \" - \" || CUS_LNAME || \", \" || CUS_FNAME AS 'CUSTOMER', CUS_CODE FROM CUSTOMER ORDER BY CUS_CODE ASC";
    sqlite3_stmt *firstStatement;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &firstStatement, NULL) != SQLITE_OK)
    {
        // When an error occurs preparing the first statement...
        std::string err = sqlite3_errmsg(db);
        sqlite3_finalize(firstStatement);
        std::cout << "There was an error: " << err << std::endl;
        return;
    }
    else
    {
        std::cout << "Please choose the customer you want to see:\n";
        int choice;
        int i = 1;

        while (sqlite3_step(firstStatement) == SQLITE_ROW)
        {
            // print first query results.
            std::cout << i++ << ". " << sqlite3_column_text(firstStatement, 0) << std::endl;
        }

        // get input and test for improper values.
        while (!(std::cin >> choice) || (choice > (i - 1)) || choice < 1)
        {
            if (!std::cin)
            {
                std::cin.clear();
                std::cin.ignore(INT_MAX, '\n');
            }

            std::cout << "That is not a valid choice! Try again!" << std::endl;
        }

        // reset the statement and iterate to the choice.
        sqlite3_reset(firstStatement);
        for (int i = 0; i < choice; i++)
        {
            sqlite3_step(firstStatement);
        }

        // retrieve our customer id, then craft our new query and prepare our next statement.
        cusID = reinterpret_cast<const char *>(sqlite3_column_text(firstStatement, 1));
        sqlite3_finalize(firstStatement);

        std::string query2 = "SELECT * FROM CUSTOMER WHERE CUS_CODE = ";
        query2 += cusID += ";";

        sqlite3_stmt *secondStatement;
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &secondStatement, NULL) != SQLITE_OK)
        {
            // When an error occurs preparing the second statement...
            std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(secondStatement);
            std::cout << "There was an error: " << err << std::endl;
            return;
        }

        else
        {
            // Since we have verified programatically that we should have a result here,
            // we step once without checking that it is valid.
            sqlite3_step(secondStatement);

            lastName = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 1));
            firstName = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 2));

            // we have to test to see if this column that CAN be empty is.
            // otherwise it will crash if you try to get results from someone without
            // an initial!
            if (!(sqlite3_column_type(secondStatement, 3) == SQLITE_NULL))
            {
                initial = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 3));
            }

            areaCode = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 4));
            phoneNum = reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 5));
            balance += reinterpret_cast<const char *>(sqlite3_column_text(secondStatement, 6));

            sqlite3_finalize(secondStatement);

            std::cout << "----Customer Information----\n";
            if (initial != "")
                std::cout << "Name: " << firstName << " " << initial << ". " << lastName;
            else
                std::cout << "Name: " << firstName << " " << lastName;
            std::cout << "\nPhone Number: (" << areaCode << ")" << phoneNum
                      << "\nBalance: " << balance;

            return;
        }
    }
}

void addInvoice(sqlite3 *db)
{
    int rc = beginTransaction(db);
    if (rc != SQLITE_OK)
    {
        return;
    }
    std::string query = "select cus_code, cus_fname || ' ' || cus_lname as name ";
    query += "from customer order by cus_code";
    sqlite3_stmt *result;
    std::string error;
    std::string cus_code;
    int inv_number;

    char formatDate[80];
    time_t currentDate = time(NULL);
    strftime(formatDate, 80, "%F", localtime(&currentDate)); // for date and time "%F %T"
    std::string inv_date(formatDate);
    double total = 0;
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        rollback(db, query);
        sqlite3_finalize(result);
        return;
    }
    std::cout << "Please choose the customer for the invoice:" << std::endl;
    int i = 0, choice;
    do
    {
        if (sqlite3_column_type(result, 0) != SQLITE_NULL)
        {
            std::cout << ++i << ". " << sqlite3_column_text(result, 0);
            std::cout << " - " << sqlite3_column_text(result, 1);
            std::cout << std::endl;
        }
        rc = sqlite3_step(result);
    } while (rc == SQLITE_ROW);
    std::cin >> choice;
    while (!std::cin || choice < 1 || choice > i)
    {
        if (!std::cin)
        {
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
        }
        std::cout << "That is not a valid choice! Try again!" << std::endl;
        std::cin >> choice;
    }
    sqlite3_reset(result);
    for (int j = 0; j < choice; j++)
    {
        sqlite3_step(result);
    }
    cus_code = reinterpret_cast<const char *>(sqlite3_column_text(result, 0));
    sqlite3_finalize(result);
}
