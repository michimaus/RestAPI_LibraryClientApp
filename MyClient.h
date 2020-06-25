#include <iostream>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <unordered_map>
#include <list>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <string>
#include "helpers.h"
#include "requests.h"

// The library I used for manipulating Json objects
using json = nlohmann::json;

#ifndef _MY_CLIENT_
#define _MY_CLIENT_

// Paths and markers used when constructing the messages
#define SERV_PATH_LOGIN "/api/v1/tema/auth/login"
#define SERV_PATH_LOGOUT "/api/v1/tema/auth/logout"
#define SERV_PATH_REGISTER "/api/v1/tema/auth/register"
#define SERV_PATH_ACCES "/api/v1/tema/library/access"
#define SERV_PATH_BOOKS "/api/v1/tema/library/books"
#define JSON_MSG_TYPE "application/json"

// The main class where functionalities of the client-app are implemented
class MyClient {
private:
    int clientFd;
    std::string ipServer;
    unsigned short int portServer;
    bool canRunClient, loggedIn;
    struct sockaddr_in addrServer;
    char cookies[10][255];
    int nrCookies;
    std::string validToke;

    
    // Function used when extracting the cookies from the login response
    void findCookies(char *msg) {
        char *fstOcc = strstr(msg, "Cookie");
        nrCookies = 0;
        int i = 0;
        if (fstOcc == NULL) {
            return;
        }

        fstOcc += 8;
        
        while (fstOcc[i] != '\n') {
            if (fstOcc[i] == ';') {
                fstOcc[i] = 0;
                strcpy(cookies[nrCookies], fstOcc);
                ++nrCookies;
                i += 2;
                fstOcc += i;
                i = 0;
            } else {
                ++i;
            }
        }
    }

    // Function used to establish a TCP connection with the server
    bool openConnection() {
        clientFd = socket(AF_INET, SOCK_STREAM, 0);

        if (clientFd < 0) {
            write(1, "Socket not opened...maybe try again.\n", 38);
            canRunClient = false;
            return false;
        }

        memset(&addrServer, 0, sizeof(addrServer));
        addrServer.sin_family = AF_INET;
        addrServer.sin_port = htons(portServer);
        inet_aton(ipServer.data(), &addrServer.sin_addr);

        if (connect(clientFd, (struct sockaddr*) &addrServer, sizeof(addrServer)) < 0) {
            write(1, "Can't establish connection.\n", 29);
            canRunClient = false;
            return false;
        }
        return true;
    }

    // Safely closing the connection
    void closeConnection() {
        shutdown(clientFd, SHUT_RDWR);
        close(clientFd);
    }

    // Function used to extract the first line of the response message
    // This will be diplayed, thus we know the response o the server
    void processFstLine(char *&message, char *&firstLine) {
        firstLine = message;
        message = strchr(message, '\n');
        message[0] = 0;
        message += 1;
    }

    // Function used to get username and password from stdin
    // Data is parsed to json and checks it is valid input
    bool toJsonNamePass(json &myJson) {
        bool ok = true;
        std::string inputCmd;
        std::getline(std::cin, inputCmd);

        write(1, "Username = ", 12);
        std::getline(std::cin, inputCmd);
        if (inputCmd.find(' ') != std::string::npos) {
            ok = false;
        }
        myJson["username"] = inputCmd;

        write(1, "Password = ", 12);
        std::getline(std::cin, inputCmd);
        if (inputCmd.find(' ') != std::string::npos) {
            ok = false;
        }
        myJson["password"] = inputCmd;
        if (!ok) {
            write(1, "\n\n--------\n\n", 13);
            write(1, "Username and password should not contain empty spaces\n", 55);
            write(1, "Process aborted\n", 17);
            write(1, "\n--------\n\n", 12);
        }
        return ok;
    }

    // The following functions get a char pointer asa parameter
    // and modify it, in order to send a specific HTTP message and afterwards
    // the HTTP response is stored in message once again
    // Meanwhile the conenction resets on each opperation and the function
    // will return false in case the connection process fails
    bool registerCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        json myJson;

        if (!toJsonNamePass(myJson)) {
            message = NULL;
            return true;
        }

        std::string inputCmd;
        inputCmd = myJson.dump(4);
        myJson.clear();

        message = postReqJson(ipServer.data(), SERV_PATH_REGISTER, JSON_MSG_TYPE, inputCmd.data(), NULL, 0);
        send_to_server(clientFd, message);
        free(message);

        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool loginCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        json myJson;

        if (!toJsonNamePass(myJson)) {
            message = NULL;
            return true;
        }

        std::string inputCmd;
        inputCmd = myJson.dump(4);
        myJson.clear();

        message = postReqJson(ipServer.data(), SERV_PATH_LOGIN, JSON_MSG_TYPE, inputCmd.data(), NULL, 0);
        send_to_server(clientFd, message);

