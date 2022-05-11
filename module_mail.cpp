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
			rcpt_user_num = 0; //vaciamos el número de receptores
			memset(rcpt_user, 0, sizeof(rcpt_user)); //x si acaso vaciamos la matriz de receptores
			mail_stat = 2;
		} else { //si tenía un estado distinto a 1, muestra error de comando no implementado
			send_data(client_sockfd, reply_code[15]);
		}
	} //mismo proceso para el mail from 
	else if (strncmp(request, "mail from", 9) == 0) {
		if (mail_stat == 2) { //comprueba que este log
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
		} else {
			send_data(client_sockfd, "503 Error: send HELO first\r\n");
		}
	} else if (strncmp(request, "rcpt to", 7) == 0) {
		//comprueba que venga de un mail from o de el rcpt to ya que puede haber varios receptores
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
		send_data(client_sockfd, reply_code[5]);
		close(client_sockfd); //closeamos server
		pthread_exit((void*)1); //cerramos el hilo
	}
}

// simplemente un send que imprime la respuesta
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
	char file[80], tp[27];

	for (i = 0; i < rcpt_user_num; i++) {
		strcpy(file, dData); //cogemos la dirección del data folder
		strcat(file, rcpt_user[i]); //concatenamos el usuario de recepción
		if (access(file,0) == -1) {
			mkdir(file,0777);
		}
		time_t timer;
		char buffer[26];
		struct tm* tm_info;
		//obtengo la fecha
		timer = time(NULL);
		//le pongo el formato adecuado
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		puts(buffer);

		int i = 0;
		string aux = "/" + to_string(i);
		const char *c_aux = aux.c_str();
		strcat(file, c_aux);

		while( access( file, F_OK ) == 0 ){ //nos vamos desplazando hasta encontrar el nº de correo que toque
			file[strlen(file)-strlen(c_aux)] = '\0'; //ponemos el /0 para eliminar el ultimo id 
			i++;
			aux = "/" + to_string(i);
			const char *c_aux = aux.c_str();
			strcat(file, c_aux);
		}
		printf("%s\n\n", file);
		FILE* fp = fopen(file, "w+"); //abrimos el nuevo fichero para escribir el correo
		if (fp != NULL) {
			string header;
			header += "Mail from: " ;
			header += from_user;
			header += "\n";
			fwrite(header.c_str(),1,header.size(), fp); //escribimos la info
			fwrite(buf, sizeof(char), strlen(buf), fp); //escribimos la info
			fwrite(buffer, 1, strlen(buffer), fp); //escribimos la fecha del mensaje

			fclose(fp); //cerramos el fichero
		} else {
			cout << "File open error!" << endl;
		}
	}
	//enviamos que todo bien
	send_data(sockfd, reply_code[6]);
}


