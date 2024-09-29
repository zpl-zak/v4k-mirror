#ifndef TRACK_SEND_BUFSIZE
#define TRACK_SEND_BUFSIZE 576
#endif

 //~ Errors
#define TRACK_ERROR_INIT_FAIL      1
#define TRACK_ERROR_SOCKET_FAIL    2
#define TRACK_ERROR_SOCKET_INVALID 3
#define TRACK_ERROR_BUFFER_FULL    4
#define TRACK_ERROR_SEND_FAIL      5
#define TRACK_ERROR_INPUT_INVALID  6

/// Initialises telemetry and connects to the specified endpoint.
/// return: error code
/// host: IP address / domain of the endpoint
/// port: service name / port
/// see: track_event, track_ident, track_group
API int track_init(char const *host, char const *port);

/// Destroys the currently established telemetry socket.
/// return: error code
/// No parameters needed for this function.
API int track_destroy(void);

/// Sends an EVENT message to the server.
/// return: error code
/// event_id: Identifier for the event type.
/// user_id: Identifier for the user.
/// json_payload: JSON-formatted metadata for the event.
API int track_event(char const *event_id, char const *user_id, char const *json_payload);

/// Sends user identification to the server.
/// return: error code
/// user_id: Identifier for the user.
/// traits: JSON-formatted traits or attributes of the user.
API int track_ident(char const *user_id, char const *traits);

/// Associates a user to a group.
/// return: error code
/// user_id: Identifier for the user.
/// group_id: Identifier for the group.
/// traits: JSON-formatted traits or attributes of the group.
API int track_group(char const *user_id, char const *group_id, char const *traits);

//~ Event utilities

/// Structure to represent key-value pairs for event properties.
typedef struct track_prop {
 char const *key;
 char const *val;
} track_prop;

/// Sends an EVENT message with custom properties.
/// return: error code
/// event_id: Identifier for the event type.
/// user_id: Identifier for the user.
/// props: Array of key-value pairs. Terminates when key is set to NULL.
API int track_event_props(char const *event_id, char const *user_id, const track_prop *props);
