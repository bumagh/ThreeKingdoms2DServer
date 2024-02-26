#include "../../App/Controller/WsController.h"
#include <stdio.h>

int main()
{
    char msgRet[256];
    char json[256] = "{\"type\":\"EnterGame\",\"data\":1,\"msg\":\"1\"}";
    printf("%s", json);
    handleWs(json, msgRet);
    printf("ret:%s", msgRet);
    return 0;
}