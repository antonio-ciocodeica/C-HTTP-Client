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
                                   &serialized_string, 1, cookies, *cookies_count, NULL);
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
        printf("ERROR: Invalid credentials");
    }

    if (strstr(response, "error\": \"Already logged in")) {
        printf("ERROR: Already logged in");
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
                                   &body_data, 1, cookies, *cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n\n", response);
    fflush(fout);

    // Possible results
    if (strstr(response, "HTTP/1.1 201 CREATED")) {
        printf("SUCCESS: Utilizator adăugat cu succes\n");
    }

    if (strstr(response, "error\":\"User already exists")) {
        printf("ERROR: User already exists");
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
                                  NULL, cookies, *cookies_count, NULL);
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
        printf("ERROR: Admin privileges required\n");
    }

    // Free the memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

// void delete_user(char **cookies, int *cookies_count) {
//     char *message;
//     char *response;
//     int sockfd;
// }

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
                                   &body_data, 1, cookies, *cookies_count, NULL);
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
        printf("ERROR: User already logged in\n");
    }

    if (strstr(response, "error\":\"Invalid credentials")) {
        printf("ERROR: Invalid credentials\n");
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
                                  NULL, cookies, *cookies_count, NULL);
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
        printf("ERROR: Admin privileges required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

/*
    TODO: When admin uses logout, the response is an error, but the admin
    logs out successfully
*/
void logout(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/user/logout",
                                  NULL, cookies, *cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Remove the cookie and JWT token from admin's session
    if (cookies[0] && cookies_count > 0) {
        free(cookies[0]);
        (*cookies_count)--;
    }

    if (*jwt) {
        free(*jwt);
    }

    // Possible cases
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: User delogat\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_access(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/library/access",
                                  NULL, cookies, *cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Extract the JWT token from the response
    *jwt = extract_jwt(response);

    // Possible cases
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Token JWT primit\n");
    }

    if (strstr(response, "error\":\"Authentication required")) {
        printf("ERROR: Authentication required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_movies(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/library/movies", NULL,
                                  cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // If the JWT token is valid, format the output string and print it
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista filmelor\n");

        // Get the JSON string from the response
        char *json_movies = strchr(response, '{');
        JSON_Value *json_value = json_parse_string(json_movies);
        JSON_Object *json_object = json_value_get_object(json_value);

        // Get the JSON array "movies" from the object
        JSON_Array *movies_array = json_object_get_array(json_object, "movies");
        int movies_count = json_array_get_count(movies_array);

        // Iterate through the movies array and print their title and id
        for (int i = 0; i < movies_count; i++) {
            JSON_Object *movies_object = json_array_get_object(movies_array, i);
            const char *title = json_object_get_string(movies_object, "title");
            int id = (int) json_object_get_number(movies_object, "id");
            printf("#%d %s:%d\n", i + 1, title, id);
        }

        json_value_free(json_value);
    }

    // If the user does not have a JWT token, print an error
    if (strstr(response, "error\":\"JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    // If you have an invalid JWT token, print an error
    if (strstr(response, "error\":\"Invalid token")) {
        printf("ERROR: Invalid JWT token\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_movie(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read the input
    int id;
    printf("id=");
    scanf("%d", &id);

    // Compute the path for the request
    char path[50];
    sprintf(path, "/api/v1/tema/library/movies/%d", id);

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, path, NULL, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // If the movie was found, print the details
    if (strstr(response, "HTTP/1.1 200 OK")) {
        // Get the JSON string from the response
        char *json_movie = strchr(response, '{');
        JSON_Value *json_value = json_parse_string(json_movie);
        JSON_Object *json_object = json_value_get_object(json_value);

        // Remove the "id" and "user_id" fields from the object
        json_object_remove(json_object, "id");
        json_object_remove(json_object, "user_id");

        // Serialize the JSON value to a string and free the memory
        char *output = json_serialize_to_string(json_value);
        json_value_free(json_value);

        // Print the movie's details
        printf("%s\n", output);
    }

    // Movie ID not found
    if (strstr(response, "error\":\"Movie not found")) {
        printf("ERROR: Movie not found\n");
    }

    // JWT token not found
    if (strstr(response, "error\":\"JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void add_movie(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read the input
    char *title = NULL;
    size_t title_len = 0;
    printf("title=");
    getchar();
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    int year;
    printf("year=");
    scanf("%d", &year);
    getchar();

    char *description = NULL;
    size_t desc_len = 0;
    printf("description=");
    getline(&description, &desc_len, stdin);
    description[strlen(description) - 1] = '\0';

    double rating;
    printf("rating=");
    scanf("%lf", &rating);

    // Parse the input fields into JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "title", title);
    json_object_set_number(json_object, "year", year);
    json_object_set_string(json_object, "description", description);
    json_object_set_number(json_object, "rating", rating);

    char *body_data = json_serialize_to_string(json_value);

    // Compute and send the POST request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/library/movies", "application/json",
                                   &body_data, 1, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Possible cases
    if (strstr(response, "HTTP/1.1 201 CREATED")) {
        printf("SUCCESS: Film adăugat\n");
    }

    if (strstr(response, "error\":\"Rating must be between 0 and 9.9")) {
        printf("ERROR: Rating must be between 0 and 9.9\n");
    }

    // Free memory used
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_collections(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/library/collections", NULL,
                                  cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Print the collections
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista colecțiilor\n");
        // Get the JSON payload of the response
        char *json_collections = strchr(response, '{');
        JSON_Value *json_value = json_parse_string(json_collections);
        JSON_Object *json_object = json_value_get_object(json_value);

        // From the json payload, get the "collections" array
        JSON_Array *json_array = json_object_get_array(json_object, "collections");
        int coll_count = json_array_get_count(json_array);

        // Iterate through the collections array and print their titles
        for (int i = 0; i < coll_count; i++) {
            JSON_Object *collection = json_array_get_object(json_array, i);
            int id = json_object_get_number(collection, "id");
            const char *title = json_object_get_string(collection, "title");
            printf("#%d: %s\n", id, title);
        }

        json_value_free(json_value);
    }

    // User tried to access collections without a JWT token
    if (strstr(response, "error\":\"JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_collection(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read input
    int id;
    printf("id=");
    scanf("%d", &id);

    // Compute the path
    char path[50];
    sprintf(path, "/api/v1/tema/library/collections/%d", id);

    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, path, NULL, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // If the collection was found, print the details
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Detalii colecție\n");

        char *string = strchr(response, '{');
        JSON_Value *json_value = json_parse_string(string);
        JSON_Object *json_object = json_value_get_object(json_value);

        // Print the name and owner
        printf("title: %s\n", json_object_get_string(json_object, "title"));
        printf("owner: %s\n", json_object_get_string(json_object, "owner"));

        // Get the movies array and movies count
        JSON_Array *movies = json_object_get_array(json_object, "movies");
        int count = json_array_get_count(movies);

        // Iterate through the movie list and print each movie
        for (int i = 0; i < count; i++) {
            JSON_Object *movie = json_array_get_object(movies, i);
            int id = json_object_get_number(movie, "id");
            const char *title = json_object_get_string(movie, "title");
            printf("#%d: %s\n", id, title);
        }

        json_value_free(json_value);
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void add_collection(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read input
    char *title = NULL;
    size_t title_len = 0;
    printf("title=");
    getchar();
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    int num_movies;
    printf("num_movies=");
    scanf("%d", &num_movies);

    // Init the JSON object, set the "title" value and serialize to string
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "title", title);
    char *body_data = json_serialize_to_string(json_value);

    // Compute and send the POST request to create a new collection
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/library/collections", "application/json",
                                   &body_data, 1, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    fprintf(fout, "%s\n", response);
    fflush(fout);

    // Extract the ID of the new collection from the response
    char *string = strchr(response, '{');
    json_value = json_parse_string(string);
    JSON_Object *new_coll = json_value_get_object(json_value);
    int coll_id = json_object_get_number(new_coll, "id");

    // Compute the path for POST request
    char path[50];
    sprintf(path, "/api/v1/tema/library/collections/%d/movies", coll_id);

    // Create the JSON object for the body data
    JSON_Value *data = json_value_init_object();
    JSON_Object *obj = json_value_get_object(data);
        
    for (int i = 0; i < num_movies; i++) {
        // Read the movie ID
        int movie_id;
        printf("movie_id=");
        scanf("%d", &movie_id);

        // Set the ID in the JSON object
        json_object_set_number(obj, "id", movie_id);
        char *body_data = json_serialize_to_string(data);

        // Send the POST request to add a movie to the new collection
        message = compute_post_request(ADDRESS, path, "application/json", &body_data, 1,
                                       cookies, *cookies_count, *jwt);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        fprintf(fout, "%s\n", response);
        fflush(fout);
    }

    // Free memory used
    json_value_free(json_value);
    json_value_free(data);
    free(message);
    free(response);
    close_connection(sockfd);
}

int main(int argc, char *argv[])
{   
    fout = fopen("output.txt", "w");
    char **cookies = calloc(10, sizeof(char *));
    int cookies_count = 0;

    char *jwt = NULL;

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

        // if (!strcmp(command, "delete_user")) {
        //     delete_user(cookies, &cookies_count);
        // }

        if (!strcmp(command, "login")) {
            login(cookies, &cookies_count);
        }

        if (!strcmp(command, "logout_admin")) {
            logout_admin(cookies, &cookies_count);
        }

        if (!strcmp(command, "logout")) {
            logout(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "get_access")) {
            get_access(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "get_movies")) {
            get_movies(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "get_movie")) {
            get_movie(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "add_movie")) {
            add_movie(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "get_collections")) {
            get_collections(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "get_collection")) {
            get_collection(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "add_collection")) {
            add_collection(cookies, &cookies_count, &jwt);
        }
    }

    return 0;
}
