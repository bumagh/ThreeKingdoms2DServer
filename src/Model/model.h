#ifndef model_h
#define model_h
#include <lwan/lwan.h>
#include <lwan/lwan-mod-serve-files.h>
void model_init();
void model_close();
int  model_user_add(char *account, int account_len, char *pwd, int pwd_len);
int  model_user_login(char *account, int account_len, char *pwd, int pwd_len);
int model_user_goods_add(int user_id, char *goods_name,int goods_name_len, int counts);
int model_user_goods_add_bygid(int user_id, int gid, int counts);
int model_user_goods_list(int user_id, char *buf);
int model_user_goods_getColVal(int user_id, int goods_id, char *colName, int *colVal);

int model_user_info_all(char *key, int key_len, char *val, int val_len,char *bufstr);
int model_user_update_kv_byid(int user_id,char *key, int key_len, void * val, int val_len,int val_type);

//msg
int model_msg_add(int user_id, char *name, char *content, int type);
int model_msg_get_by_type(int val, char *bufstr);
#endif
