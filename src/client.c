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

#define MAX_COOKIES 100

void login_admin(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;
    getchar();

    // Read and validate input
    char username[BUFLEN];
    printf("username=");
    if (read_user(username) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    char password[BUFLEN];
    printf("password=");
    if (read_user(password) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    // Parse the input into JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

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

    // Possible results
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Admin autentificat cu succes\n");
    }

    if (strstr(response, "Invalid credentials")) {
        printf("ERROR: Invalid credentials\n");
    }

    if (strstr(response, "Already logged in")) {
        printf("ERROR: Already logged in\n");
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
    getchar();

    // Read and validate input
    char username[BUFLEN];
    printf("username=");
    if (read_user(username) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    char password[BUFLEN];
    printf("password=");
    if (read_user(password) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

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

    // Possible results
    if (strstr(response, "HTTP/1.1 201 CREATED")) {
        printf("SUCCESS: Utilizator adăugat cu succes\n");
    }

    if (strstr(response, "User already exists")) {
        printf("ERROR: User already exists\n");
    }

    if (strstr(response, "Admin privileges required")) {
        printf("ERROR: Admin privileges required\n");
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

    // If server responds with 200 OK, print the users
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista utilizatorilor\n");

        // Get the JSON string from the response
        char *json_users = basic_extract_json_response(response);
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
    if (strstr(response, "Admin privileges required")) {
        printf("ERROR: Admin privileges required\n");
    }

    // Free the memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_user(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;
    getchar();

    // Read and validate input
    char username[BUFLEN];
    printf("username=");
    if (read_user(username) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    char path[strlen("/api/v1/tema/admin/users/") + BUFLEN];
    sprintf(path, "/api/v1/tema/admin/users/%s", username);

    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_delete_request(ADDRESS, path, cookies, *cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Utilizator șters\n");
    }
 
    if (strstr(response, "Admin privileges required")) {
        printf("ERROR: Admin privileges required\n");
    }

    if (strstr(response, "User not found")) {
        printf("ERROR: User not found\n");
    }
    
    // Free the memory used
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

    // Remove the cookie from admin's session
    if (cookies[0] && cookies_count > 0) {
        free(cookies[0]);
        cookies[0] = NULL;
        (*cookies_count)--;
    }
    
    // Possible cases
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Admin delogat\n");
    }

    if (strstr(response, "Admin privileges required")) {
        printf("ERROR: Admin privileges required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void login(char **cookies, int *cookies_count) {
    char *message;
    char *response;
    int sockfd;
    getchar();

    // Read and validate input
    char admin[BUFLEN];
    printf("admin_username=");
    if (read_user(admin) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    char username[BUFLEN];
    printf("username=");
    if (read_user(username) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    char password[BUFLEN];
    printf("password=");
    if (read_user(password) == -1) {
        printf("ERROR: Username/Password should not contain whitespaces\n");
        return;
    }

    // Parse the input to JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "admin_username", admin);
    json_object_set_string(json_object, "username", username);
    json_object_set_string(json_object, "password", password);

    char *body_data = json_serialize_to_string(json_value);

    // Send a POST request to the server containing the account's information
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, "/api/v1/tema/user/login", "application/json",
                                   &body_data, 1, cookies, *cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Extract the cookie from the response
    char *cookie = extract_cookie(response);
    if (cookie) {
        cookies[(*cookies_count)++] = cookie;
    }

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Autentificare reușită\n");
    }

    if (strstr(response, "User already logged in")) {
        printf("ERROR: User already logged in\n");
    }

    if (strstr(response, "Invalid credentials")) {
        printf("ERROR: Invalid credentials\n");
    }

    if (strstr(response, "Admin user not found")) {
        printf("ERROR: Admin user not found\n");
    }

    // Free the memory used
    json_value_free(json_value);
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

    // Extract the JWT token from the response
    *jwt = extract_jwt(response);

    // Possible cases
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Token JWT primit\n");
    }

    if (strstr(response, "Authentication required")) {
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

    // If the JWT token is valid, format the output string and print it
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista filmelor\n");

        // Get the JSON string from the response
        char *json_movies = basic_extract_json_response(response);
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
            printf("#%d %s\n", id, title);
        }

        json_value_free(json_value);
    }

    // If the user does not have a JWT token, print an error
    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
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

    // Read and validate input
    int id;
    printf("id=");

    // Check if the id is an int
    if (read_int(&id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Compute the path for the request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/movies/%d", id);

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, path, NULL, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // If the movie was found, print the details
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Detalii film\n");

        // Extract the JSON string from the response
        char *string = basic_extract_json_response(response);
        JSON_Value *json_value = json_parse_string(string);
        JSON_Object *json_object = json_value_get_object(json_value);

        // Print the details of the movie
        printf("title: %s\n", json_object_get_string(json_object, "title"));
        printf("year: %d\n", (int) json_object_get_number(json_object, "year"));
        printf("description: %s\n", json_object_get_string(json_object, "description"));
        printf("rating: %s\n", json_object_get_string(json_object, "rating"));
    }

    // Movie ID not found
    if (strstr(response, "Movie not found")) {
        printf("ERROR: Movie not found\n");
    }

    // JWT token not found
    if (strstr(response, "JWT token required")) {
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

    // Read and validate input
    char *title;
    size_t title_len = 0;
    printf("title=");
    getchar();
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    int year;
    printf("year=");

    // Check if the year is an int
    if (read_int(&year) == -1) {
        printf("ERROR: Invalid year\n");
        return;
    }

    char *description = NULL;
    size_t desc_len = 0;
    printf("description=");
    getline(&description, &desc_len, stdin);
    description[strlen(description) - 1] = '\0';

    float rating;
    printf("rating=");

    // Check if the rating is a float
    if (read_float(&rating) == -1) {
        printf("ERROR: Invalid rating\n");
        return;
    }

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

    // Possible cases
    if (strstr(response, "HTTP/1.1 201 CREATED")) {
        printf("SUCCESS: Film adăugat\n");
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    if (strstr(response, "Rating must be between 0 and 9.9")) {
        printf("ERROR: Rating must be between 0 and 9.9\n");
    }

    if (strstr(response, "All fields (title, year, description, rating) are required")) {
        printf("ERROR: All fields (title, year, description, rating) are required\n");
    }

    // Free memory used
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_movie(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read and validate input
    int id;
    printf("id=");

    // Check if the id is an int
    if (read_int(&id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Compute the path for the request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/movies/%d", id);

    // Compute and send the DELETE request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_delete_request(ADDRESS, path, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Film șters cu succes\n");
    }

    if (strstr(response, "Invalid movie id")) {
        printf("ERROR: Invalid movie id\n");
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void update_movie(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read and validate input
    int id;
    printf("id=");
    
    // Check if the id is an int
    if (read_int(&id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    char *title = NULL;
    size_t title_len = 0;
    printf("title=");
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    int year;
    printf("year=");
    
    // Check if the year is an int
    if (read_int(&year) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    char *description = NULL;
    size_t desc_len = 0;
    printf("description=");
    getline(&description, &desc_len, stdin);
    description[strlen(description) - 1] = '\0';

    float rating;
    printf("rating=");
    
    // Check if the rating is a float
    if (read_float(&rating) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Parse the input fields into JSON format
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);

    json_object_set_string(json_object, "title", title);
    json_object_set_number(json_object, "year", year);
    json_object_set_string(json_object, "description", description);
    json_object_set_number(json_object, "rating", rating);

    char *body_data = json_serialize_to_string(json_value);

    // Compute the path for the request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/movies/%d", id);

    // Compute and send the PUT request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_put_request(ADDRESS, path, "application/json",&body_data, 1,
                                  cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Film actualizat\n");
    }
    
    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
    }
    
    if (strstr(response, "Invalid movie id")) {
        printf("ERROR: Invalid movie id\n");
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

    // Print the collections
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Lista colecțiilor\n");
        // Get the JSON payload of the response
        char *json_collections = basic_extract_json_response(response);;
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
    if (strstr(response, "JWT token required")) {
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

    // Read and validate input
    int id;
    printf("id=");
    
    // Check if the id is an int
    if (read_int(&id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Compute the path
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/collections/%d", id);

    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, path, NULL, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // If the collection was found, print the details
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Detalii colecție\n");

        char *string = basic_extract_json_response(response);
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

    if (strstr(response, "Invalid collection id or you do not own this collection")) {
        printf("ERROR: Invalid collection id or you do not own this collection\n");
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
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

    // Read and validate input
    char *title;
    size_t title_len = 0;
    printf("title=");
    getchar();
    getline(&title, &title_len, stdin);
    title[strlen(title) - 1] = '\0';

    int num_movies;
    printf("num_movies=");
    
    // Check if the number of movies is an int
    if (read_int(&num_movies) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

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

    // Possible responses
    int success = 0;
    if (strstr(response, "HTTP/1.1 200 OK")) {
        success = 1;
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
        json_value_free(json_value);
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    // Extract the ID of the new collection from the response
    char *string = basic_extract_json_response(response);
    json_value = json_parse_string(string);
    JSON_Object *new_coll = json_value_get_object(json_value);
    int coll_id = json_object_get_number(new_coll, "id");

    // Compute the path for POST request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/collections/%d/movies", coll_id);

    // Create the JSON object for the body data
    JSON_Value *data = json_value_init_object();
    JSON_Object *obj = json_value_get_object(data);
        
    for (int i = 0; i < num_movies; i++) {
        // Read the movie ID
        int movie_id;
        printf("movie_id[%d]=", i);
        
        // Check if the movie_id is an int
        if (read_int(&movie_id) == -1) {
            printf("ERROR: Movie ID not found\n");
            continue;
        }

        // Set the ID in the JSON object
        json_object_set_number(obj, "id", movie_id);
        char *body_data = json_serialize_to_string(data);

        // Send the POST request to add a movie to the new collection
        message = compute_post_request(ADDRESS, path, "application/json", &body_data, 1,
                                       cookies, *cookies_count, *jwt);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        // Check if the movie id is valid
        if (strstr(response, "Invalid movie id")) {
            printf("ERROR: Invalid movie id\n");
            return;
        }
    }

    if (success == 1) {
        printf("SUCCESS: Colecție adăugată\n");
    }

    // Free memory used
    json_value_free(json_value);
    json_value_free(data);
    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_collection(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read and validate input
    int id;
    printf("id=");
    
    // Check if the id is an int
    if (read_int(&id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Compute path for the request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/collections/%d", id);

    // Compute and send the DELETE request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_delete_request(ADDRESS, path, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Colecție ștearsă\n");
    }

    if (strstr(response, "HTTP/1.1 404 NOT FOUND")) {
        printf("ERROR: Invalid collection id or you do not own this collection\n");
    }

    if (strstr(response, "HTTP/1.1 401 UNAUTHORIZED")) {
        printf("ERROR: Invalid JWT token\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void add_movie_to_collection(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read and validate input
    int collection_id;
    printf("collection_id=");
    
    // Check if the collection id is an int
    if (read_int(&collection_id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    int movie_id;
    printf("movie_id=");
    
    // Check if the movie id is an int
    if (read_int(&movie_id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/collections/%d/movies", collection_id);

    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_number(json_object, "id", movie_id);
    char *body_data = json_serialize_to_string(json_value);

    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_post_request(ADDRESS, path, "application/json", &body_data, 1,
                                   cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "message\":\"Movie added to collection successfully")) {
        printf("SUCCESS: Film adăugat în colecție\n");
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    if (strstr(response, "Invalid collection id or you do not own this collection")) {
        printf("ERROR: Invalid collection id or you do not own this collection\n");
    }

    if (strstr(response, "Invalid movie id")) {
        printf("ERROR: Invalid movie id\n");
    }

    // Free memory used
    json_value_free(json_value);
    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_movie_from_collection(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Read and validate input
    int collection_id;
    printf("collection_id=");
    
    // Check if the collection id is an int
    if (read_int(&collection_id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    int movie_id;
    printf("movie_id=");
    
    // Check if the movie id is an int
    if (read_int(&movie_id) == -1) {
        printf("ERROR: Invalid input\n");
        return;
    }

    // Compute the path for the request
    char path[BUFLEN];
    sprintf(path, "/api/v1/tema/library/collections/%d/movies/%d", collection_id, movie_id);

    // Compute and send the DELETE request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_delete_request(ADDRESS, path, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Possible responses
    if (strstr(response, "HTTP/1.1 200 OK")) {
        printf("SUCCESS: Film șters din colecție\n");
    }

    if (strstr(response, "Invalid collection id or you do not own this collection")) {
        printf("ERROR: Invalid collection id or you do not own this collection\n");
    }

    if (strstr(response, "JWT token required")) {
        printf("ERROR: JWT token required\n");
    }

    // Free memory used
    free(message);
    free(response);
    close_connection(sockfd);
}

void logout(char **cookies, int *cookies_count, char **jwt) {
    char *message;
    char *response;
    int sockfd;

    // Compute and send the GET request
    sockfd = open_connection(ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request(ADDRESS, "/api/v1/tema/user/logout",
                                  NULL, cookies, *cookies_count, *jwt);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // Remove the cookie and JWT token from user's session
    if (cookies[0] && cookies_count > 0) {
        free(cookies[0]);
        cookies[0] = NULL;
        (*cookies_count)--;
    }

    if (*jwt) {
        free(*jwt);
        *jwt = NULL;
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

int main(int argc, char *argv[])
{   
    char **cookies = calloc(MAX_COOKIES, sizeof(char *));
    int cookies_count = 0;

    char *jwt = NULL;

    while (1) {
        char command[BUFLEN];
        scanf("%s", command);

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

        if (!strcmp(command, "logout_admin")) {
            logout_admin(cookies, &cookies_count);
        }
        
        if (!strcmp(command, "login")) {
            login(cookies, &cookies_count);
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

        if (!strcmp(command, "delete_movie")) {
            delete_movie(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "update_movie")) {
            update_movie(cookies, &cookies_count, &jwt);
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

        if (!strcmp(command, "delete_collection")) {
            delete_collection(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "add_movie_to_collection")) {
            add_movie_to_collection(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "delete_movie_from_collection")) {
            delete_movie_from_collection(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "logout")) {
            logout(cookies, &cookies_count, &jwt);
        }

        if (!strcmp(command, "exit")) {
            break;
        }
    }

    // Free the memory used for cookies and JWT token
    for (int i = 0; i < cookies_count; i++) {
        if (cookies[i]) {
            free(cookies[i]);
        }
    }

    if (jwt) {
        free(jwt);
    }

    free(cookies);

    return 0;
}
