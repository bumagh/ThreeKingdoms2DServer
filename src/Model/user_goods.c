#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "user_goods.h"
#include "yyjson.h"
static char *err = NULL;
static char *userGoodsDbpath = "./data/user_goods.db";
char *goodsDbPath = "./data/goods.db";
static int select_callback(void *ret, int argc, char **argv, char **azColName)
{
    int i;
    int *pret = (int *)ret;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i]);
        //  fprintf(stderr, "%s = %s\n", azColName[i], argv[i]);
        *pret = atoi(argv[i]);
    }
    if (argc == 0)
    {
        *pret = -1;
    }

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

        strcpy(jsonbuf, json);
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

// 1 notexist 0 ok 2 error
int addUserGoods(int user_id, char *goods_name, int goods_name_len, int addcounts)
{ // 添加背包物品
    int ret_goods_id = 0;
    int selGoodIdRet = selectGoodsId(goods_name, goodsDbPath, &ret_goods_id);
    if (selGoodIdRet == 0)
    {

        if (ret_goods_id != 0)
        {
            // writeGoodsToUserGoods
            // find the goods
            // todo check if exsit in user_goods
            int counts = 0;
            int ret_user_goods_id = 0;
            int retSelectId = selectColByGoodsIdUserId(user_id, ret_goods_id, "id", &ret_user_goods_id);
            if (retSelectId == 0)
            {
                // select ok
                int retSelectCounts = selectColByGoodsIdUserId(user_id, ret_goods_id, "counts", &counts);
                if (retSelectCounts == 0)
                {
                    // update
                    int retUpdate = updateCountsByUserGoodsId(ret_user_goods_id, counts + addcounts);
                    return retUpdate;
                }

                return retSelectCounts;
            }
            else if (retSelectId == 2)
            {
                // insert
                int retInsert = writeGoodsToUserGoods(user_id, ret_goods_id, addcounts);
                return 2;
            }
            else
            {
                // error
                return -1;
            }
            return ret_goods_id;
        }

        else
        {

            return 1;
        }
    }

    else if (selGoodIdRet == -1)
        return 2;
}
int addUserGoodsByGid(int user_id, int gid, int addcounts)
{ // 添加背包物品
    int ret_goods_id = gid;
    int selGoodIdRet = 0;

    if (selGoodIdRet == 0)
    {

        if (ret_goods_id != 0)
        {
            // writeGoodsToUserGoods
            // find the goods
            // todo check if exsit in user_goods
            int counts = 0;
            int ret_user_goods_id = 0;
            int retSelectId = selectColByGoodsIdUserId(user_id, ret_goods_id, "id", &ret_user_goods_id);
            if (retSelectId == 0)
            {
                printf("retselid=%d", retSelectId);
                // select ok
                int retSelectCounts = selectColByGoodsIdUserId(user_id, ret_goods_id, "counts", &counts);
                if (retSelectCounts == 0)
                {
                    // update
                    int retUpdate = updateCountsByUserGoodsId(ret_user_goods_id, counts + addcounts);
                    return retUpdate;
                }

                return retSelectCounts;
            }
            else if (retSelectId == 2)
            {
                // insert
                int retInsert = writeGoodsToUserGoods(user_id, ret_goods_id, addcounts);
                return 2;
            }
            else
            {
                // error
                return -1;
            }
            return ret_goods_id;
        }

        else
        {

            return 1;
        }
    }

    else if (selGoodIdRet == -1)
        return 2;
}
int writeGoodsToUserGoods(int user_id, int goods_id, int counts)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char *data = "callback";
    char *tb = "user_goods";
    /* Open database */
    rc = sqlite3_open(userGoodsDbpath, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

    sql = "insert into %s (user_id,goods_id,counts) "
          "values (%d,%d,%d);";
    char buf[250];
    sprintf(buf, sql, tb, user_id, goods_id, counts);
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

int selectGoodsId(char *goods_name, int goods_name_len, int *goods_id)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;

    /* Open database */
    rc = sqlite3_open(goodsDbPath, &db);

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
    sql = "select id from goods where name='%s';";
    char buf[128];
    sprintf(buf, sql, goods_name);
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
    if (ret == -1)
    {
        // can not find goods_id named goods_name
        printf("select data is null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 2;
    }
    *goods_id = ret;
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    return 0;
}

int selectColByGoodsIdUserId(int user_id, int goods_id, char *colName, int *colVal)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;
    char *tb = "user_goods";
    char *dbpath = userGoodsDbpath;
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
    sql = "select %s from %s where user_id=%d and goods_id=%d;";
    char buf[128];

    sprintf(buf, sql, colName, tb, user_id, goods_id);
    printf("%s", buf);
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
    if (ret == -1)
    {
        // can not find goods_id named goods_name
        printf("select data is null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 2;
    }
    *colVal = ret;
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    return 0;
}

int updateCountsByUserGoodsId(int user_goods_id, int newcounts)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int ret = 0;
    char *sql;
    const char *data = "callback";
    char *tb = "user_goods";
    /* Open database */
    rc = sqlite3_open(userGoodsDbpath, &db);
    if (rc)
    {

        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

    sql = "update %s set counts=%d where id=%d;"
          "SELECT * from user_goods";
    char buf[1024];
    sprintf(buf, sql, tb, newcounts, user_goods_id);
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

int getUserGoodsListByUserId(int user_id, char *bufstr)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    int selectRet;
    char *sql;
    int ret = 0;
    char *tb = "user_goods";
    char *dbpath = userGoodsDbpath;
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
    sql = "select goods_id,counts from %s where user_id=%d;";
    char buf[128];
    sprintf(buf, sql, tb, user_id);
    /* Execute SQL statement */
    //  char *jsonbuf = (char *)calloc(1024,sizeof(char*));
    char jsonbuf[2048];
    selectRet = sqlite3_exec(db, buf, selectjson_callback, (void *)jsonbuf, &zErrMsg);
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
        // can not find goods_id named goods_name
        printf("select data is null!\n");
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 2;
    }
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    strcpy(bufstr, jsonbuf);

    return 0;
}
