build: client

subscriber:
	g++ -Wall -Wextra -std=c++11 client.cpp -o client

clean:
	rm -rf client
