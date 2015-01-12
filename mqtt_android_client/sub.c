/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MQTTPacket.h"
#include "transport.h"

#include "src/tools.h"


/* This is in order to get an asynchronous signal to stop the sample,
as the code loops waiting for msgs on the subscribed topic.
Your actual code will depend on your hw and approach*/
#include <signal.h>

int toStop = 0;
void print_usage(void);
int getopts(int argc, char** argv);

struct Options
{
	char* host;         /**< connection to system under test. */
	int port;
	char *id;
	int keepalive;
	int topic_qos;
	char* topic;
	int topic_count;
	bool clean_session;
	bool verbose;
	char* username;
	char* password;
	char *will_payload;
	long will_payloadlen;
	int will_qos;
	bool will_retain;
	char *will_topic;
} options =
{
	"localhost",	//char* host;  
	61883,			//int port;
	NULL,	//char *id;
	20,				//int keepalive;
	0,				//int topic_qos;
	NULL,			//char* topic;
	0,				//int topic_count;
	true,			//bool clean_session;
	false,			//bool verbose;
	NULL,			//char* username;
	NULL,			//char* password;
	NULL,			//char *will_payload;
	0,				//long will_payloadlen;
	0,				//int will_qos;
	false,			//bool will_retain;
	NULL,			//char *will_topic;
};

char pre_build_id[16];

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}

void stop_init(void)
{
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
}
/* */

int main(int argc, char *argv[])
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	int mysock = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	int req_qos = 0;
	char* payload = "mypayload";
	int payloadlen = strlen(payload);
	int len = 0;
	// char *host = "172.22.198.174";
	// int port = 61883;
	int timeoutcnt = 0;


	stop_init();

	rc = getopts(argc,argv);
	if ( rc != 0)
	{
		goto exit;
	}

	req_qos = options.topic_qos;
	


	mysock = transport_open(options.host, options.port);
	if(mysock < 0)
		return mysock;

	printf("Sending to host %s port %d\n", options.host, options.port);

	data.clientID.cstring = options.id;
	data.keepAliveInterval = options.keepalive;
	data.cleansession = options.clean_session;
	data.username.cstring = options.username;
	data.password.cstring = options.password;

	len = MQTTSerialize_connect(buf, buflen, &data);
	rc = transport_sendPacketBuffer(mysock, buf, len);

	/* wait for connack */
	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{
		unsigned char sessionPresent, connack_rc;

		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n", connack_rc);
			goto exit;
		}
	}
	else
		goto exit;
	printf("connected\n");
	/* subscribe */
	topicString.cstring = options.topic;
	// topicString.cstring = "zhaowei/#";
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);

	rc = transport_sendPacketBuffer(mysock, buf, len);
	if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK) 	/* wait for suback */
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
		if (granted_qos != options.topic_qos)
		{
			printf("granted qos != %d, %d\n", options.topic_qos, granted_qos);
			goto exit;
		}
	}
	else
		goto exit;
	printf("subscribed\n");

	while (!toStop)
	{
		/* transport_getdata() has a built-in 1 second timeout,
		your mileage will vary */
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
		{
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char* payload_in;
			int rc;
			MQTTString receivedTopic;

			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
					&payload_in, &payloadlen_in, buf, buflen);

			if (options.verbose)
			{
				printf("%s %.*s\n", options.topic, payloadlen_in, payload_in);
			}else
			{
				printf("%.*s\n", payloadlen_in, payload_in);
			}
			
		}else if (++timeoutcnt > options.keepalive)
		{
			len = MQTTSerialize_pingreq(buf, buflen);
			rc = transport_sendPacketBuffer(mysock, buf, len);
			timeoutcnt = 0;
		}
	}

	printf("disconnecting\n");
	len = MQTTSerialize_disconnect(buf, buflen);
	rc = transport_sendPacketBuffer(mysock, buf, len);

exit:
	transport_close(mysock);

	return 0;
}





