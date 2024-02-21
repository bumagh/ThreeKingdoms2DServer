#ifndef user_h
#define user_h
int open();
void freedb();
int addUser(char *account, int account_len, char *pwd, int pwd_len);
int write(char *account, int account_len, char *pwd, int pwd_len);
int loginUser(char *account, int account_len, char *pwd, int pwd_len);
int select_user_info_all(char *key, int key_len, char *val, int val_len, char *bufstr);
int updateKvById(int user_id,char *key, int key_len, void * val, int val_len,int val_type);
#endif