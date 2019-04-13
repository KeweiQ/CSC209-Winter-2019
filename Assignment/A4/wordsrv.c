#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
    #define PORT 58116
#endif
#define MAX_QUEUE 5


/* Helper functions
 */
int buf_read(int cur_fd, char *client_inbuf);
int find_network_newline(const char *buf, int n);
int valid_name(struct game_state *game, char *new_name);
int valid_guess(char *client_inbuf);
int right_guess(struct game_state *game, char guess);
void remove_from_waitlist(struct client **new_players, struct client *player);
void add_to_active(struct game_state *game, struct client *player, struct client **new_players);
void remove_quitted(struct game_state *game, struct client **players, struct client *player);

/* Given in starter code
 */
void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);

/* These are some of the function prototypes that we used in our solution
 * You are not required to write functions that match these prototypes, but
 * you may find the helpful when thinking about operations in your program.
 */
/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf);
void announce_turn(struct game_state *game, int type, struct client *player);
void announce_sep(struct game_state *game, int type, struct client *player);
/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game, struct client *player);


/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;


/* Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found.
 */
int find_network_newline(const char *buf, int n) {
    for (int i = 0; i < (n - 1); i++) {
        if ((buf[i] == '\r') && (buf[i + 1] == '\n')) {
            return (i + 2);
        }
    }
    return -1;
}


/* Read input from client by using buffer
 */
int buf_read(int cur_fd, char *client_inbuf) {
    char buf[MAX_BUF] = {'\0'};
    int inbuf = 0;           // Number of bytes currently in buffer
    int room = sizeof(buf);  // Number of bytes remaining in buffer
    char *after = buf;       // Pointer to position after the data in buf

    // Keep reading until find network newline
    int nbytes;
    while ((nbytes = read(cur_fd, after, room)) > 0) {
        // Update inbuf
        inbuf += nbytes;
        printf("[%d] Read %d bytes\n", cur_fd, nbytes);

        // Try to find network new line in buf contents
        // If found, copy valid input to player and terminate the function
        int where = find_network_newline(buf, inbuf);
        if (where != -1) {
            buf[where - 2] = '\0';
            strncpy(client_inbuf, buf, where - 1);
            printf("[%d] Found newline %s\n", cur_fd, buf);
            return 1;
        }

        // Update after and room, in preparation for the next read.
        after = &(buf[inbuf]);
        room = MAX_BUF - inbuf;
    }

    // Return 0 if current player has quitted
    return 0;
}


/* Move the has_next_turn pointer to the next active client
*/
void advance_turn(struct game_state *game, struct client *player) {
    game->has_next_turn = player->next;
    if (game->has_next_turn == NULL) {
        game->has_next_turn = game->head;
    }
}


/* Check whether new player enters a valid name
 */
int valid_name(struct game_state *game, char *new_name) {
    // Check empty string
    if (strcmp(new_name, "") == 0) {
        return 0;
    }

    // Check already existed name
    for (struct client *p = game->head; p != NULL; p = p->next) {
        if (strcmp(p->name, new_name) == 0) {
            return 0;
        }
    }
    return 1;
}


/* Check whether current player's guess is valid
 */
int valid_guess(char *client_inbuf) {
    if ((strlen(client_inbuf) == 1) && (client_inbuf[0] >= 'a') && (client_inbuf[0] <= 'z')) {
        return 1;
    }
    return 0;
}


/* Check whether the current player made a right guess
 */
int right_guess(struct game_state *game, char guess) {
    int in_word = 0;
    int already_guessed = 0;
    int word_length = strlen(game->word);

    // Check whether guess is in word
    for (int i = 0; i < word_length; i++) {
        if (game->word[i] == guess) {
            in_word = 1;
        }
    }

    // Check whether guess is already gussed
    for (int i = 0; i < word_length; i++) {
        if (game->guess[i] == guess) {
            already_guessed = 1;
        }
    }

    // Return 1 if guess is in word and have not been guessed yet
    if ((in_word == 1) && (already_guessed == 0)) {
        return 1;
    }
    return 0;
}


/* Broadcast given message to each active player
 */
void broadcast(struct game_state *game, char *outbuf) {
    struct client *c = game->head;

    // Write message in oufbuf into each active player's socket
    while (c != NULL) {
        if (write(c->fd, outbuf, strlen(outbuf)) == -1) {
            remove_quitted(game, &(game->head), c);
        }
        c = c->next;
    }
}


/* Announce current game state to each active player
 */
void announce_turn(struct game_state *game, int type, struct client *player) {
    struct client *c = game->head;
    char msg[MAX_BUF] = {'\0'};

    if (type == 1) { // Announce everyone
        while (c != NULL) {
            // Write game state into each active player's socket
            status_message(msg, game);
            msg[MAX_BUF - 1] = '\0';
            if (write(c->fd, msg, strlen(msg)) == -1) {
                remove_quitted(game, &(game->head), c);
            }
            c = c->next;
        }

    } else if (type == 2) { // Only announce new_player
        // Write game state into new player's socket
        status_message(msg, game);
        msg[MAX_BUF - 1] = '\0';
        if (write(player->fd, msg, strlen(msg)) == -1) {
            remove_quitted(game, &(game->head), player);
        }
    }
}


