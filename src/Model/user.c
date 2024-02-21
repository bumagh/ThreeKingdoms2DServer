#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "user.h"
#include "yyjson.h"
char *err = NULL;
char *dbpath = "./data/user.db";
static int select_callback(void *ret, int argc, char **argv, char **azColName)
{
    int i;
    int *pret = (int *)ret;
    for (i = 0; i < argc; i++)
    {
        // printf( "%s = %s\n", azColName[i], argv[i]);
        //  fprintf(stderr, "%s = %s\n", azColName[i], argv[i]);
    }

    *pret = 1;
    return 0;
}

static int selectjson_callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    char *jsonbuf = (char *)data;

    // read json
    if (strlen(jsonbuf) == 0)
    {
        // new json
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *newarr = yyjson_mut_arr(doc);
        yyjson_mut_val *ele = yyjson_mut_obj_with_str(doc, (const char **)azColName, (const char **)argv, argc);
        yyjson_mut_arr_append(newarr, ele);
        yyjson_mut_doc_set_root(doc, newarr);
        const char *json = yyjson_mut_write(doc, 0, NULL);

        // printf("%s\n", json);
        strcpy(jsonbuf, json);
        // printf("jsonbuf:%s\n", jsonbuf);
        free(json);
        yyjson_mut_doc_free(doc);
    }
    else
    {

        yyjson_doc *doc = yyjson_read(jsonbuf, strlen(jsonbuf), 0);
        if (doc)
        {
            yyjson_mut_doc *mutdoc = yyjson_doc_mut_copy(doc, NULL);
            yyjson_mut_val *mutroot = yyjson_mut_doc_get_root(mutdoc);
            if (yyjson_is_arr(mutroot))
            {

                yyjson_mut_val *ele = yyjson_mut_obj_with_str(mutdoc, (const char **)azColName, (const char **)argv, argc);
                yyjson_mut_arr_append(mutroot, ele);
                const char *json = yyjson_mut_write(mutdoc, 0, NULL);
                // printf("newjson:%s\n", json);
                strcpy(jsonbuf, json);
                free(json);
            }
            else
            {
                printf("no it not arr\n");
            }
            yyjson_doc_free(doc);
            yyjson_mut_doc_free(mutdoc);
        }
        else
            printf("read error:\n");
    }

    for (i = 0; i < argc; i++)
    {
        // printf( "%s = %s\n", azColName[i], argv[i]);
        // fprintf(stderr, "%s = %s\n", azColName[i], argv[i]);
    }

    return 0;
}

static int select_login_callback(void *ret, int argc, char **argv, char **azColName)
{
    int i;
    int *pret = (int *)ret;
    for (i = 0; i < argc; i++)
    {
        // printf( "%s = %s\n", azColName[i], argv[i]);
        // fprintf(stderr, "%s = %s\n", azColName[i], argv[i]);
        *pret = atoi(argv[i]);
    }

    if (argc == 0)
    {
        *pret = 0;
    }
    return 0;
}

static int write_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        // fprintf(stderr, "%s = %s\n", azColName[i], argv[i]);
    }
    printf("\n");
    return 0;
}
int open()
{

    return 0;
}
void freedb()
{
}
// 1 exist 0 ok 2 error
int addUser(char *account, int account_len, char *pwd, int pwd_len)
{ // 添加用户
    int selret = selectByKey(account);
    if (selret == 1)
        return 1;
    else if (selret == -1)
        return 2;
    else
        return write(account, account_len, pwd, pwd_len);
}
// 1 not exist 0 ok 2 error
int loginUser(char *account, int account_len, char *pwd, int pwd_len)
{ // 添加用户
    int selret = selectLogin(account, pwd);
    if (selret == -2)
        return -2;
    else if (selret == -1)
        return -1;
    else
        return selret;
}
int write(char *account, int account_len, char *pwd, int pwd_len)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char *data = "callback";
    char *tb = "user";
    /* Open database */
    rc = sqlite3_open(dbpath, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

    sql = "insert into %s (account,pwd) "
          "values ('%s','%s');";
    char buf[250];
    sprintf(buf, sql, tb, account, pwd);
    /* Execute SQL statement */
    rc = sqlite3_exec(db, buf, write_callback, (void *)data, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    else
    {
        fprintf(stdout, "Records created successfully\n");
    }
    sqlite3_close(db);
    return 0;
}

int selectByKey(char *key)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;
    /* Open database */
    rc = sqlite3_open(dbpath, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    /* Create SQL statement */
    sql = "select id from user where account='%s';";
    char buf[128];
    sprintf(buf, sql, key);
    /* Execute SQL statement */
    selectRet = sqlite3_exec(db, buf, select_callback, (void *)&ret, &zErrMsg);
    if (selectRet != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -1;
    }

    fprintf(stdout, "Operation done successfully\n");
    if (ret == 1)
    {
        printf("select data not null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    return 0;
}

int selectLogin(char *account, char *pwd)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;
    /* Open database */
    rc = sqlite3_open(dbpath, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    /* Create SQL statement */
    sql = "select id from user where account='%s' and pwd='%s';";
    char buf[128];
    sprintf(buf, sql, account, pwd);
    /* Execute SQL statement */
    selectRet = sqlite3_exec(db, buf, select_login_callback, (void *)&ret, &zErrMsg);
    if (selectRet != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -1;
    }

    fprintf(stdout, "Operation done successfully\n");
    if (ret == 0)
    {
        printf("select data not null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -2;
    }

    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    return ret;
}

int select_user_info_all(char *key, int key_len, char *val, int val_len, char *bufstr)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;
    /* Open database */
    rc = sqlite3_open(dbpath, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    /* Create SQL statement */
    sql = "select * from user where %s='%s';";
    char buf[128];
    sprintf(buf, sql, key, val);
    char jsonbuf[2048];
    /* Execute SQL statement */
    selectRet = sqlite3_exec(db, buf, selectjson_callback, (void *)&jsonbuf, &zErrMsg);
    if (selectRet != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -1;
    }

    fprintf(stdout, "Operation done successfully\n");
    if (strlen(jsonbuf) == 0)
    {
        printf("select data not null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -2;
    }

    // printf("ends:%s\n", jsonbuf);
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    strcpy(bufstr, jsonbuf);

    return ret;
}

// update

int updateKvById(int user_id, char *key, int key_len, void *val, int val_len, int val_type)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int ret = 0;
    char *sql;
    const char *data = "callback";
    char *tb = "user";
    /* Open database */
    rc = sqlite3_open(dbpath, &db);
    if (rc)
    {

        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }
    char buf[1024];

    if (val_type == 0)
    {
        // int
        sql = "update %s set %s=%d where id=%d;"
              "SELECT * from %s";
        sprintf(buf, sql, tb, key, (int)val, user_id, tb);
    }
    else
    {
        sql = "update %s set %s='%s' where id=%d;"
              "SELECT * from %s";
        sprintf(buf, sql, tb, key, (char *)val, user_id, tb);
    }

    /* Execute SQL statement */
    rc = sqlite3_exec(db, buf, NULL, NULL, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    else
    {
        fprintf(stdout, "Records created successfully\n");
    }
    sqlite3_close(db);
    return 0;
}
