#include "model.h"
#include "user.h"
#include "msg.h"
#include "user_goods.h"
#include<stdbool.h>
void model_init(){
   open();
}
void model_close(){
    // freedb();
}
int  model_user_add(char *account, int account_len, char *pwd, int pwd_len){
    return addUser(account, account_len, pwd, pwd_len);
}
int  model_user_login(char *account, int account_len, char *pwd, int pwd_len){
    return loginUser(account, account_len, pwd, pwd_len);
}
int model_user_goods_add(int user_id, char *goods_name,int goods_name_len, int counts){
    return addUserGoods(user_id,goods_name, goods_name_len,counts);
}
int model_user_goods_list(int user_id, char *buf){
   return getUserGoodsListByUserId(user_id, buf);
}
int model_user_goods_add_bygid(int user_id, int gid, int counts){
    return addUserGoodsByGid(user_id, gid, counts);
}
int model_user_goods_getColVal(int user_id, int goods_id, char *colName, int *colVal){
    return selectColByGoodsIdUserId(user_id,goods_id,colName, colVal);
}
int model_user_info_all(char *key, int key_len, char *val, int val_len,char *bufstr){
    return select_user_info_all(key,key_len, val, val_len, bufstr);
}
int model_user_update_kv_byid(int user_id,char *key, int key_len, void * val, int val_len,int val_type){
    return updateKvById(user_id,key,key_len,val,val_len,val_type);
}

//msg
int model_msg_add(int user_id, char *name,char *content, int type){
    return msg_add(user_id, name, content, type);
}

int model_msg_get_by_type(int val, char *bufstr){
    return msg_get_by_type(val,bufstr);
}