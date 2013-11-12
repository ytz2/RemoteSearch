/* client1tcp.c - client program that uses the socket interface to tcp
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
void build_msg1(msg_one *msg1,search *mysearch)
{
	msg1->options_flags=htonl(mysearch->options_flags);
	msg1->max_dir_depth=htonl(mysearch->max_dir_depth);
	msg1->line_buffer_size=htonl(mysearch->line_buffer_size);
	if (mysearch->line_buffer_size==MAX_LINE_BUFFER)
		msg1->max_line_number=htonl(-1);
	else
		msg1->max_line_number=htonl(mysearch->max_line_number);
	msg1->column_number=htonl(mysearch->column_number);
	msg1->thread_limits=htonl(mysearch->thread_limits);
}

void
client(remote *rmt,search *mysearch)
{
	int			fd,n;
	unsigned int len;
	struct sockaddr		server, client;
	struct sockaddr_in	*iptr;
	char			text_buf[TEXT_SIZE];
	enum header_types	type;
	msg_one msg1;
	char *out_buffer;

	/*prepare for message 1*/
	build_msg1(&msg1,mysearch);

	/* ignore sigpipe signals -- handle the error synchronously */
	no_sigpipe();

	/* establish a connection to indicated server */
	fd = openclient(rmt->port,rmt->node, &server, &client);
	if (fd < 0)
		exit(EXIT_FAILURE);
	/* we are now successfully connected to a remote server */
	iptr = (struct sockaddr_in *)&client;
	if (inet_ntop(iptr->sin_family, &iptr->sin_addr, text_buf, TEXT_SIZE)
								== NULL) {
		perror("inet_ntop client");
		exit(EXIT_FAILURE);
	}
	printf("client1tcp at IP address %s port %d\n", 
		text_buf, ntohs(iptr->sin_port));
	iptr = (struct sockaddr_in *)&server;
	if (inet_ntop(iptr->sin_family, &iptr->sin_addr, text_buf, TEXT_SIZE)
								== NULL) {
		perror("inet_ntop server");
		exit(EXIT_FAILURE);
	}
	printf("client1tcp connected to server at IP address %s port %d\n",
		text_buf, ntohs(iptr->sin_port));

	/* send the first message */

	if (our_send_message(fd, OPTION_PARAMETER,sizeof(msg_one),&msg1) != 0)
	{
		fprintf(stderr,"Fail to send options parameters\n");
		pthread_exit(NULL);
	}

	/* send the second message */
	if (our_send_message(fd, TO_SEARCH,(strlen(mysearch->search_pattern)+1)*sizeof(char),mysearch->search_pattern) != 0)
	{
		fprintf(stderr,"Fail to send search string\n");
		pthread_exit(NULL);
	}

	/* send the third message */
	if (our_send_message(fd, REMOTE_NAME,(strlen(rmt->name)+1)*sizeof(char),rmt->name) != 0)
	{
		fprintf(stderr,"Fail to send remote_name\n");
		pthread_exit(NULL);
	}


	out_buffer = pthread_getspecific(out_buffer_key);
		/* allocate memory to buffer*/
	if (out_buffer == NULL) {
		out_buffer = (char*) malloc(MAX_TCP_STD);
		pthread_setspecific(out_buffer_key, out_buffer);
	}

	/* receive the message 4,5 */

	                /* first message should be the file name */

	while (1) {
		/* read next chunk of text then the text */
		len =MAX_TCP_STD;
		if ((n = our_recv_message(fd, &type, &len,
				out_buffer)) < 0)
			break;
		if (type !=OUTPUT_STD && type!=OUTPUT_ERR)
			break;
		if (type==OUTPUT_STD)
			fprintf(stdout, "%s:%s/%s\n",rmt->node,rmt->port,out_buffer);
		fflush(stderr);
		fflush(stdout);
	}


	/* close the connection to the server */
	if (close(fd) < 0) {
		perror("client1tcp close");
		exit(EXIT_FAILURE);
	}
}	/* client */


void *client_agent(void *para)
{
	int errcode;
	remote *rmt;
	search *mysearch;
	Client_para *parameter;
	/*
	 * create the key
	 */
	pthread_once(&init_done, thread_init);
	parameter=(Client_para*)para;
	rmt=parameter->rmt;
	mysearch=parameter->mysearch;
	errcode = pthread_detach(pthread_self());
	if (errcode != 0) {
		fprintf(stderr, "pthread_detach client agent: %s\n",
			strerror(errcode));
	}
	client(rmt,mysearch);
	free(parameter);
	return NULL;
}

/* vi: set autoindent tabstop=8 shiftwidth=8 : */
