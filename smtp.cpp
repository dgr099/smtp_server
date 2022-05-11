#include "smtp.h"

int mail_stat = 0;
int rcpt_user_num = 0;
char from_user[64] = "";
char rcpt_user[MAX_RCPT_USR][30] = {""};
int server_sockfd;
void quit(int arg);

int main(int argc,char* argv[]) {
	signal(SIGINT, quit);  //para que cierre el
	int client_sockfd; //sock para cliente smtp
	socklen_t sin_size;
	struct sockaddr_in server_addr, client_addr;
    //similar al proceso de redes 2
	memset(&server_addr, 0, sizeof(server_addr));
	//creamos un socket de stream
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error en creación de socket\n");
		exit(1);
	}

	//le ponemos los atribtos al socket
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(server_addr.sin_zero), 8);

	//lo bindeamos
	if (bind(server_sockfd, (struct sockaddr *) &server_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("Error en el bind\n");
		exit(1);
	}

	//listening
	if (listen(server_sockfd, MAX_CLIENTS - 1) == -1) {
		perror("listen error！\n");
		exit(1);
	}
	//ya está el server corriendo lets GOOOO
	cout << "================================================================\n";
	cout << "-Servidor smtp iniciado by G4R1 en puerto: " << PORT << endl;
	sin_size = sizeof(client_addr);
	//bucle infinito aceptando peticiones
	while (1) {
		//se queda esperando recibir algna conexión
		if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &sin_size)) == -1) {
			//si hay un error (debug)
			//perror("Bro, el accept falló\n");
			sleep(1);
			continue;
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

		//si acepta la conexión imprime la ip y la fecha
		cout << "He recibido una conexión desde: " 
			 << inet_ntoa(client_addr.sin_addr) << " con fecha: " 
			 << buffer << endl;
		//se crea el hilo
		pthread_t id;
		//ha funcionar, se lo mando al mail_proc
		pthread_create(&id, NULL, mail_proc, &client_sockfd);
		pthread_join(id, NULL);
	}
	close(client_sockfd);
	return 0;
}


//función pa terminar el server (closea el server)
void quit(int arg) {
	if (arg)
		printf("\nDetectamos señal (%d). G4R1 se va a dormir wenas noches...\n\n", arg);
	close(server_sockfd);
	exit(1);
}