        free(message);

        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool enterLibCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }

        char *auxCookie[10];
        for (int i = 0; i < nrCookies; ++i) {
            auxCookie[i] = cookies[i];
        }

        message = getReqBasic(ipServer.data(), SERV_PATH_ACCES, NULL, auxCookie, nrCookies);
        send_to_server(clientFd, message);
        free(message);

        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool addBookCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        json myJson;
        std::string inputCmd;
        std::getline(std::cin, inputCmd);

        write(1, "Title = ", 9);
        std::getline(std::cin, inputCmd);
        myJson["title"] = inputCmd;

        write(1, "Author = ", 10);
        std::getline(std::cin, inputCmd);
        myJson["author"] = inputCmd;

        write(1, "Genre = ", 9);
        std::getline(std::cin, inputCmd);
        myJson["genre"] = inputCmd;

        write(1, "Number of pages = ", 19);
        std::getline(std::cin, inputCmd);
        myJson["page_count"] = inputCmd;

        write(1, "Publisher = ", 13);
        std::getline(std::cin, inputCmd);
        myJson["publisher"] = inputCmd;

        inputCmd = myJson.dump(4);
        myJson.clear();

        message = postReqJsonAuthorized(ipServer.data(), SERV_PATH_BOOKS, JSON_MSG_TYPE,
                                        validToke.data(), inputCmd.data(), NULL, 0);

        send_to_server(clientFd, message);

        free(message);
        message = receive_from_server(clientFd);
        closeConnection();

        return true;
    }

    bool getBooksCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        message = getReqAuthorized(ipServer.data(), SERV_PATH_BOOKS, NULL, validToke.data(), NULL, 0);
        send_to_server(clientFd, message);
        free(message);
        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool getBookCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        json myJson;
        std::string inputCmd;
        std::getline(std::cin, inputCmd);


        write(1, "Book ID = ", 11);
        std::getline(std::cin, inputCmd);
        std::string newPath = SERV_PATH_BOOKS;
        newPath = newPath + '/' + inputCmd;

        message = getReqAuthorized(ipServer.data(), newPath.data(), NULL,
                                        validToke.data(), NULL, 0);
        send_to_server(clientFd, message);
        free(message);
        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool deleteBookCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }
        json myJson;
        std::string inputCmd;
        std::getline(std::cin, inputCmd);

        write(1, "Book ID = ", 11);
        std::getline(std::cin, inputCmd);
        std::string newPath = SERV_PATH_BOOKS;
        newPath = newPath + '/' + inputCmd;

        message = delReqJsonAuthorized(ipServer.data(), newPath.data(), NULL,
                                        validToke.data(), NULL, 0);
        send_to_server(clientFd, message);
        free(message);
        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    bool logoutCommand(char *&message) {
        if (!openConnection()) {
            return false;
        }

        char *auxCookie[10];
        for (int i = 0; i < nrCookies; ++i) {
            auxCookie[i] = cookies[i];
        }

        message = getReqBasic(ipServer.data(), SERV_PATH_LOGOUT, NULL, auxCookie, nrCookies);
        send_to_server(clientFd, message);
        free(message);

        message = receive_from_server(clientFd);
        closeConnection();
        return true;
    }

    // The next two functions are used to print the content of a Json
    // Without key
    void writeJsonContent(char *toJson) {
        json myJson;
        if (toJson != NULL) {
            myJson = json::parse(basic_extract_json_response(toJson));
            for (auto it = myJson.begin(); it != myJson.end(); ++it) {
                std::cout << it.value() << '\n';
            }
        }
    }

    // With key
    void writeJsonFIeldsAndContent(char *toJson) {
        json myJson;
        if (toJson != NULL) {
            myJson = json::parse(basic_extract_json_response(toJson));
            for (auto it = myJson.begin(); it != myJson.end(); ++it) {
                std::cout << it.key() << " : " << it.value() << '\n';
            }
        }
    }


