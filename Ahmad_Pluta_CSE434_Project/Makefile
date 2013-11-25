#
# FILE: Makefile
# AUTHOR: Rizwan Ahmad and Stephen Pluta
# DESCRIPTION: This file is used to compile udpClient.c and udpServer.c
#

CLIENTSOURCES = udpClient.c
SERVERSOURCES = udpServer.c
CLIENTTARGET = client.out
SERVERTARGET = server.out

# Perform the compilation and build the executable
all: $(SERVERTARGET) $(CLIENTTARGET)

$(CLIENTTARGET): $(CLIENTSOURCES)
	gcc $(CLIENTSOURCES) -o $(CLIENTTARGET)
	
$(SERVERTARGET): $(SERVERSOURCES)
	gcc $(SERVERSOURCES) -o $(SERVERTARGET)


# Instructions to be executed when make clean is called (delete the executables and the incarnation file)
.PHONY: clean
clean:
	rm -f inc.txt
	rm -f $(CLIENTTARGET)
	rm -f $(SERVERTARGET)