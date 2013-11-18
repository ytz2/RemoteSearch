/*
 * Yanhua Liu (ytz2) CS820
 * client.c - client program that uses the socket interface to tcp
 *		  This client repeatedly prompts the user for the name
 *		  of a file, then sends the name, length and file data
 *		  to the server where it gets printed.
 *		must be linked with -lnsl and -lsocket on solaris
 */
#include "send_recv.h"
#include "client.h"

#define BUFSIZE		1024			/* size of data buffers */
#define TEXT_SIZE	 256			/* size of text buffer */

/*use the mysearch to build msg 1*/
void build_msg1(msg_one *msg1, search *mysearch) {
	msg1->options_flags = htonl(mysearch->options_flags);
	msg1->max_dir_depth = htonl(mysearch->max_dir_depth);
	msg1->line_buffer_size = htonl(mysearch->line_buffer_size);
	if (mysearch->line_buffer_size == DEFAULT_LINE_BUFFER)
		msg1->max_line_number = htonl(-1);
	else
		msg1->max_line_number = htonl(mysearch->max_line_number);
	msg1->column_number = htonl(mysearch->column_number);
	msg1->thread_limits = htonl(mysearch->thread_limits);
}

/* push it on the thread clean up stack
 * make sure once it exit, it would be
 * able to tell his parent he is done
 */
static void cleanup_client(void *arg) {
	search* mysearch;
	mysearch = (search*) arg;
	pthread_mutex_lock(&(mysearch->lock));
	mysearch->stk_count--;
	pthread_cond_signal(&(mysearch->ready));
	pthread_mutex_unlock(&(mysearch->lock));
}

/*
 * client function accept the remote and search
 * object to perform remote search
 */
void client(remote *rmt, search *mysearch) {
	int fd, n;
	unsigned int len;
	struct sockaddr server, client;
	enum header_types type;
	msg_one msg1;
	char *out_buffer;
	Statistics *temp;
	temp = NULL;

	/*prepare for message 1*/
	build_msg1(&msg1, mysearch);
	pthread_cleanup_push(cleanup_client,mysearch);
		/* ignore sigpipe signals -- handle the error synchronously */
		no_sigpipe();

		/* establish a connection to indicated server */
		fd = openclient(rmt->port, rmt->node, &server, &client);
		if (fd < 0)
			pthread_exit(NULL);
		/* we are now successfully connected to a remote server */
		/* send the first message */

		if (our_send_message(fd, OPTION_PARAMETER, sizeof(msg_one), &msg1)
				!= 0) {
			fprintf(stderr, "Fail to send options parameters\n");
			pthread_exit(NULL);
		}

		/* send the second message */
		if (our_send_message(fd, TO_SEARCH,
				(strlen(mysearch->search_pattern) + 1) * sizeof(char),
				mysearch->search_pattern) != 0) {
			fprintf(stderr, "Fail to send search string\n");
			pthread_exit(NULL);
		}

		/* send the third message */
		if (our_send_message(fd, REMOTE_NAME,
				(strlen(rmt->name) + 1) * sizeof(char), rmt->name) != 0) {
			fprintf(stderr, "Fail to send remote_name\n");
			pthread_exit(NULL);
		}

		out_buffer = pthread_getspecific(out_buffer_key);
		/* allocate memory to buffer*/
		if (out_buffer == NULL) {
			out_buffer = (char*) malloc(MAX_TCP_STD);
			pthread_setspecific(out_buffer_key, out_buffer);
		}

		/* receive the message 4,5 */

		while (1) {
			/* read next chunk of text then the text */
			len = MAX_TCP_STD;
			if ((n = our_recv_message(fd, &type, &len, out_buffer)) < 0)
				break;
			if (type == OUTPUT_STD)
				fprintf(stdout, "%s:%s/%s", rmt->node, rmt->port, out_buffer);
			else if (type == OUTPUT_ERR)
				fprintf(stderr, "%s:%s/%s", rmt->node, rmt->port, out_buffer);
			else if (type == STATISTICS_MSG) {
				temp = (Statistics *) out_buffer;
				trans_stat2recv(temp); /* avoid little/big endian issues*/
				pthread_mutex_lock(&(mysearch->lock));
				update_statistics_sock(&(mysearch->statistics), temp);
				pthread_mutex_unlock(&(mysearch->lock));
				break;
			} else
				break;
			fflush(stderr);
			fflush(stdout);
		}

		/* explicitly pop*/
		pthread_cleanup_pop(1);
		/* close the connection to the server */
	if (close(fd) < 0) {
		perror("client close");
		pthread_exit(NULL);
	}
} /* client */

/* client agent function to be used in
 * thread creations
 */
void *client_agent(void *para) {
	int errcode;
	remote *rmt;
	search *mysearch;
	Client_para *parameter;
	/*
	 * create the key
	 */
	pthread_once(&init_done, thread_init);
	/*extract parameters*/
	parameter = (Client_para*) para;
	rmt = parameter->rmt;
	mysearch = parameter->mysearch;
	errcode = pthread_detach(pthread_self());
	if (errcode != 0) {
		fprintf(stderr, "pthread_detach client agent: %s\n", strerror(errcode));
	}
	client(rmt, mysearch);
	free(parameter);
	return NULL;
}

/* vi: set autoindent tabstop=8 shiftwidth=8 : */
