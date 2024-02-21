#ifndef user_goods_h
#define user_goods_h
// int open();
// void freedb();
int addUserGoods(int user_id, char *goods_name,int goods_name_len, int counts);
int addUserGoodsByGid(int user_id, int gid, int counts);
// int write(int user_id, int goods_id, int counts);
int getUserGoodsListByUserId(int user_id, char *buf);
int selectColByGoodsIdUserId(int user_id, int goods_id, char *colName, int *colVal);
#endif