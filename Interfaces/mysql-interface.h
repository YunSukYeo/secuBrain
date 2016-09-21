#ifndef __MYSQL_INTERFACE__
#define __MYSQL_INTERFACE__

#include <mysql/mysql.h>

typedef void (*MysqlCallback) (MYSQL_RES *);

int MysqlInitalize();
int MysqlGetNumRows(MYSQL_RES *sqlResult);
int MysqlGetNumColumns(MYSQL_RES *sqlResult);
MYSQL_FIELD* MysqlGetFields(MYSQL_RES *sqlResult);
MYSQL_ROW MysqlGetRow(MYSQL_RES *sqlResult);
void MysqlSelectQuery(char *tableName, char *columns, char *where, MysqlCallback callback);
void MysqlInsertQuery(char *tableName, char *colums, char *values, MysqlCallback callback);
void MysqlUpdateQuery(char *tableName, char *columns, char *values, char *where, MysqlCallback callback);
void MysqlDeleteQuery(char *tableName, char *where, MysqlCallback callback);
#endif
