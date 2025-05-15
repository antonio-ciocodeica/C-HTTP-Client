#ifndef _CLIENT_H_
#define _CLIENT_H_

void login_admin(char **cookies, int *cookies_count);
void add_user(char **cookies, int *cookies_count);
void get_users(char **cookies, int *cookies_count);
void delete_user(char **cookies, int *cookies_count);
void logout_admin(char **cookies, int *cookies_count);
void login(char **cookies, int *cookies_count);
void get_access(char **cookies, int *cookies_count, char **jwt);
void get_movies(char **cookies, int *cookies_count, char **jwt);
void get_movie(char **cookies, int *cookies_count, char **jwt);
void add_movie(char **cookies, int *cookies_count, char **jwt);
void delete_movie(char **cookies, int *cookies_count, char **jwt);
void update_movie(char **cookies, int *cookies_count, char **jwt) ;
void get_collections(char **cookies, int *cookies_count, char **jwt);
void get_collection(char **cookies, int *cookies_count, char **jwt);
void add_collection(char **cookies, int *cookies_count, char **jwt);
void delete_collection(char **cookies, int *cookies_count, char **jwt);
void add_movie_to_collection(char **cookies, int *cookies_count, char **jwt);
void delete_movie_from_collection(char **cookies, int *cookies_count, char **jwt);
void logout(char **cookies, int *cookies_count, char **jwt);

#endif