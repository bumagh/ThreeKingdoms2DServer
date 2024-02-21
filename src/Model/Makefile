model.a: user.o user_goods.o model.o yyjson.o msg.o
	ar rcs model.a user.o user_goods.o model.o  msg.o yyjson.o 
model.o: model.c model.h 
	gcc -c model.c
user.o: user.h user.c
	gcc -c user.c
user_goods.o: user_goods.h user_goods.c 
	gcc -c user_goods.c 
msg.0: msg.h msg.c
	gcc -c msg.c
yyjson.o:yyjson.c yyjson.h
	gcc -c yyjson.c
clean:
	-rm *.o
	-rm *.gch
	-rm *.a
