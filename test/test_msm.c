#include <stdio.h>
#include <stdlib.h>

void report_exec() {
}
int main() {
	char *msm_output = getenv("MSM_OUTPUT=fic.txt");
	if(msm_output == NULL) {
		FILE * f = fopen("fic.txt", "w");
		fprintf(f, "je peux poser une qs indiscrète?");
		fclose(f);
	}	
	printf("written in msm_output = fic.txt");

	///printf("%s", msm_output);
	return 0;
}