public:

    // Constructor - initialization of the app
    MyClient(char* ipToServer, unsigned short int portToServer) {
        this->portServer = portToServer;
        this->ipServer = ipToServer;
        this->canRunClient = true;
        this->loggedIn = false;
        this->nrCookies = 0;
        this->validToke.clear();
    }

    ~MyClient() { }

    // Implementation of the main runner where an acction is executed
    // depending on which command is introduced
    void runClient() {
        if (canRunClient == false) {
            write(1, "Can't run client.\n", 19);
            return;
        }

        std::string inputCmd;
        char *message = NULL;
        char *firstLine = NULL;
        json myJson;

        while (canRunClient) {
            write(1, "Your command: ", 15);
            std::cin >> inputCmd;

            // Ignore a command if it doesn't exist
            if (inputCmd != "register" &&
                inputCmd != "exit" &&
                inputCmd != "login" &&
                inputCmd != "enter_library" &&
                inputCmd != "get_books" &&
                inputCmd != "get_book" &&
                inputCmd != "add_book" &&
                inputCmd != "logout" &&
                inputCmd != "delete_book") {

                write(1, "\n\n--------\n\n", 13);
                write(1, "Please insert a valid commnad.\n", 32);
                write(1, "\n--------\n\n", 12);
                continue;
            }

            // The next conditional structures are checking which
            // command is introduced
            // Excanging information with the server and composing the output
            // depending on the response comming from the server
            if (inputCmd == "register") {
                if (!registerCommand(message)) {
                    break;
                }
                if (message == NULL) {
                    continue;
                }
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);

                // In case of error the server will send a json
                // with an explicit cause of the mallfunction
                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                writeJsonContent(toJson);
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "exit") {
                // The loop is interupted so the program comes to an end
                write(1, "\n\n--------\n\n", 13);
                write(1, "Closing the app.\n", 18);
                write(1, "\n--------\n\n", 12);
                break;
            } else if (inputCmd == "login") {
                if (loggedIn == true) {
                    // The command is skipped in case there is someone logged in
                    write(1, "\n\n--------\n\n", 13);
                    write(1, "You are already logged in.\n", 28);
                    write(1, "\n--------\n\n", 12);
                    continue;
                }
                if (!loginCommand(message)) {
                    break;
                }
                if (message == NULL) {
                    continue;
                }

                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);

                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                // Printing the output in case of successful response
                if (strstr(firstLine, "200") != NULL) {
                    loggedIn = true;
                    // Extracting the cookies for library access
                    findCookies(message);
                    write(1, "Logged in successfully!\n", 25);
                }
                // In case of error the server will send a json
                // with an explicit cause of the mallfunction
                writeJsonContent(toJson);
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } 
            else if (loggedIn == false) {
                // Next commands will not be valid unless there is somenone logged in
                write(1, "\n\n--------\n\n", 13);
                write(1, "You are not logged in.\n", 24);
                write(1, "\n--------\n\n", 12);
                continue;
            } else if (inputCmd == "enter_library") {
                if (!enterLibCommand(message)) {
                    break;
                }
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);

                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                myJson.clear();
                // Extracting the json in case of positive response
                if (strstr(firstLine, "200") != NULL) {
                    if (toJson != NULL) {
                        myJson = json::parse(basic_extract_json_response(toJson));
                        validToke = myJson["token"];
                    } else {
                        write(1, "Something really unexpected just happened. Token was not found", 63);
                    }
                } else {
                    writeJsonContent(toJson);
                }
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "get_books") {
                if (!getBooksCommand(message)) {
                    break;
                }
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);

                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';

                // In case there are no books a specific message is displayed
                if (strstr(firstLine, "200") != NULL) {
                    if (toJson != NULL) {
                        bool areBooks = true;
                        // Otherwise the list of Json objects is processed
                        do {
                            char *auxPtr = strstr(toJson, "},{");
                            if (auxPtr == NULL) {
                                auxPtr = strstr(toJson, "}]");
                                areBooks = false;
                            }
                            auxPtr += 1;
                            auxPtr[0] = 0;
                            write(1, "\n****************\n", 19);
                            writeJsonFIeldsAndContent(toJson);
                            write(1, "****************\n", 18);
                            auxPtr += 1;
                            toJson = auxPtr;
                        } while (areBooks);
                    } else {
                        write(1, "\n****************\n", 19);
                        write(1, "There are no books.\n", 21);
                        write(1, "****************\n", 18);
                    }
                } else {
                    writeJsonContent(toJson);
                }
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "get_book") {
                if (!getBookCommand(message)) {
                    break;
                }
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);
                write(1, "\n\n--------\n\n", 13);
                // Display data regarding requested book
                std::cout << firstLine << '\n';
                if (strstr(firstLine, "200") != NULL) {
                    char *auxPtr = strstr(toJson, "}]");
                    auxPtr += 1;
                    auxPtr[0] = 0;
                    write(1, "\n****************\n", 19);
                    writeJsonFIeldsAndContent(toJson);
                    write(1, "****************\n", 18);
                } else {
                   // Error message in case there is no book
                    writeJsonContent(toJson);
                }
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "add_book") {
                if (!addBookCommand(message)) {
                    break;
                }
                // Calls the function that builds up a json and puts
                // it on the right path.
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);
                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                // In case the acction fails an error message
                // is sent and proccessed
                writeJsonContent(toJson);
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "delete_book") {
                if (!deleteBookCommand(message)) {
                    break;
                }
                // Calls the function that builds up the path to the book
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);
                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                // In case the acction fails an error message
                // is sent and proccessed
                writeJsonContent(toJson);
                write(1, "\n--------\n\n", 12);
                free(firstLine);

            } else if (inputCmd == "logout") {
                if (!logoutCommand(message)) {
                    break;
                }
                processFstLine(message, firstLine);
                char *toJson = basic_extract_json_response(message);

                write(1, "\n\n--------\n\n", 13);
                std::cout << firstLine << '\n';
                if (strstr(firstLine, "200") != NULL) {
                    // In case of success a specific message is displayed
                    write(1, "Logged out with success.\n", 26);
                    loggedIn = false;
                    validToke.clear();
                    for (int i = 0; i < nrCookies; ++i) {
                        cookies[i][0] = 0;
                    }
                    nrCookies = 0;
                } else {
                    // otherwise an error message will be coming from the server
                    writeJsonContent(toJson);
                }
                write(1, "\n--------\n\n", 12);
                free(firstLine);
            }
        }
    }
};

#endif
