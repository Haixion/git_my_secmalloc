/*	; ---------------------------
	Auhor: HEXION @Haixion (git)
	Last update: 06/04/2022
	
	@2600's project
	Duo:	Bastos | realloc, calloc
		Hexion | malloc, free, makefile, static & dynamic libraries
	; ---------------------------
*/

#include "../include/my_secmalloc.h"

/*	------------------- MEMORY LEAK ------------------------
	If we don't deallocate the dynamic memory, it'll reside in the heap section.
	It is called memory leak. It'll reduce the system performances by reducing
	the amount of available memory. 

	Total size: K, N = byte of memory allocated dynamically (= in heap section)
		M = number of time the code is executed
		thus, M * N = bytes of memory consumed by the program for a
		M number of time the code is executed

	    HEAP                                + ---------- +   ^   M * N > K
	+ --------- +  ^    + ---------- +  ^   |   N Bytes  |   |
	|           |  |    |  Available |  |   + ---------- +   |
	|           |  |    |   memory   |  |   |   N Bytes  |   |
	| Available |  |    + ---------- +  |   + ---------- +   |
	|   memory  |  |    |   N Bytes  |  |   |   N Bytes  |   |
	|           |  |    + ---------- +  |   + ---------- +   |
	|           |  |    |   N Bytes  |  |   |   N Bytes  |   |   
	+ --------- +  |    + ---------- +  |   + ---------- +   |
	|  N Bytes  |  |    |   N Bytes  |  |   |   N Bytes  |   |
	+ --------- +  |    + ---------- +  |   + ---------- +   |
	   (NORMAL)           (LOW PERF.)        (SYSTEM CRASH)
	   		(LOW AVAILABLE MEMORY)	 (NO AVAILABLE MEMORY 
   						FOR FURTHER OP.)
	---------------------------------------------------------	*/
void memory_leak() {
	        FILE *f = fopen("memory_leak.txt","w");
		t_chunk *temp = head;
		if(f == NULL) {
			return;
		}
		else if(!f) {
		fprintf(f, "==========================================\n\t\tMEMORY LEAK\n==========================================\n");
		printf("==========================================\n\t\tMEMORY LEAK\n==========================================\n");
		}
		while (temp != NULL)
		{
			printf("addr : %p, length : %ld\n",temp->m_addr, temp->m_size);
			fprintf(f,"addr : %p, length : %ld\n",temp->m_addr, temp->m_size);
			
			if (munmap(temp->m_addr, temp->m_size)!=0)
			{
				printf("Error on munmap");
				exit(EXIT_FAILURE);	
			}
			
			head = temp;
			temp = temp->m_next;
			
			if (munmap(head,sizeof(t_chunk))!=0)
			{
				printf("Error on munmap2");
				exit(EXIT_FAILURE);	
			}
			
		}
		head = NULL;
		fclose(f);
}