void print_usage(void)
{
	int major, minor, revision;

	printf("\n\nSimple mqtt client that will subscribe to a single topic and print all messages it receives.\n");

	printf("Usage: sub [-c] [-h host] [-k keepalive] [-p port] [-q qos] [-R] -t topic ...\n");

	printf("       MQTT Client sub --help\n\n");

	printf(" -c : disable 'clean session' (store subscription and pending messages when client disconnects).\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_sub_ appended with the process id.\n");
	printf(" -k : keep alive in seconds for this client. Defaults to 60.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to 0.\n");
	printf(" -t : mqtt topic to subscribe to. May be repeated multiple times.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -v : print published messages verbosely.\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" --help : display this message.\n");
	printf(" --quiet : don't print error messages.\n");
	printf(" --will-payload : payload for the client Will, which is sent by the broker in case of\n");
	printf("                  unexpected disconnection. If not given and will-topic is set, a zero\n");
	printf("                  length message will be sent.\n");
	printf(" --will-qos : QoS level for the client Will.\n");
	printf(" --will-retain : if given, make the client Will retained.\n");
	printf(" --will-topic : the topic on which to publish the client Will.\n\n");

}


int getopts(int argc, char** argv)
{
	int i;
	int client_id=0;

	for(i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")){
			if(i==argc-1){
				fprintf(stderr, "Error: -p argument given but no port specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.port = atoi(argv[i+1]);
				if(options.port<1 || options.port>65535){
					fprintf(stderr, "Error: Invalid port given: %d\n", options.port);
					print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--disable-clean-session")){
			options.clean_session = false;
		}else if(!strcmp(argv[i], "--help")){
			print_usage();
			return 0;
		}else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host")){
			if(i==argc-1){
				fprintf(stderr, "Error: -h argument given but no host specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.host = argv[i+1];
			}
			i++;
		// }else if(!strcmp(argv[i], "--insecure")){
		// 	insecure = true;
		}else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--id")){
			if(i==argc-1){
				fprintf(stderr, "Error: -i argument given but no id specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.id = argv[i+1];
				client_id=1;
			}
			i++;
		}else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepalive")){
			if(i==argc-1){
				fprintf(stderr, "Error: -k argument given but no keepalive specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.keepalive = atoi(argv[i+1]);
				if(options.keepalive>65535){
					fprintf(stderr, "Error: Invalid keepalive given: %d\n", options.keepalive);
					print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: -q argument given but no QoS specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.topic_qos = atoi(argv[i+1]);
				if(options.topic_qos<0 || options.topic_qos>2){
					fprintf(stderr, "Error: Invalid QoS given: %d\n", options.topic_qos);
					print_usage();
					return 1;
				}
			}
			i++;
		// }else if(!strcmp(argv[i], "--quiet")){
		// 	options.quiet = true;
		// }else if(!strcmp(argv[i], "-R")){
		// 	options.no_retain = true;
		}else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: -t argument given but no topic specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.topic_count++;
				options.topic = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-u") || !strcmp(argv[i], "--username")){
			if(i==argc-1){
				fprintf(stderr, "Error: -u argument given but no username specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.username = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")){
			options.verbose = 1;
		}else if(!strcmp(argv[i], "-P") || !strcmp(argv[i], "--pw")){
			if(i==argc-1){
				fprintf(stderr, "Error: -P argument given but no password specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.password = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "--will-payload")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-payload argument given but no will payload specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.will_payload = argv[i+1];
				options.will_payloadlen = strlen(options.will_payload);
			}
			i++;
		}else if(!strcmp(argv[i], "--will-qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-qos argument given but no will QoS specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.will_qos = atoi(argv[i+1]);
				if(options.will_qos < 0 || options.will_qos > 2){
					fprintf(stderr, "Error: Invalid will QoS %d.\n\n", options.will_qos);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--will-retain")){
			options.will_retain = true;
		}else if(!strcmp(argv[i], "--will-topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-topic argument given but no will topic specified.\n\n");
				print_usage();
				return 1;
			}else{
				options.will_topic = argv[i+1];
			}
			i++;
		}else{
			fprintf(stderr, "Error: Unknown option '%s'.\n",argv[i]);
			print_usage();
			return 1;
		}
	}

	if (options.topic_count  == 0)
	{
		fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
		print_usage();
		return 1;
	}

	if (client_id == 0)
	{
		// char id[16];
		// printf("id is null\n");
		if (getID(pre_build_id) != 0)
		{
			return 1;
		}
		options.id = pre_build_id;
		printf("\n##use prebuild id: %s##\n", options.id);
	}

	return 0;

}