void announce_sep(struct game_state *game, int type, struct client *player) {
    struct client *c = game->head;
    char msg[MAX_BUF] = {'\0'};

    // Announce all active players about the winner
    if (type == 1) {
        while (c != NULL) {
            if (c == player) {
                sprintf(msg, "Game over! You win!\r\n");
                if (write(c->fd, msg, strlen(msg)) == -1) {
                    remove_quitted(game, &(game->head), c);
                }
            } else {
                sprintf(msg, "Game over! %s won!\r\n", player->name);
                if (write(c->fd, msg, strlen(msg)) == -1) {
                    remove_quitted(game, &(game->head), c);
                }
            }
            c = c->next;
        }

    // Announce all previous players about the new player
    } else if (type == 2) {
        sprintf(msg, "%s has just joined\r\n", player->name);
        while (c != NULL) {
            if (c != player) {
                if (write(c->fd, msg, strlen(msg)) == -1) {
                    remove_quitted(game, &(game->head), c);
                }
            }
            c = c->next;
        }

    // Announce all players about current player
    } else if (type == 3) {
        while (c != NULL) {
            if (c == player) {
                sprintf(msg, "Your guess?\r\n");
                if (write(c->fd, msg, strlen(msg)) == -1) {
                    remove_quitted(game, &(game->head), c);
                }
            } else {
                sprintf(msg, "It's %s's turn.\r\n", player->name);
                if (write(c->fd, msg, strlen(msg)) == -1) {
                    remove_quitted(game, &(game->head), c);
                }
            }
            c = c->next;
        }
    }
}


/* Remove player who has entered a valid name from waiting list
 */
void remove_from_waitlist(struct client **new_players, struct client *player) {
    if (*new_players == player) {
        *new_players = (*new_players)->next;
    } else {
        for (struct client *c = *new_players; c != NULL; c = c->next) {
            if (c->next == player) {
                c->next = player->next;
            }
        }
    }
}


/* Add player to active player list
 */
void add_to_active(struct game_state *game, struct client *player, struct client **new_players) {
    char buf[MAX_BUF] = {'\0'};

    remove_from_waitlist(new_players, player);
    strcpy(player->name, player->inbuf);
    player->name[strlen(player->inbuf)] = '\0';
    player->next = game->head;
    game->head = player;

    // Only for the first player
    if (game->head->next == NULL) {
        game->has_next_turn = player;
    }

    // Announce other players that a new player has joined the game
    announce_sep(game, 2, player);
    // Announce new player current game state
    announce_turn(game, 2, player);

    // If new player is current player, ask for guess
    // If not, show current player
    if (game->has_next_turn == player) {
        sprintf(buf, "Your guess?\r\n");
        if (write(player->fd, buf, strlen(buf)) == -1) {
            remove_quitted(game, &(game->head), player);
        }
    } else {
        sprintf(buf, "It's %s's turn.\r\n", game->has_next_turn->name);
        if (write(player->fd, buf, strlen(buf)) == -1) {
            remove_quitted(game, &(game->head), player);
        }
    }
}


/*  Remove quitted players and announce inactive players
 */
void remove_quitted(struct game_state *game, struct client **players, struct client *player) {
    char msg[MAX_BUF] = {'\0'};

    // If is active player, print goodbye message in server and to other active players
    if (player->name[0] != '\0') {
        printf("Goodbye %s\n", player->name);
        sprintf(msg, "Goodbye %s\r\n", player->name);
        broadcast(game, msg);
    }

    // remove player and announce active players
    remove_player(players, player->fd);
    announce_sep(game, 3, game->has_next_turn);
}


/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}


