# Tema 3 PCom - Web Client

## Project structure
- `client.c`, `client.h`: Implement all 20 possible commands
- `requests.c`, `requests.h`: Build `GET`, `POST`, `DELETE` and `PUT` requests
- `parson.c`, `parson.h`: JSON parser
- `helpers.c`, `helpers.h`: Helper functions for computing messages, 
sending/receiving messages to/from the server, validating input and extracting
cookies / JWT token
- `buffer.c`, `buffer.h`: Implementation of a custom buffer

## Client
The client is implemented in the `client.c` file. The client waits for input
from `STDIN` and checks if the input represents one of the 20 possible 
commands. Each command is implemented in a different function that takes the 
cookies, the number of cookies and (optional) the JWT token as parameters.

Each function:
- reads and validates input, if necessary
- opens the connection with the server
- computes and sends the request
- receives the response from the server
- prints the output to `stdout` or an error message if the request wasn't
successful
- closes the connection with the server

The `login` and `login_admin` commands send a `POST` request to the server
containing the login information and, if successful, the server responds
with a session `cookie`. The cookie is stored in the `cookies` array until
the `logout` or `logout_admin` command is given by the client and the cookie
is deleted.

The `get_access` command works similar to the login commands. The command
doesn't require any input from the user. A `GET` request is sent to the server
and the it responds with a JWT token that is stored until the user logs out.

## Input validation
Each time the user enters a command that requires input, the input is read and 
validated using these functions implemented in `helpers.h`:
- `read_int`: Used for reading integers. The function calls
`scanf("%d%c", n, &new_line)` and checks the return value. `%d` should catch
the number and `%c` the new line terminator. If `new_line` is not equal to
`\n`, the value read by scanf is not an integer.
- `read_float`: Used for reading float values. Works the same as `read_int`
- `read_user`: Used for reading usernames and passwords. The function calls
`fgets` to read a string and then checks if it contains any whitespaces.

All functions return `-1` for invalid inputs and `0` for valid inputs.

Movie titles and descriptions can contain whitespaces, numbers, normal/special 
characters so they don't need validation.

## Parson
Responses from the server are sent as `JSON`-formatted strings. The `Parson`
parser is used to extract the values from the server responses or to build
body data for requests.

#### Sending a JSON string to the server:
- Initialize a JSON object using `json_value_init_object()`.
- Set any field to a value. E.g. `json_object_set_number(obj, "id", 10)`.
sets the `id` field to `10`.
- Serialize the object to a string using `json_serialize_to_string()`.
- Send the serialized string to the server.
- Free the JSON object using `json_value_free()`.

#### Parsing a JSON response
- Initialize the JSON object using `json_parse_string()`. This function creates
an object containing all values sent by the server.
- Get the value of a field using `json_object_get_[value_type]()`. For example,
the `get_collections` command sends a `GET` request and the server responds
with an array of collections. To extract the array from the response, simply
call `json_object_get_array(obj, "collections")`. To get the number of
collections of the array, use `json_array_get_count(array)`.
- Free the JSON object using `json_value_free()`