#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>


// typedef __int32 int32_t;
// typedef unsigned __int32 uint32_t;
// typedef __int16 int16_t;
// typedef unsigned __int16 uint16_t;
// typedef __int8 int8_t;
// typedef unsigned __int8 uint8_t;

struct fs_header {
	char title[8];
	uint32_t size;		// size = 0x11 0x22 0x33 0x44 big endian
	uint32_t checksum;
	char volume_name[];	// we could use 'char * volume_name' in this way the compiler doenst recognize the thing that comes after checksum to have value, but instead to have a memory address. 
						// if we use this that way it will give a segmentation fault on reading the volume_name
						// the best solution is to put 'char volume_name []' to receive the first address after checksum as the first volume_name address.
};

struct file_header {
	uint32_t next_filehdr;
	uint32_t spec_info;
	uint32_t size;
	uint32_t checksum;
	char file_name[];	// same thing as volume_name
};

uint32_t round16(uint32_t n){
	if(n%16 == 0) return n;
	return (n - n%16 + 16);
}

uint32_t read32(char ptr[4]){		// big endian to little endian
	uint32_t out = 0;
	out = (uint32_t) ((uint8_t)ptr[0] << 8*3 | (uint8_t)ptr[1] << 2*8 | (uint8_t)ptr[2] << 1*8 | (uint8_t)ptr[3]) ;
	return out;
}


int str_len(char* p){
	int i = 0;
	for(; p[i] != '\0'; i++){}
	return i;
}

int str_comp(const char *a, const char *b) {
    for (; *a && *b; a++, b++) {
        if (*a != *b) return 0;
    }
    return (*a == '\0' && *b == '\0');
}

const char* file_type(uint32_t next_filehdr) {
// recursive solution to list files

	// os 3 bits menos significativos
	uint8_t type = next_filehdr & 0x7; // 0b111

	switch(type) {
		case 0: return "hard link";
		case 1: return "directory";
		case 2: return "regular file";
		case 3: return "symbolic link";
		case 4: return "block device";
		case 5: return "char device";
		case 6: return "socket";
		case 7: return "fifo";
		default: return "unknown";
	}
}

void ls_old(struct file_header *p) {
	if (read32((char*)&p->next_filehdr) == 0x00000000) return;	// if no more size, returns

	uint32_t next_offset = read32((char*)&p->next_filehdr);
	printf("%-20s %-12s\n", p->file_name, file_type(next_offset));							
	uint32_t fname_len = str_len(p->file_name) + 1;		// length + '\0'
	uint32_t header_size = 16 + round16(fname_len); 	// checks the file header size
	
	uint32_t data_size = round16(str_len((char*)p + header_size));	// here we get the length of the file data. We dont use 'str_len + 1 ('\0')' because the round16 function already 
																	// help us in that
																	// it is a little resource-consuming because we calculate the length in each interaction

	data_size = round16(read32((char*)&p->size));	// using the file's own defined size

	uint32_t next_file_offset = header_size + data_size;
	ls_old((struct file_header *)((char*)p + next_file_offset));
}


// ls with the offset caculation
void ls(struct file_header *p, char *base) {
	uint32_t next_offset = read32((char*)&p->next_filehdr);	// the next offset comes in big endian and has to be converted using our read function and then we use the wildcard
	if (next_offset == 0) return; 
	if(!str_comp(file_type(next_offset), "hard link")){		// index for files and directorys that excludes the hard links "." and ".."
		if(str_comp(file_type(next_offset), "regular file")){
			printf("	L ");
		}
		printf("%-50s %-12s\n", p->file_name, file_type(next_offset));
	}

	if(str_comp(file_type(next_offset), "directory")){		// if file is a directory, we should check it's data to view the internal file's informations
		if (!str_comp(p->file_name, ".") && !str_comp(p->file_name, "..")) {	// avoids repeating entrys
			uint32_t fname_len = str_len(p->file_name) + 1;		// length + '\0'
			uint32_t header_size = 16 + round16(fname_len);		// check where starts the new data = 16 (next_header + size + checksum + spec_info) + file_name. it uses 16 as the size of the fieds
			
			struct file_header * child = (struct file_header *)((char*)p + header_size);
			ls(child, base);	// checks for the new files using the first file in the directory -- check ./TP1/view.png for better understading
		}
	}
	
	next_offset &= 0xFFFFFFF0; // filter the file type from the offset (last 4 bits = 1111 wildcard = 0xF)
	if (next_offset == 0) return; 
	struct file_header * next = (struct file_header *)((char*)base + next_offset);
	ls(next, base);	// go to the next "main" course of reading


}



