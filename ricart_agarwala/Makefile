all : client res

client : client.c client.h res.h
	gcc client.c -o client -lpthread -g 

res: res.c res.h
	gcc res.c -o res
