#include "mysql-interface.h"
#include <mysql/mysql.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "secu"
#define DB_NAME "hackathon"

static MYSQL *connection = NULL, conn;

int MysqlInitialize() {
    mysql_init(&conn);
    connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *) NULL, 0);
    if(connection == NULL) {
        fprintf(stderr, "Mysql connection error : %s\n", mysql_error(&conn));
        return 0;
    }
    return 1;
}

int MysqlGetLastInsertedRowID() {
    char *query = (char *) malloc(32);
    int n = sprintf(query, "SELECT LAST_INSERT_ID()");
    query[n] = '\0';
    
    int queryStat = mysql_query(connection, query);
    free(query);

    if(queryStat != 0) {
        fprintf(stderr, "Mysql query error %s\n", mysql_error(&conn));
        return 0;
    }

    MYSQL_RES *sqlResult = mysql_store_result(connection);
    if(MysqlGetNumRows(sqlResult) != 0) {
        MYSQL_ROW row = MysqlGetRow(sqlResult);
        return (int) *(row[0]);
    }
    return 0;
}

int MysqlGetNumRows(MYSQL_RES *sqlResult) {
    if(sqlResult == NULL) return 0;
    return mysql_num_rows(sqlResult);
}

int MysqlGetNumColumns(MYSQL_RES *sqlResult) {
    if(sqlResult == NULL) return 0;
    return mysql_num_fields(sqlResult);
}

MYSQL_FIELD* MysqlGetFields(MYSQL_RES *sqlResult) {
    if(sqlResult == NULL) return 0;
    return mysql_fetch_fields(sqlResult);
}

MYSQL_ROW MysqlGetRow(MYSQL_RES *sqlResult) {
    if(sqlResult == NULL) return 0;
    return mysql_fetch_row(sqlResult);
}

MYSQL_RES* MysqlSelectQuery(char *tableName, char *columns, char *where, int limitOneOption) {
    int lenColumns = strlen(columns), lenTable = strlen(tableName), lenWhere = where == NULL ? 0 : strlen(where);
    int len = lenColumns + lenTable + lenWhere + 25;
    char *query = (char*)malloc(len);
    MYSQL_RES *sqlResult;
    memset(query, '\0', len);

    char *tmp = query;
    strcpy(tmp, "SELECT "); tmp += 7;
    strcpy(tmp, columns); tmp += lenColumns;
    strcpy(tmp, " FROM "); tmp += 6;
    strcpy(tmp, tableName); tmp += lenTable;

    if(where != NULL) {
        strcpy(tmp, " WHERE "); tmp += 7;
        strcpy(tmp, where); tmp += lenWhere;
    }

    if(limitOneOption) {
        strcpy(tmp, " LIMIT 1"); tmp += 8;
    }

    int queryStat = mysql_query(connection, query);

    free(query);

    if(queryStat != 0) {
        fprintf(stderr, "Mysql query error %s\n", mysql_error(&conn));
        return NULL;
    }

    sqlResult = mysql_store_result(connection);
    return sqlResult;
    //callback(sqlResult);
    //mysql_free_result(sqlResult);
}

bool MysqlInsertQuery(char *tableName, char *columns, char *values) {
    int lenTable = strlen(tableName), lenColumns = strlen(columns), lenValues = strlen(values);
    int len = lenTable + lenColumns + lenValues + 30;
    char *query = (char*)malloc(len);

    memset(query, '\0', len);

    strcpy(query, "INSERT INTO ");
    strcpy(query + 12, tableName);
    strcpy(query + 12 + lenTable, " (");
    strcpy(query + 12 + lenTable + 2, columns);
    strcpy(query + 12 + lenTable + 2 + lenColumns, ") VALUES (");
    strcpy(query + 12 + lenTable + 2 + lenColumns + 10, values);
    strcpy(query + 12 + lenTable + 2 + lenColumns + 10 + lenValues, ")");

    int queryStat = mysql_query(connection, query);

    free(query);
    if(queryStat != 0) {
        fprintf(stderr, "Mysql query error %s\n", mysql_error(&conn));
        return false;
    }

    return true;
}

bool MysqlUpdateQuery(char *tableName, char *columns, char *values, char *where) {
    int lenTable = strlen(tableName), lenColumns = strlen(columns), lenValues = strlen(values), lenWhere = strlen(where);
    int len = lenTable + lenColumns + lenValues + lenWhere + 30;
    char *query = (char*)malloc(len), *tmp;

    memset(query, '\0', len);

    tmp = query;
    strcpy(tmp, "UPDATE "); tmp += 7;
    strcpy(tmp, tableName); tmp += lenTable;
    strcpy(tmp, " set "); tmp += 5;

    char *cols = (char *)malloc(lenColumns), *vals = (char *)malloc(lenValues);
    char *s1, *s2;
    strcpy(cols, columns); strcpy(vals, values);

    char *colPointer = strtok_r(cols, ",", &s1);
    char *rowPointer = strtok_r(vals, ",", &s2);

    while(colPointer && rowPointer) {
        strcpy(tmp, colPointer); tmp += strlen(colPointer);
        strcpy(tmp, "="); tmp += 1;
        strcpy(tmp, rowPointer); tmp += strlen(rowPointer);
        strcpy(tmp, ", "); tmp += 2;

        colPointer = strtok_r(NULL, ",", &s1);
        rowPointer = strtok_r(NULL, ",", &s2);
    }
    tmp -= 2; // delete ","

    strcpy(tmp, " WHERE "); tmp += 7;
    strcpy(tmp, where);

    int queryStat = mysql_query(connection, query);

    free(query);
    free(cols);
    free(vals);

    if(queryStat != 0) {
        fprintf(stderr, "Mysql query error %s\n", mysql_error(&conn));
        return false;
    }

    return true;
}

bool MysqlDeleteQuery(char *tableName, char *where) {
    int lenTable = strlen(tableName), lenWhere = strlen(where);
    int len = lenTable + lenWhere + 25;
    char *query = (char*)malloc(len);
    memset(query, '\0', len);

    strcpy(query, "DELETE FROM ");
    strcpy(query + 12, tableName);
    strcpy(query + 12 + lenTable, " WHERE ");
    strcpy(query + 12 + lenTable + 7, where);
    int queryStat = mysql_query(connection, query);

    free(query);
    if(queryStat != 0) {
        fprintf(stderr, "Mysql query error %s\n", mysql_error(&conn));
        return false;
    }

    return true;
}
