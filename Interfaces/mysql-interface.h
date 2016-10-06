#ifndef __MYSQL_INTERFACE__
#define __MYSQL_INTERFACE__

#ifndef bool
#define bool short
#endif

#define true 1
#define false 0

#include <mysql/mysql.h>

typedef void (*MysqlCallback) (MYSQL_RES *);

int MysqlInitialize();
int MysqlGetLastInsertedRowID();
int MysqlGetNumRows(MYSQL_RES *sqlResult);
int MysqlGetNumColumns(MYSQL_RES *sqlResult);
MYSQL_FIELD* MysqlGetFields(MYSQL_RES *sqlResult);
MYSQL_ROW MysqlGetRow(MYSQL_RES *sqlResult);
MYSQL_RES *MysqlSelectQuery(char *tableName, char *columns, char *where, int limitOneOption);
bool MysqlInsertQuery(char *tableName, char *colums, char *values);
bool MysqlUpdateQuery(char *tableName, char *columns, char *values, char *where);
bool MysqlDeleteQuery(char *tableName, char *where);
#endif
