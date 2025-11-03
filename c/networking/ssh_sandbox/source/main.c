

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <libssh/libssh.h>

typedef struct {
    char hostname[256];
    char user[64];
    int port;
    int verbosity;
    char command[4096];
} arguments_t;

void print_help() {
    printf("ssh_sandbox <username>@<host-ip>:<port> <cmd>\n");
}

char* get_user() {
#ifdef WIN32
    return getenv("USERNAME");
#else
    return getenv("USER");
#endif
}

int parse_args(int argc, char *argv[], arguments_t *arguments) {
    //printf("%s: argc: %d\n", __func__, argc);
    int pos_arg_index = 0;
    for (int i = 1; i < argc; i++) {
        //printf("%s:    parsing arg[%d]: %s\n", __func__, i, argv[i]);
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
                arguments->verbosity = atoi(argv[i++]);
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_help();
                exit(-1);
            } else {
                fprintf(stderr, "Unknown argument: %s\n", argv[i]);
                exit(-1);
            }
        } else {
            //printf("%s:   parsing positional argument [%d]: %s\n", 
            //    __func__, pos_arg_index, argv[i]);
            if (pos_arg_index == 0) {
                char *conn_string = argv[i];
                char *at_sign = strchr(conn_string, '@');
                char *colon = strchr(conn_string, ':');
                char *env_user = get_user();
                strncpy(arguments->user, env_user, sizeof(arguments->user));
                arguments->port = 22;
                if (at_sign == NULL && colon == NULL) {
                    strncpy(arguments->hostname, conn_string, sizeof(arguments->hostname));
                } else if (at_sign == NULL){
                    strncpy(arguments->hostname, conn_string, colon - conn_string);
                    char port_string[32] = {0};
                    strncpy(port_string, colon + 1, strlen(colon + 1));
                    arguments->port = atoi(port_string);
                } else if (colon == NULL) {
                    strncpy(arguments->hostname, at_sign + 1, strlen(at_sign + 1));
                    memset(arguments->user, 0, sizeof(arguments->user));
                    strncpy(arguments->user, conn_string, at_sign - conn_string);
                } else {
                    strncpy(arguments->hostname, at_sign + 1, colon - at_sign - 1);
                    memset(arguments->user, 0, sizeof(arguments->user));
                    strncpy(arguments->user, conn_string, at_sign - conn_string);
                    char port_string[32] = {0};
                    strncpy(port_string, colon + 1, strlen(colon + 1));
                    arguments->port = atoi(port_string);
                }
                //printf("ip: %s, user: %s, port: %d\n", 
                //    arguments->hostname, arguments->user, arguments->port);
                pos_arg_index += 1;
            } else if (pos_arg_index == 1) {
                char *command_string = argv[i];
                strncpy(arguments->command, command_string, sizeof(arguments->command));
                //printf("%s: cmd: %s\n", __func__, arguments->command);
                pos_arg_index += 1;
            }
        }
    }
    if (pos_arg_index != 2) {
        printf("wrong number of positional arguments: %d\n", pos_arg_index);
        return(-1);
    }
    return(0);
}

#ifdef WIN32
#include <conio.h>
char *get_password_win32(const char *prompt, char *buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return NULL;
    }
    printf("%s", prompt);

    size_t index = 0;
    int ch;
    while (index < (size - 1)) {
        ch = _getch();

        if (ch == '\r') {
            break;
        } else if (ch == '\b') {
            if (index > 0) {
                index--;
                printf("\b \b");
            }
        } else if (ch == 0 || ch == 224) {
            _getch();
            continue;
        } else if (ch >= 0 && ch <= 255) {
            buffer[index] = (char)ch;
            index++;
            printf("*");
        }
    }
    buffer[index] = '\0';
    printf("\n");
    return buffer;
}
#else
#include <termios.h>
#include <unistd.h> // for STDIN_FILENO
char *get_password_linux(const char *prompt, char *buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return NULL;
    }
    printf("%s", prompt);

    printf("TOOD: %s\n", __func__);
    return NULL;
}
#endif

char *get_password(const char *prompt, char *buffer, size_t size) {
#ifdef WIN32
    return get_password_win32(prompt, buffer, size);
#else
    return get_password_linux(prompt, buffer, size);
#endif
}

