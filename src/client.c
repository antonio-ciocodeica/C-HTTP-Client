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
#include "parson.h"

#define ADDRESS "63.32.125.183"
#define PORT 8081

#define ADMIN_USER "antonio.ciocodeica"
#define ADMIN_PASSWORD "10bb25a982ac"

FILE *fout;

void login_admin(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;

    // Read the input
    char username[50];
    printf("username=");
    scanf("%s", username);

    char password[50];
    printf("password=");
    scanf("%s", password);

    // Parse the input into JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "username", ADMIN_USER);
    json_object_set_string(json_object, "password", ADMIN_PASSWORD);

    char *serialized_string = json_serialize_to_string(json_value);

    // Send a POST request to the server containing admin's login information
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/admin/login", "application/json",
                                   &serialized_string, 1, cookies, *cookies_count);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Extract the cookie and save it into the cookies array
    char *cookie = extract_cookie(response);
    if (cookie) {
        cookies[(*cookies_count)++] = cookie;
    }

    fprintf(fout, "%s\n\n", response);
    fflush(fout);

    // Possible results
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Admin autentificat cu succes\n");
    }

    if (strstr(response, "error\": \"Invalid credentials")) {
        printf("Invalid credentials");
    }

    if (strstr(response, "error\": \"Already logged in")) {
        printf("Already logged in");
    }
 
    // Free the memory used
    json_free_serialized_string(serialized_string);
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void add_user(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;

    // Read the input
    char username[50];
    printf("username=");
    scanf("%s", username);

    char password[50];
    printf("password=");
    scanf("%s", password);

    // Parse the input into JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *body_data = json_serialize_to_string(json_value);

    // Send a POST request to the server with new user's information
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/admin/users", "application/json",
                                   &body_data, 1, cookies, *cookies_count);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n\n", response);
    fflush(fout);

    // Possible results
    if (strstr(response, "HTTP/1.1 201 CREATED")) {
        printf("SUCCESS: Utilizator adăugat cu succes\n");
    }

    if (strstr(response, "error\":\"User already exists")) {
        printf("User already exists");
    }

    // Free memory used
    json_free_serialized_string(body_data);
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_users(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/admin/users",
                                  NULL, cookies, *cookies_count);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n\n", response);
    fflush(fout);

    // If server responds with 200 OK, print the users
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista utilizatorilor\n");

        // Get the JSON string from the response
        char *json_users = strchr(response, '{');
        JSON_Value *json_value = json_parse_string(json_users);
        JSON_Object *json_object = json_value_get_object(json_value);

        // Get the JSON array "users" from the object
        JSON_Array *users_array = json_object_get_array(json_object, "users");
        int users_count = json_array_get_count(users_array);

        // Iterate through each element of the "users" JSON array and print it
        for (int i = 0; i < users_count; i++) {
            JSON_Object *user_object = json_array_get_object(users_array, i);
            const char *username = json_object_get_string(user_object, "username");
            const char *password = json_object_get_string(user_object, "password");
            printf("#%d %s:%s\n", i + 1, username, password);
        }

        json_value_free(json_value);
    }

    // Error case: normal user can't access users list
    if (strstr(response, "error\":\"Admin privileges required")) {
        printf("error\":\"Admin privileges required\n");
    }

    // Free the memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_user(char **cookies, int *cookies_count) {
    // char *message;
    // char *response;
    // int sockfd;
}

void login(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;

    // Read input
    char admin[50];
    printf("admin_username=");
    scanf("%s", admin);

    char username[50];
    printf("username=");
    scanf("%s", username);

    char password[50];
    printf("password=");
    scanf("%s", password);

    // Parse the input to JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "admin_username", ADMIN_USER);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *body_data = json_serialize_to_string(json_value);

    // Send a POST request to the server containing the account's information
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/user/login", "application/json",
                                   &body_data, 1, cookies, *cookies_count);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Extract the cookie from the response
    char *cookie = extract_cookie(response);
    if (cookie) {
        cookies[(*cookies_count)++] = cookie;
    }

    // Possible results
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Autentificare reușită\n");
    }

    if (strstr(response, "error\":\"User already logged in")) {
        printf("error\":\"User already logged in\n");
    }

    // Free the memory used
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void logout_admin(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/admin/logout",
                                  NULL, cookies, *cookies_count);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // remove the cookie from admin's session
    free(cookies[0]);
    (*cookies_count)--;

    // Possible cases
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Admin delogat\n");
    }

    if (strstr(response, "error\":\"Admin privileges required")) {
        printf("Admin privileges required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

int main(int argc, char *argv[])
{   
    fout = fopen("output.txt", "w");
    char **cookies = calloc(100, sizeof(char *));
    int cookies_count = 0;

    while (1) {
        char command[50];
        scanf("%s", command);

        if (!strcmp(command, "exit")) {
            break;
        }

        if (!strcmp(command, "login_admin")) {
            login_admin(cookies, &cookies_count);
        }

        if (!strcmp(command, "add_user")) {
            add_user(cookies, &cookies_count);
        }

        if (!strcmp(command, "get_users")) {
            get_users(cookies, &cookies_count);
        }

        if (!strcmp(command, "delete_user")) {
            delete_user(cookies, &cookies_count);
        }

        if (!strcmp(command, "login")) {
            login(cookies, &cookies_count);
        }

        if (!strcmp(command, "logout_admin")) {
            logout_admin(cookies, &cookies_count);
        }

        if (!strcmp(command, "logout")) {
            logout(cookies, &cookies_count);
        }
    }

    return 0;
}
