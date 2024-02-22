#ifndef user_h
#define user_h

int msg_add(int user_id,char *user_name, char *content, int type);
int msg_get_by_type(int val, char *bufstr);
#endif