int sbox_configure(ssh_session *session, arguments_t *arguments) {
    //printf("%s:\n", __func__);
    *session = ssh_new();
    if (*session == NULL) {
        fprintf(stderr, "Failed to create SSH session");
        return -1;
    }
    ssh_options_set(*session, SSH_OPTIONS_HOST, arguments->hostname);
    ssh_options_set(*session, SSH_OPTIONS_USER, arguments->user);
    ssh_options_set(*session, SSH_OPTIONS_LOG_VERBOSITY, &arguments->verbosity);
    return 0;
}

int sbox_connect(ssh_session *session, arguments_t *arguments) {
    //printf("%s: Connecting to %s@%s:%d...\n", __func__,
    //    arguments->user, arguments->hostname, arguments->port);
    if (ssh_connect(*session) != SSH_OK) {
        fprintf(stderr, "Error connecting to %s: %s\n",
            arguments->hostname, ssh_get_error(*session)
        );
        ssh_free(*session);
        return -1;
    }
    //printf("%s: Connected!\n", __func__);
    return 0;
}

char *get_answer(const char *prompt, char *buffer, size_t buffer_size) {
    printf("%s", prompt);
    fflush(stdout);
    char *result = fgets(buffer, (int)buffer_size, stdin);
    if (result == NULL) {
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = 0;
    return buffer;
}

/**
 * Attempts to authenticate with the remote server.
 */
int sbox_authenticate(ssh_session *session,
    const char *user,
    bool public_key_auth,
    bool password_auth,
    bool password_keyboard_interactive_auth,
    bool password_auth_plaintext
) {
    if (public_key_auth) {
        // PubkeyAuthentication yes
        int rc = ssh_userauth_publickey_auto(*session, user, NULL);
        if (rc == SSH_AUTH_SUCCESS) {
            return(0);
        } else {
            fprintf(stderr, "Error authenticating with public key: %s\n", 
                ssh_get_error(*session)
            );
        }
    }

    if (password_auth) {
        // PasswordAuthentication yes
        if (password_keyboard_interactive_auth) {
            // KbdInteractiveAuthentication yes
            printf("%s: keyboard-interactive auth...\n", __func__);
            int rc = ssh_userauth_kbdint(*session, user, NULL);
            char password[256] = { 0 };
            while (rc == SSH_AUTH_INFO) {
                int num_prompts = ssh_userauth_kbdint_getnprompts(*session);
                if (num_prompts == 0) {
                    fprintf(stderr, "kbd-int error: received 0 prompts\n");
                    break;
                }

                for (int i = 0; i < num_prompts; i++) {
                    unsigned char echo = 0;
                    const char *prompt = ssh_userauth_kbdint_getprompt(*session, i, &echo);
                    if (prompt == NULL) {
                        fprintf(stderr, "Error getting kbd-int prompt %d\n", i);
                        rc = SSH_AUTH_ERROR; // Force an error
                        break;
                    }

                    char answer[256] = { 0 };
                    if (echo) {
                        if (get_answer(prompt, answer, sizeof(answer)) == NULL) {
                             fprintf(stderr, "Error reading answer for kbd-int\n");
                             return -1;
                        } else {
                            printf("TODO: store new user?\n");
                        }
                    } else {
                        if (get_password(prompt, answer, sizeof(answer)) == NULL) {
                            fprintf(stderr, "Error reading password for kbd-int\n");
                            memset(answer, 0, sizeof(answer));
                            return -1;
                        }
                    }

                    if (ssh_userauth_kbdint_setanswer(*session, i, answer) < 0) {
                        fprintf(stderr, "Error setting kbd-int answer %d\n", i);
                        rc = SSH_AUTH_ERROR;
                        break;
                    }
                    memset(answer, 0, sizeof(answer));
                }

                if (rc == SSH_AUTH_ERROR) {
                    break; 
                }
                rc = ssh_userauth_kbdint(*session, user, NULL);
            }
            if (rc != SSH_AUTH_SUCCESS) {
                fprintf(stderr, "Error authenticating with keyboard-interactive: %s\n",
                    ssh_get_error(*session)
                );
            } else {
                return 0;
            }
        }

        if (password_auth_plaintext) {
            // KbdInteractiveAuthentication no
            char password[256] = { 0 };
            get_password("Password:", password, sizeof(password));
            //printf("%s: Authenticating...\n", __func__);
            int rc = ssh_userauth_password(*session, user, password);
            memset(password, 0, sizeof(password));
            if (rc != SSH_AUTH_SUCCESS) {
                fprintf(stderr, "Error authenticating: %s\n", ssh_get_error(*session));
                ssh_disconnect(*session);
                ssh_free(*session);
                return -1;
            }
            return 0;
        }
    }
    fprintf(stderr, "Error authenticating: %s\n", ssh_get_error(*session));
    ssh_disconnect(*session);
    ssh_free(*session);
    return -1;
}

int sbox_open_channel(ssh_session *session, ssh_channel *channel) {
    //printf("%s: opening channel...\n", __func__);
    if (*channel == NULL) {
        fprintf(stderr, "Failed to create channel.\n");
        ssh_disconnect(*session);
        ssh_free(*session);
        return(-1);
    }
    if (ssh_channel_open_session(*channel) != SSH_OK) {
        fprintf(stderr, "Failed to open channel session: %s\n",
            ssh_get_error(*session)
        );
        ssh_channel_free(*channel);
        ssh_disconnect(*session);
        ssh_free(*session);
        return(-1);
    }
    return(0);
}

int sbox_execute_command(ssh_session *session, ssh_channel *channel, char *command) {
    //printf("%s: Executing remote command: %s\n", __func__, command);
    if (ssh_channel_request_exec(*channel, command) != SSH_OK) {
        fprintf(stderr, "Failed to execute command: %s\n",
            ssh_get_error(*session)
        );
        ssh_channel_close(*channel);
        ssh_channel_free(*channel);
        ssh_disconnect(*session);
        ssh_free(*session);
        return(-1);
    }
    return(0);
}

int sbox_read_output(ssh_session *session, ssh_channel *channel, char *buffer, int buffer_size) {
    //printf("Output:\n");
    int nbytes = ssh_channel_read(*channel, buffer, buffer_size, 0);
    while (nbytes > 0) {
        if (fwrite(buffer, 1, nbytes, stdout) != (size_t)nbytes) {
            fprintf(stderr, "Error writing to stdout\n");
            ssh_channel_close(*channel);
            // full cleanup
            return(-1);
        }
        nbytes = ssh_channel_read(*channel, buffer, buffer_size, 0);
    }
    printf("\n");
    if (nbytes < 0) {
        fprintf(stderr, "Error reading from channel: %s\n",
            ssh_get_error(*session)
        );
        return(-1);
    }
    return(0);
}

void sbox_cleanup(ssh_session *session, ssh_channel *channel) {
    ssh_channel_send_eof(*channel);
    ssh_channel_close(*channel);
    ssh_channel_free(*channel);
    ssh_disconnect(*session);
    ssh_free(*session);
}

int main(int argc, char *argv[]) {
    printf("Starting ssh_sandbox.\n");
    ssh_session my_ssh_session = NULL;
    arguments_t arguments = { 0 };
    arguments.verbosity = SSH_LOG_PROTOCOL;
    if (parse_args(argc, argv, &arguments) != 0) { exit(-1); }
    if (sbox_configure(&my_ssh_session, &arguments) != 0) { exit(-1); }
    if (sbox_connect(&my_ssh_session, &arguments) != 0) { exit(-1); }
    if (sbox_authenticate(&my_ssh_session, arguments.user, true, true, true, true) != 0) { exit(-1); }
    ssh_channel channel = ssh_channel_new(my_ssh_session);
    if (sbox_open_channel(&my_ssh_session, &channel) != 0) { exit(-1); }
    if (sbox_execute_command(&my_ssh_session, &channel, arguments.command) != 0) { exit(-1); }
    char output_buffer[256];
    if (sbox_read_output(&my_ssh_session, &channel, output_buffer, sizeof(output_buffer)) != 0) { exit(-1); }
    sbox_cleanup(&my_ssh_session, &channel);
    printf("Exiting ssh_sandbox.\n");
    return 0;
}