void	*my_malloc(size_t size) {
	FILE *historic = fopen("historic.txt", "w");
	/*	ATEXIT
		Allows you to add a function that will be executed at 
		the exit of the main function or at the exit of the 
		process (of the program) via a call to the exit function.
		Successive calls to atexit create a register of functions 
		that are executed in "last in, first out" (LIFO) order.
		atexit returns 0 on success, or a non-zero value if an 
		error occurs.
	*/
	if (call_memory_leak == 0)
	{
		atexit(memory_leak);
		call_memory_leak = 1;
	}
	void *addr = NULL;
	if(size == 0) {
		printf("size is null\n");
		exit(EXIT_FAILURE);
	}
	
	addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	fprintf(historic, "Allocated memory for the pointer\n");
	
	if(addr == MAP_FAILED)
	{
		printf("error during the mmap\n");
		fprintf(historic, "Error during the malloc | MAP_FAILED");
		exit(EXIT_FAILURE);
	}
	if(msm_output != NULL)
	{
		FILE *f = fopen(msm_output,"a+");
		fprintf(f,"Malloc - addr : %p, allocated length (size) : %ld\n",addr,size);
		fclose(f);
	}
	if (head == NULL)
	{
		t_chunk *new = mmap(NULL, sizeof(t_chunk), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		fprintf(historic, "Created a new chunk as there was no linked list yet.\n");
		new->m_addr = addr;
		new->m_size = size;
		//new->m_occupe = 1;
		new->m_next = NULL;
		new->m_prev = NULL;
		head = new;
	}
	else  {
		t_chunk *temp = head;
		
		while(temp->m_next != NULL)
			temp = temp->m_next;

		t_chunk *new = mmap(NULL, sizeof(t_chunk), PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		fprintf(historic, "malloc for a new chunk");
		new->m_addr = addr;
		new->m_size = size;
		//new->m_occupe = 1;
		new->m_next = NULL;
		new->m_prev = temp;
		temp->m_next = new;
	}
	return addr;
	fprintf(historic, "Returned %p", addr);
	fclose(historic);
}

/*	------------------- DOUBLE FREE CHECK -------------------
	If we free the same pointer two or more time, then the
	behavior is undefined. So if we free the same pointer 
	which is freed already, the program will stop its exec
	---------------------------------------------------------	*/
void my_free(void *ptr) {
	int error = 0;
	t_chunk *temp = head;
	
	while(temp != NULL && temp->m_addr != ptr)
		temp = temp->m_next;
		
	if (temp == NULL) {
		printf("This pointer does not exist\n");
		exit(EXIT_FAILURE);
	}
	
	if (temp->m_next != NULL && temp->m_prev != NULL)
	{
		temp->m_prev->m_next = temp->m_next;
		temp->m_next->m_prev = temp->m_prev;
	}
	else if (temp->m_next == NULL && temp->m_prev!=NULL)
		temp->m_prev->m_next = temp->m_next;
		
	else if (temp->m_next != NULL && temp->m_prev==NULL)
	{
		head = temp->m_next;
		temp->m_next->m_prev = temp->m_prev;
	}
	else head = NULL;

	if (munmap(temp->m_addr,temp->m_size) != 0)
	{
		printf("Error on munmap");
		exit(EXIT_FAILURE);	
	}
	if (munmap(temp,sizeof(t_chunk)) != 0)
	{
		printf("Error on munmap2");
		exit(EXIT_FAILURE);	
	}
	if(msm_output != NULL)
	{
		FILE *f = fopen(msm_output,"a+");
		fprintf(f,"Free : addr = %p, desallocated length = %ld\n",temp->m_addr, temp->m_size);
		fclose(f);
	}
}

/*	----------------------------------------------------
				@BASTOS
	realloc & calloc
	----------------------------------------------------	*/



void *my_calloc(size_t size_el, size_t nb_el) {
	if (call_memory_leak == 0)     // Linked through atexit
	{
		call_memory_leak = 1;
		atexit(memory_leak);    // Verify a function call in order to check memory_leak through atexit by single-use call of it.
	}
	
	void *addr = NULL;

	if(size_el == 0) {
		printf("size is null\n");
		return(NULL);
	}
	
    if(nb_el == 0) {
		printf("number of elements is null\n");
		return(NULL);
	}

    addr = my_malloc(nb_el * size_el);

    if(addr){

        char *byte = addr;

        while(byte != addr+size_el*nb_el){
            //memset(&byte, 0, nb_el);
			*byte=0;
            printf("Value test per byte : %c\n", *byte);
            byte++;
        }

    }
	return(addr);
}

void *my_realloc(void *ptr, size_t size){
    t_chunk *new_alloc = head;      // Running through the meta data linked list

	if(new_alloc == NULL){
        printf("Erreur : header null\n");  
        call_memory_leak = 1;
        return(MAP_FAILED);
    }

    while(new_alloc->m_addr!=ptr){

        if(new_alloc == NULL){
            printf("Erreur : bloc non trouvé\n");  
            call_memory_leak = 1;
            return(MAP_FAILED);
        }

		new_alloc = new_alloc->m_next;

    }

    new_alloc->m_addr = mremap(new_alloc->m_addr, new_alloc->m_size, size, MREMAP_MAYMOVE);
	
	if(new_alloc->m_addr == MAP_FAILED){
		printf("Erreur : Aucun espace suffisant n'a été trouvé (MAP_FAILED)\n");
		call_memory_leak = 1;
		return(MAP_FAILED);
	}

    new_alloc->m_size = size;
    printf("Reallocation mémoire à l'espace mémoire : %p\n", new_alloc->m_addr);
    return(new_alloc->m_addr);
}
