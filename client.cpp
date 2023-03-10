#include <iostream>
#include <bits/stdc++.h>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

#define PORT 8080

char *printError(char *message) {
    char *msg;
    msg = strtok(message, "\"");
    for (int j = 0; j < 3; j++) {
        msg = strtok(NULL, "\"");
    }
    return msg;
}
string find_cookie(char *response) { 
    char *p = strstr(response, "Set-Cookie: ");
    char *cookie_token = strtok(p, " ;:");
    if(cookie_token != NULL) {
        cookie_token = strtok(NULL, " ");
    }
    string cookie(cookie_token);
    // eliminam virgula de la final
    cookie.pop_back();
    return cookie;
}

int main(int argc, char *argv[]) {
    const char *HOST= (const char*)"34.241.4.235";
    int sockfd;
    char *message;
    char *response;
    char *request;
    char comanda[30];
    string my_cookie;
    char **cookies =  (char**)malloc(1 * sizeof(char*));
    string token;
    char *jwt_token = (char*)malloc(1024 * sizeof(char));
    char *c;
    size_t len = 0;
    while(1) {
        scanf("%s", comanda);
        // deschid conexiunea cu serverul
        sockfd = open_connection((char*)"34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        // verific daca primesc comanda register
        if(strcmp(comanda, "register") == 0) {
            // citesc username-ul si parola 
            string username, password;
            getline(&c, &len,stdin);
            printf("username=");
            getline(cin, username, '\n');

            printf("password=");
            getline(cin, password, '\n');

            // se verifica daca exista spatii in credentiale
            const char *nume = username.c_str();
            const char *parola = password.c_str();
            if(strstr(nume, " ")  || strstr(parola, " ") ) {
                cout<<"You didn't type username or password properly!"<<endl;
                continue;
            } 
            // se creeaza obiectul de tip json
            json obj = {{"username", username}, {"password", password}};
            string info = obj.dump();

            // se convertesc credentialele la char*
            char *infos = (char *) malloc(info.size() + 1);
            char *credentials[1];
            strcpy(infos, info.c_str());
            credentials[0] = (char *) malloc(strlen(infos) + 1);
            strcpy(credentials[0], infos);


            // se trimite o cerere catre server
            request = compute_post_request(HOST, (char*) "/api/v1/tema/auth/register",
                                           (char*)"application/json", credentials, 1, NULL, 0, NULL);
            send_to_server(sockfd, request);

            // primim raspuns de la server
            response = receive_from_server(sockfd);
            char *resp = strdup(response);
            // se verifica daca s-a creat contul si se afiseaza mesajul corespunzator
            if(strstr(resp, "HTTP/1.1 201") != NULL) {
                cout<<"You registered!"<<endl;
            } else  {
                message = basic_extract_json_response(response);
                if(message != NULL) {
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    puts("Too many requests, please try again later.");
                }
            }
            free(response);
            free(request);
            free(resp);
        }
        if(strcmp(comanda, "login") == 0) {
            // se verifica daca utilizatorul este deja conectat pe un cont
            if(!my_cookie.empty()) {
                cout<<"You are already logged!"<<endl;
                continue;
            }
            // citesc username-ul si parola
            string username, password;
            getline(&c, &len,stdin);
            printf("username=");
            getline(cin, username, '\n');

            printf("password=");
            getline(cin, password, '\n');

            // se creeaza obiectul de tip json
            json obj = {{"username", username}, {"password", password}};
            string info = obj.dump();

            char *infos = (char *) malloc(info.size() + 1);
            char *credentials[1];
            strcpy(infos, info.c_str());
            credentials[0] = (char *) malloc(strlen(infos) + 1);
            strcpy(credentials[0], infos);

            // se trimite o cerere catre server
            request = compute_post_request( HOST, (char*) "/api/v1/tema/auth/login",
                                           (char*)"application/json", credentials, 1, NULL, 0, NULL);
            send_to_server(sockfd, request);

            // primesc raspuns de la server
            response = receive_from_server(sockfd);
            char *resp = strdup(response);

            // se verifica daca utilizatorul s-a logat si se afiseaza mesajul corespunzator
            if(strstr(resp, "HTTP/1.1 200") != NULL) {
                puts("You logged in!");
                my_cookie = find_cookie(response);
            } else{
                message = basic_extract_json_response(response); 
                if(message != NULL) {
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    puts("Too many requests, please try again later.");
                }
            }
            
            free(response);
            free(request);
            free(resp);
        }
        if(strcmp(comanda, "enter_library") == 0) {
            // se verifica daca utilizatorul este logat
            if(my_cookie.size() == 0) {
                cout<<"You are not logged in!"<<endl; 
                continue;
            } else {
                // se formeaza arrayul de cookies
                cookies[0] = (char*) malloc(my_cookie.size() + 1);
                strcpy(cookies[0], my_cookie.c_str());
                
                // se trimite cerere catre server
                request = compute_get_request(HOST,(const char*) "/api/v1/tema/library/access",
                                           NULL, cookies, 1, NULL);
                send_to_server(sockfd, request);

                // se primeste raspuns de la server
                response = receive_from_server(sockfd);

                // se cauta tokenul
                string server_response(response);
                istringstream line(server_response);

                while (getline(line, token, '\n')) {
                    if (strstr(token.c_str(), "token")) {
                        break;
                    }
                }
                char* my_token = (char*)token.c_str();
                jwt_token = strtok(my_token, ":");
                jwt_token = strtok(NULL, "\"");
                char *resp = strdup(response);
                // se verifica daca utilizatorul are access si se afiseaza mesajul corespunzator
                if(strstr(resp, "HTTP/1.1 200") != NULL) {
                    cout<<"You have access now!"<<endl;
                } else {
                    message = basic_extract_json_response(response);
                    if(message != NULL) {
                            char *error = printError(message);
                            cout<<error<<endl;
                        } else {
                            puts("Too many requests, please try again later.");
                        }
                }    
                free(response);
                free(resp);
                free(request);
            }

        }
        if(strcmp(comanda, "get_books") == 0){
            // se verifica daca utilizatorul este logat si/sau are access
            if (strlen(jwt_token) == 0 && my_cookie.size() == 0) {
                 cout << "You don't have access to the library!"<<endl;
                 cout << "You are not logged in!"<<endl; 
                continue;
            } else if(strlen(jwt_token) == 0) {
                cout << "You don't have access to the library!"<<endl;
                continue;
            }
            // se trimite cerere la server
            request = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, cookies, 1, jwt_token);
            send_to_server(sockfd, request);

            // se primeste raspuns de la server
            response = receive_from_server(sockfd);
            char *resp = strdup(response);

            //se verifica daca s-au trimis detaliile cartiilor
            if(strstr(resp, "HTTP/1.1 200") != NULL) {
                if(basic_extract_json_response(response) != NULL) {
                    char *details = basic_extract_json_response(response);
                    printf("%s\n", details);
                }
            } else {
                message = basic_extract_json_response(response);
                if(message != NULL) {
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    puts("Too many requests, please try again later");
                }
            }
            free(response);
            free(resp);
            free(request);
        }
        if (strcmp(comanda,"get_book") == 0) {
            // se tastateaza id-ul si se verifica daca e numar
            char id[20];
            printf("id=");
            scanf("%s", id);
            bool not_number = false;
            for(int i = 0; id[i] != '\0'; i++) {
                if(isdigit(id[i]) == 0) {
                    not_number = true;
                }
            }
            // se verifica daca utilizatorul este logat si/sau are acces
            if (strlen(jwt_token) == 0 && my_cookie.size() == 0) {
                 cout << "You don't have access to the library!"<<endl;
                 cout << "You are not logged in!"<<endl; 
                continue;
            } else if(strlen(jwt_token) == 0) {
                cout << "You don't have access to the library!"<<endl;
                continue;
            }
            if(not_number == true) {
                cout<<"The id is not valid!"<<endl;
                continue;
            }
            // se trimite cerere la server
            string pathToBook = "/api/v1/tema/library/books/"+ to_string(atoi(id));
            request = compute_get_request(HOST, pathToBook.c_str(), NULL, cookies, 1, jwt_token);
            send_to_server(sockfd, request);

            // se primeste raspuns de la server
            response = receive_from_server(sockfd);
            char *resp = strdup(response);
            
            //  verific daca utilizatorul s-a putut conecta:
            if(strstr(resp, "HTTP/1.1 200") != NULL) {
                char *details = basic_extract_json_response(response);
                printf("%s\n", details);
            } else {
                if(basic_extract_json_response(response) != NULL) {
                    message = basic_extract_json_response(response);
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    puts("Too many requests, please try again later.");
                }
            }
            free(response);
            free(resp);
            free(request);
        }
        if(strcmp(comanda, "add_book") == 0) {
            // se verifica daca utilizatorul este logat si/sau are acces
            if (strlen(jwt_token) == 0 && my_cookie.size() == 0) {
                 cout << "You don't have access to the library!"<<endl;
                 cout << "You are not logged in!"<<endl; 
                continue;
            } else if(strlen(jwt_token) == 0) {
                cout << "You don't have access to the library!"<<endl;
                continue;
            }
            // se citesc informatiile de la tastatura
            getline(&c, &len,stdin);
            string title, author, genre, publisher;
            char page_count[10];
            printf("title=");
            getline(cin, title, '\n');
            printf("author=");
            getline(cin, author, '\n');
            printf("genre=");
            getline(cin, genre, '\n');
            printf("publisher=");
            getline(cin, publisher, '\n');
            printf("page_count=");
            scanf("%s", page_count);

            // se verifica daca page_count e numar
            bool not_number = false;
            for(int i = 0; page_count[i] != '\0'; i++) {
                if(isdigit(page_count[i]) == 0) {
                    not_number = true;
                }
            }
            if(not_number == true) {
                cout<<"The page_count is not valid!"<<endl;
                continue;
            }
            int page_counter = atoi(page_count);
            
            // se creeaza obiectul json
            json obj = {
                {"title", title},
                {"author", author},
                {"genre", genre},
                {"publisher", publisher},
                {"page_count", page_counter}
            };
            string info = obj.dump();  

            // se pun informatiile intr-un char array
            char *infos = (char *) malloc((info.size() + 1)*sizeof(char));
            strcpy(infos, info.c_str());
            char *data[1];
            data[0] = (char *) malloc(strlen(infos) + 1);
            strcpy(data[0], infos); 

            // se trimite mesaj la server
            request = compute_post_request( HOST, (char*)  "/api/v1/tema/library/books",
                                           (char*)"application/json", data, 1, cookies, 1, jwt_token);
            send_to_server(sockfd, request);

            // primesc raspuns de la server:
            response = receive_from_server(sockfd);
            char *aux = strdup(response);

            // verific daca cartea a fost adaugata cu succes
            if(strstr(response, "HTTP/1.1 200") != NULL) {
                puts("Book was added!");
            } else {
                //  intorc mesajul de eroare primit de la server
                if(basic_extract_json_response(response) != NULL) {
                    message = basic_extract_json_response(response);
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    cout<<"Too many requests, please try again later!"<<endl;
                }
            }
            free(response);
            free(aux);
            free(request);
        }
        if(strcmp(comanda, "delete_book") == 0) {
            // se tastateaza id-ul si se verifica daca e numar
            char id[20];
            printf("id=");
            scanf("%s", id);
            bool not_number = false;
            for(int i = 0; id[i] != '\0'; i++) {
                if(isdigit(id[i]) == 0) {
                    not_number = true;
                }
            }
            // se verifica daca utilizatorul este logat si/sau are acces
            if (strlen(jwt_token) == 0 && my_cookie.size() == 0) {
                 cout << "You don't have access to the library!"<<endl;
                 cout << "You are not logged in!"<<endl; 
                continue;
            } else if(strlen(jwt_token) == 0) {
                cout << "You don't have access to the library!"<<endl;
                continue;
            }
            if(not_number == true) {
                cout<<"The id is not valid!"<<endl;
                continue;
            }
            // se trimite cerere la server
            string pathToBook = "/api/v1/tema/library/books/"+ to_string(atoi(id));
            request = compute_delete_request(HOST, pathToBook.c_str(), NULL, cookies, 1, jwt_token);
            send_to_server(sockfd, request);

            // raspunsul serverului
            response = receive_from_server(sockfd);
            char *aux = strdup(response);

            //  verific daca cartea a fost stearsa
            if(strstr(response, "HTTP/1.1 200") != NULL) {
                cout<<"The book was deleted!"<<endl;
            } else {
                // intorc mesajul de eroare primit de la server
                if(basic_extract_json_response(response) != NULL) {
                    message = basic_extract_json_response(response);
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    cout<<"Too many requests, please try again later!"<<endl;
                }
            }
            free(response);
            free(aux);
            free(request);
        }
        if(strcmp(comanda, "logout") == 0) {
            // verific daca sunt logat
            if(my_cookie.empty()) {
                cout<<"You are not logged in!"<<endl; 
                continue;
            } 
            // creez char arrayul cookies
            cookies[0] = (char*) malloc(my_cookie.size() + 1);
            strcpy(cookies[0], my_cookie.c_str());

            // trimit mesaj la server
            request = compute_get_request(HOST, (char*)"/api/v1/tema/auth/logout", NULL, cookies, 1, jwt_token);
            send_to_server(sockfd, request);

            // preiau raspunsul de la server
            response = receive_from_server(sockfd);
            char *aux = strdup(response);

            // verific daca utilizatorul s-a putut deloga
            if(strstr(response, "HTTP/1.1 200") != NULL) {
                memset(jwt_token, 0, strlen(jwt_token));
                my_cookie = "";
                cout<<"You logged out!"<<endl;
            } else {
                //  intorc mesajul de eroare primit de la server
                if(basic_extract_json_response(response) != NULL) {
                    message = basic_extract_json_response(response);
                    char *eroare = printError(message);
                    cout<<eroare<<endl;
                } else {
                    cout<<"Too many requests, please try again later!"<<endl;
                }
            }
            free(response);
            free(aux);
            free(request);
        }
        if(strcmp(comanda, "exit") == 0) {
            close_connection(sockfd);
            return 0;
        }
        close_connection(sockfd);
    }
    return 0;
}
