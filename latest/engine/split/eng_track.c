static __thread int track__sock = -1;

//~ Lifecycle methods
int track_init(char const *host, char const *port) {
    if (track__sock > -1) {
        swrapClose(track__sock);
        track__sock = -1;
    }

    track__sock = swrapSocket(SWRAP_UDP, SWRAP_CONNECT, SWRAP_DEFAULT, host, port);

    if (track__sock == -1) {
        return -TRACK_ERROR_SOCKET_FAIL;
    }

    return 0;
}
int track_destroy(void) {
    if (track__sock > -1) swrapClose(track__sock);
    track__sock = -1;
    return 0;
}

//~ Buffer utilities
static __thread char track__buffer[TRACK_SEND_BUFSIZE+1];
static __thread int track__buffer_len = 0;
static __thread int track__errno = 0;

static void track__buffer_flush(void) {
    track__buffer_len = 0;
}

static int track__buffer_appendc(char *buf, int *len, char const *str) {
    int size = (int)strlen(str);
    if (*len+size > TRACK_SEND_BUFSIZE)
        return -TRACK_ERROR_BUFFER_FULL;
    memcpy(buf+*len, str, size);
    *len += size;
    return 0;
}

#define TRACK__APPEND_SAFE_EX(buf, len, xx)\
    track__errno = track__buffer_appendc(buf, len, xx);\
    if (track__errno) return track__errno;


#define TRACK__APPEND_SAFE(xx)\
    TRACK__APPEND_SAFE_EX(track__buffer, &track__buffer_len, xx);

//~ Event tracking
int track_event(char const *event_id, char const *user_id, char const *json_payload) {
    if (track__sock == -1)
        return -TRACK_ERROR_SOCKET_INVALID;
    if (!event_id || !user_id || !json_payload)
        return -TRACK_ERROR_INPUT_INVALID;
    track__buffer_flush();

    TRACK__APPEND_SAFE("{\"userId\":\"");
    TRACK__APPEND_SAFE(user_id);
    TRACK__APPEND_SAFE("\",\"event\":\"");
    TRACK__APPEND_SAFE(event_id);
    TRACK__APPEND_SAFE("\",\"properties\":");
    TRACK__APPEND_SAFE(json_payload);
    TRACK__APPEND_SAFE("}");

    if (!swrapSend(track__sock, track__buffer, track__buffer_len))
        return -TRACK_ERROR_SEND_FAIL;
    return 0;
}

int track_ident(char const *user_id, char const *traits) {
    if (track__sock == -1)
        return -TRACK_ERROR_SOCKET_INVALID;
    if (!user_id || !traits)
        return -TRACK_ERROR_INPUT_INVALID;
    track__buffer_flush();

    TRACK__APPEND_SAFE("{\"userId\":\"");
    TRACK__APPEND_SAFE(user_id);
    TRACK__APPEND_SAFE("\",\"traits\":");
    TRACK__APPEND_SAFE(traits);
    TRACK__APPEND_SAFE("}");

    if (!swrapSend(track__sock, track__buffer, track__buffer_len))
        return -TRACK_ERROR_SEND_FAIL;
    return 0;
}

int track_group(char const *user_id, char const *group_id, char const *traits) {
    if (track__sock == -1)
        return -TRACK_ERROR_SOCKET_INVALID;
    if (!user_id || !group_id || !traits)
        return -TRACK_ERROR_INPUT_INVALID;
    track__buffer_flush();
    
    TRACK__APPEND_SAFE("{\"userId\":\"");
    TRACK__APPEND_SAFE(user_id);
    TRACK__APPEND_SAFE("\",\"groupId\":\"");
    TRACK__APPEND_SAFE(group_id);
    TRACK__APPEND_SAFE("\",\"traits\":");
    TRACK__APPEND_SAFE(traits);
    TRACK__APPEND_SAFE("}");

    if (!swrapSend(track__sock, track__buffer, track__buffer_len))
        return -TRACK_ERROR_SEND_FAIL;
    return 0;
}

int track_event_props(char const *event_id, char const *user_id, const track_prop *props) {
    static char buf[TRACK_SEND_BUFSIZE+1] = {0};
    int len = 0;


    if (!props)
        return track_event(event_id, user_id, "");

    TRACK__APPEND_SAFE_EX(buf, &len, "{");
    while (props->key) {
        TRACK__APPEND_SAFE_EX(buf, &len, "\"");
        TRACK__APPEND_SAFE_EX(buf, &len, props->key);
        TRACK__APPEND_SAFE_EX(buf, &len, "\":");
        TRACK__APPEND_SAFE_EX(buf, &len, props->val);
        ++props;
        if (props->key) {
            TRACK__APPEND_SAFE_EX(buf, &len, ",");
        }
    }
    TRACK__APPEND_SAFE_EX(buf, &len, "}");

    return track_event(event_id, user_id, buf);
}

#undef TRACK__APPEND_SAFE
#undef TRACK__APPEND_SAFE_EX
