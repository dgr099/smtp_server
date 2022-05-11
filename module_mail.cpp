#include "module_mail.h"
#include "module_user.h"

//función que procesa los correos, es a la que se manda cuando se acepta conexión
void *mail_proc(void* param) {
	int client_sockfd, len;
	char buf[BUF_SIZE];
	//cojemos el descriptor del client socket y lo pasamos a entero
	//pasamos param a puntero de entero y cojemos su contenido
	client_sockfd = *(int*) param;

	send_data(client_sockfd, reply_code[4]); //le mandamos el mensaje 4 (200 ok)
	mail_stat = 1;
	//bucle infinito
	while (1) {
		//Rellenamos con 0 el buffer
		memset(buf, 0, sizeof(buf));
		len = recv(client_sockfd, buf, sizeof(buf), 0);
		if (len > 0) {
			cout << "Recibí esto: " << buf;
			respond(client_sockfd, buf); //mandamos a respond que se encarga del strcmp
		} else {
			sleep(1); //esperamos un ratillo
			continue;
		}
	}
	return NULL;
}

//al recibir info nos ponemos a analizarla
void respond(int client_sockfd, char* request) {
	char output[1024];
	//el output a 0 x si las moscas u know
	memset(output, 0, sizeof(output));
	//pasamos la request a minúsculas
	for(int i=0, size=strlen(request); i!=size; i++){
		request[i] = tolower(request[i]);
	}
	//smtp
	//comprobamos si el comando esperado es un helo
	if (strncmp(request, "helo", 4) == 0) {
		if (mail_stat == 1) {
			send_data(client_sockfd, reply_code[6]);
			rcpt_user_num = 0;
			memset(rcpt_user, 0, sizeof(rcpt_user));
			mail_stat = 2;
		} else { //si tenía un estado distinto a 1, muestra error de comando no implementado
			send_data(client_sockfd, reply_code[15]);
		}
	} else if (strncmp(request, "mail from", 9) == 0) {
		if (mail_stat == 2 || mail_stat == 13) {
			char *pa, *pb;
			pa = strchr(request, '<');
			pb = strchr(request, '>');
			strncpy(from_user, pa + 1, pb - pa - 1);
			if (check_user()) {
				send_data(client_sockfd, reply_code[6]);
				mail_stat = 3;
			} else {
				send_data(client_sockfd, reply_code[15]);
			}
		} else if (mail_stat == 12) {
			send_data(client_sockfd, reply_code[23]);
		} else {
			send_data(client_sockfd, "503 Error: send HELO/EHLO first\r\n");
		}
	} else if (strncmp(request, "rcpt to", 7) == 0) {
		if ((mail_stat == 3 || mail_stat == 4) && rcpt_user_num < MAX_RCPT_USR) {
			char *pa, *pb;
			pa = strchr(request, '<');
			pb = strchr(request, '>');
			strncpy(rcpt_user[rcpt_user_num++], pa + 1, pb - pa - 1);
			send_data(client_sockfd, reply_code[6]);
			mail_stat = 4;
		} else {
			send_data(client_sockfd, reply_code[16]);
		}
	} else if (strncmp(request, "data", 4) == 0) {
		if (mail_stat == 4) {
			send_data(client_sockfd, reply_code[8]);
			mail_data(client_sockfd);
			mail_stat = 5;
		} else {
			send_data(client_sockfd, reply_code[16]);
		}
	} else if (strncmp(request, "rset", 4) == 0) {
		mail_stat = 1;
		send_data(client_sockfd, reply_code[6]);
	} else if (strncmp(request, "noop", 4) == 0) {
		send_data(client_sockfd, reply_code[6]);
	} else if (strncmp(request, "quit", 4) == 0) { //cuando quiere terminar
		mail_stat = 0; //reiniciamos el mail stat
		user_quit(); //hacemos un user quit
		send_data(client_sockfd, reply_code[5]);
		close(client_sockfd);
		pthread_exit((void*)1);
	}
	//esmpt
	else if (strncmp(request, "ehlo", 4) == 0) {
		if (mail_stat == 1) {
			send_data(client_sockfd, reply_code[24]);
			mail_stat = 12; //mail stat en 12 para indicar que usa el extendido
		} else {
			send_data(client_sockfd, reply_code[15]);
		}
	} else if (strncmp(request, "auth login", 10) == 0 || strncmp(request, "auth login plain", 10) == 0 || strncmp(request, "auth=login plain", 10) == 0) {
		auth(client_sockfd); //pon el mail stat en 13 para indicar que se ha logeado
	} else {
		send_data(client_sockfd, reply_code[15]);
	}
}

// send data by socket
void send_data(int sockfd, const char* data) {
	if (data != NULL) {
		send(sockfd, data, strlen(data), 0);
		cout << "Reply stream: " << data;
	}
}

// recieve mail contents
void mail_data(int sockfd) {
	sleep(1);
	char buf[BUF_SIZE];
	memset(buf, 0, sizeof(buf));
	recv(sockfd, buf, sizeof(buf), 0); // recieve mail contents
	cout << "Mail Contents: \n" << buf << endl;

	//mail content and format check

	//mail content store
	int tm = time(NULL), i;
	char file[80], tp[20];

	for (i = 0; i < rcpt_user_num; i++) {
		strcpy(file, dData);
		strcat(file, rcpt_user[i]);
		if (access(file,0) == -1) {
			mkdir(file,0777);
		}
		sprintf(tp, "/%d", tm);
		strcat(file, tp);

		FILE* fp = fopen(file, "w+");
		if (fp != NULL) {
			fwrite(buf, 1, strlen(buf), fp);
			fclose(fp);
		} else {
			cout << "File open error!" << endl;
		}
	}
	send_data(sockfd, reply_code[6]);
}

// decode base64 streams
char *base64_decode(char *s) { 
	char *p = s, *e, *r, *_ret;
	int len = strlen(s);
	unsigned char i, unit[4];

	e = s + len;
	len = len / 4 * 3 + 1;
	r = _ret = (char *) malloc(len);

	while (p < e) {
		memcpy(unit, p, 4);
		if (unit[3] == '=')
			unit[3] = 0;
		if (unit[2] == '=')
			unit[2] = 0;
		p += 4;

		for (i = 0; unit[0] != B64[i] && i < 64; i++)
			;
		unit[0] = i == 64 ? 0 : i;
		for (i = 0; unit[1] != B64[i] && i < 64; i++)
			;
		unit[1] = i == 64 ? 0 : i;
		for (i = 0; unit[2] != B64[i] && i < 64; i++)
			;
		unit[2] = i == 64 ? 0 : i;
		for (i = 0; unit[3] != B64[i] && i < 64; i++)
			;
		unit[3] = i == 64 ? 0 : i;
		*r++ = (unit[0] << 2) | (unit[1] >> 4);
		*r++ = (unit[1] << 4) | (unit[2] >> 2);
		*r++ = (unit[2] << 6) | unit[3];
	}
	*r = 0;

	return _ret;
}

