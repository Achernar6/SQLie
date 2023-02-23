#include "stdlib.h"

#include "common/sql_app.h"

int main(int argc, char* argv[])
{
    sql::SqlApp sql_app;
    if (!sql_app.init())
    {
        return EXIT_FAILURE;
    }
    sql_app.runApp();

    return EXIT_SUCCESS;
}