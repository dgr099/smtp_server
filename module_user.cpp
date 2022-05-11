#include "module_user.h"
#include "module_mail.h"

// find if user exists
int check_user() {
	FILE* fp;
	char file[80] = "";
	char data[60];

	strcpy(file, dData);
	strcat(file, dUInfo);
	printf("%s\n", file);
	fp = fopen(file, "r");
	while (fgets(data, sizeof(data), fp) != NULL) {
		if (strncmp(from_user, data, strlen(from_user)) == 0) // valid user
			fclose(fp);
			return 1;
	}
	fclose(fp);
	return 0;
}