// for the find function we are going to use the same logic used in the 'ls' function
void find(struct file_header *p, char * base, char * file_name, struct file_header ** ret_addr){	// ret_addr is a has to return a pointer, so we use it as a pointer to a pointer
	if (*ret_addr != NULL) return;
	uint32_t next_offset = read32((char*)&p->next_filehdr);
	const char *type = file_type(next_offset);

	if(str_comp(type, "regular file")){
		if(str_comp(p->file_name, file_name)){
			printf("Found %s\n", file_name);
			*ret_addr = p; 	// "pointer to ->(*ret_addr)". If it was a common variable, we could use a single pointer
			return;
		}
	}

	if(str_comp(type, "directory")){		
		if (!str_comp(p->file_name, ".") && !str_comp(p->file_name, "..")) {
			uint32_t fname_len = str_len(p->file_name) + 1;		
			uint32_t header_size = 16 + round16(fname_len);	

			struct file_header * child = (struct file_header *)((char*)p + header_size);
			find(child, base, file_name, ret_addr);	
				if (*ret_addr != NULL) return;
		}
	}

	next_offset &= 0xFFFFFFF0; 
		if (next_offset == 0) return; 

	struct file_header * next = (struct file_header *)((char*)base + next_offset);
	find(next, base, file_name, ret_addr);	
		return;

}

// simply reads the file content using our offsets
void read_content(struct file_header *p){
	uint32_t fname_len = str_len(p->file_name) + 1;		
	uint32_t header_size = 16 + round16(fname_len);	
	printf("File content: \n%s\n", ((char*)p + header_size));
}

void decode(struct fs_header *p, size_t size){
	const char * magic = "-rom1fs-";
	for(int i = 0; i < 8; i++){
		assert(magic[i] == p->title[i]);
	}
	printf("The title is correct\n");

	uint32_t size_read = read32((char*)&p->size);
	uint32_t size_headers = (size - size_read)/8;
	uint32_t round_header_size = round16(size_headers);
	printf("The read size is %d\n", size_read);
	printf("Headers size up to %d hexatects\n", round_header_size);
	printf("Volume Name: %s\n", p->volume_name);
	int vol_len = str_len(p->volume_name) + 1;				// calculate the 'volume_name' size (adding 1 to the lenght because of '\0')
	size_t offset = 16 + round16(vol_len);  								// title + size + checksum (16) + offset = begin of the file header
	struct file_header *fh = (struct file_header *)((char*)p + offset); 	// here we use '(char*) p + offset' instead of 'p + offset' because there is an implict conmvertion
																			// between 'offset' and 'p'. if we do 'p + offset', C will do 'p + offset*(sizeof(struct p))' and it will generate errors
																			// we can also use uint8_t * and it will work the same way
	// now that we have the correct pointer to our struct, we can continue to pass through the files
	char * file_name = "message.txt";
		printf("Listing all the files...\n");
		ls(fh, (char *) p);
	struct file_header *result = NULL;
		find(fh, (char *) p, file_name, &result);	// in this function we use a pointer to a pointer because we want to use the pointer's address, not it's value
													// the & is used to say "I want to use the address of 'result'". So we use the pointer to the address of result to pass
													// it as an argument
			if(result != NULL) read_content(result);
			else printf("File %s not found", file_name);
}


int main(void)
{
	int fd = open("data/tp1fs.romfs", O_RDONLY);
	assert (fd != -1);
	off_t fsize;
	fsize = lseek(fd, 0, SEEK_END);
	printf("The real size is %zd\n", fsize);
	char *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
	assert (addr != MAP_FAILED);
	decode((struct fs_header *)addr, fsize);
	assert (munmap(addr, fsize) == 0);

	return 0;
}