int main(int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }

    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);

    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;

    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);

    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    // Pervent SIGPIPE terminates the server
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while (1) {
        // Buffer for different usages
        char buf[MAX_BUF] = {'\0'};

        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&new_players, p->fd);
            };
        }

        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be
         * valid.
         */
        int cur_fd;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                for (p = game.head; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        //TODO - handle input from an active client
                        if (game.has_next_turn == p) { // Current player gives a guess
                            // Read guess from socket using buffer
                            if (buf_read(cur_fd, p->inbuf) == 1) {
                                if (valid_guess(p->inbuf) == 1) { // Valid guess
                                    game.guesses_left--;
                                    game.letters_guessed[p->inbuf[0] - 'a'] = 1;

                                    int guess = right_guess(&game, p->inbuf[0]);
                                    if (guess == 1) { // Right guess
                                        // Announce current player
                                        sprintf(buf, "Good guess!\r\n");
                                        buf[13] = '\0';
                                        if (write(cur_fd, buf, strlen(buf)) == -1) {
                                            remove_quitted(&game, &(game.head), p);
                                        }

                                        // Announce all players
                                        sprintf(buf, "%s guesses: %c\r\n", p->name, p->inbuf[0]);
                                        buf[strlen(p->name) + 13] = '\0';
                                        broadcast(&game, buf);

                                        // Show guessed letter
                                        for (int i = 0; i < strlen(game.word); i++) {
                                            if (game.word[i] == p->inbuf[0]) {
                                                game.guess[i] = p->inbuf[0];
                                            }
                                        }

                                        // Announce all players about current state
                                        announce_turn(&game, 1, p);
                                        // Ask current player for guess
                                        announce_sep(&game, 3, game.has_next_turn);

                                        // Current player wins
                                        if (strcmp(game.word, game.guess) == 0) {
                                            printf("Game over. %s won!\n", p->name);

                                            // Notify all players about the winner
                                            announce_sep(&game, 1, p);
                                            sprintf(buf, "The word is: %s\r\n", game.word);
                                            buf[strlen(game.word) + 15] = '\0';
                                            broadcast(&game, buf);
                                            broadcast(&game, NEW_GAME_MSG);

                                            // Restart the game
                                            restart_game(&game, argv[1]);
                                            // Announce every player new game state
                                            announce_turn(&game, 1, p);
                                            // Ask current player for guess
                                            announce_sep(&game, 3, game.has_next_turn);
                                        }

                                    } else { // Wrong guess or already guessed
                                        printf("Letter %c is not in the word\n", p->inbuf[0]);

                                        // Announce current player
                                        sprintf(buf, "%c is not in the word\r\n", p->inbuf[0]);
                                        buf[22] = '\0';
                                        if (write(cur_fd, buf, strlen(buf)) == -1) {
                                            remove_quitted(&game, &(game.head), p);
                                        }

                                        // Announce all players
                                        sprintf(buf, "%s guesses: %c\r\n", p->name, p->inbuf[0]);
                                        buf[strlen(p->name) + 13] = '\0';
                                        broadcast(&game, buf);

                                        advance_turn(&game, p);
                                        // Announce all players about current state
                                        announce_turn(&game, 1, p);
                                        // Ask current player for guess
                                        announce_sep(&game, 3, game.has_next_turn);
                                    }

                                    // Game over with no player wins
                                    if (game.guesses_left == 0) {
                                        printf("Game over. No one wins.\n");

                                        // Notify all players that the game has ended
                                        broadcast(&game, OVER_MSG);
                                        sprintf(buf, "The wors is: %s\r\n", game.word);
                                        buf[strlen(game.word) + 15] = '\0';
                                        broadcast(&game, buf);
                                        broadcast(&game, NEW_GAME_MSG);

                                        // Restart the game
                                        restart_game(&game, argv[1]);
                                        // Announce every player new game state
                                        announce_turn(&game, 1, p);
                                        // Ask current player for guess
                                        announce_sep(&game, 3, game.has_next_turn);
                                    }

    			                      } else { // Invalid guess
    				                        if (write(cur_fd, INVALID_GUESS_MSG, (int) strlen(INVALID_GUESS_MSG)) == -1) {
                                        remove_quitted(&game, &(game.head), p);
                                    }
                                }
                            } else { // Player has quitted
                                advance_turn(&game, p);
                                remove_quitted(&game, &(game.head), p);
                            }

                        } else { // Non current player gives a guess
                            printf("Player %s tried to guess out of turn\n", p->name);
                            // Empty the socket
                            if (buf_read(cur_fd, p->inbuf) == 1) {
                                // Notify non current player
                                if (write(cur_fd, NOT_TURN_MSG, strlen(NOT_TURN_MSG)) == -1) {
                                    remove_quitted(&game, &(game.head), p);
                                }
                            } else { // Player has quitted
                                remove_quitted(&game, &(game.head), p);
                            }
                        }
                    }
                }

                // Check if any new player has entered a name
                for (p = new_players; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.
                        // Check whether new player entered a valid name
                        if (buf_read(cur_fd, p->inbuf) == 1) {
                            if (valid_name(&game, p->inbuf) == 1) {
                                // Add player to active player list
                                printf("%s has just joined.\n", p->inbuf);
                                add_to_active(&game, p, &new_players);
                            } else {
                                if (write(cur_fd, INVALID_NAME_MSG, strlen(INVALID_NAME_MSG)) == -1) {
                                    remove_quitted(&game, &new_players, p);
                                }
                            }
                        } else { // Player has quitted without entering a name
                            remove_quitted(&game, &new_players, p);
                        }
                    }
                }
            }
        }

        // All players have quitted
        if (game.head == NULL) {
            game.has_next_turn = NULL;
        }
        // Print next player in server
        if ((game.has_next_turn != NULL) && (game.head != NULL)) {
            printf("It's %s's turn.\n", game.has_next_turn->name);
        }
    }
    return 0;
}
