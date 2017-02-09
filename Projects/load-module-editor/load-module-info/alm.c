#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include "exec.h"
#include <endian.h>
/// Lucas Shadler
/// alm.c


/*
		Forward Declaration of helper functions
 */
int getSym(FILE *fp, char *str);
char *sections(size_t);


/**
 * main: takes the filenames from command line arguments
 *		 and, if they are an object or a load module, it
 *		 will parse the data given using MIPS R2000 
 *		 format. If the file is neither of these two,
 *		 an error will be printed, but it will move on
 *		 to the next cl argument
 * 
 * argc: number of command line arguments
 *
 * argv: string array containing arguments
 *
 */
int main(int argc, char **argv)
{
	if( argc > 1 )
	{
		
		for( int i = 1; i < argc ; ++i )
		{
			exec_t header;
			int numOffset = 0;
			size_t strIndex = 0;
			FILE *fp = fopen(argv[i],"rb");
		    if( fp == NULL )
		    {
		    	perror(argv[i]);
		    	continue;
		    }

			int n = fread(&header , sizeof(exec_t), 1 , fp);
			if( n > 0 )
			{
				//manipulate header
				header.magic   = be16toh(header.magic);
				header.entry   = be32toh(header.entry);
				header.version = be16toh(header.version);
				//check for magic word
				if( header.magic == HDR_MAGIC )
				{
					//test object/load file
					fprintf(stdout,"--------------------\n");
					if(header.entry)
					{
						fprintf(stdout,"File %s is an R2K load module (entry point "
								"%#010x)\n",argv[i], header.entry);
					}
					else
					{
						fprintf(stdout,"File %s is an R2K object module\n",argv[i]);
					}
				}
				else
				{
					fprintf(stderr, "error: File %s is not an R2K object file.\n",
							argv[i]);
					continue;
				}
				
				//Decode the Version

				// 1111 1110 0000 0000 === 0xfe00
				size_t year  = (size_t)((header.version & 0xfe00) >> 9);
				// 0000 0001 1110 0000 === 0x01e0
				size_t month = (size_t)((header.version & 0x01e0) >> 5);
				// 0000 0000 0001 1111 === 0x001f
				size_t day   = (size_t)(header.version & 0x001f);

				fprintf(stdout,"Module version: 2%03lu/%02lu/%02lu\n",
						year,month,day);

				//organize data fields
				for(int index = 0; index < N_EH; ++index)
				{
					header.data[index] = be32toh(header.data[index]);
				}
				
				//individually print each one
				//text
				if(header.sz_text)
				{
					fprintf(stdout,"Section TEXT is %lu bytes long\n",
							(size_t)header.sz_text);
					fseek(fp,(size_t)header.sz_text,SEEK_CUR);
				}
				//rdata
				if(header.sz_rdata)
				{
					fprintf(stdout,"Section RDATA is %lu bytes long\n",
							(size_t)header.sz_rdata);
					fseek(fp,(size_t)header.sz_rdata,SEEK_CUR);
				}
				//data
				if(header.sz_data)
				{
					fprintf(stdout,"Section DATA is %lu bytes long\n",
							(size_t)header.sz_data);
					fseek(fp,(size_t)header.sz_data,SEEK_CUR);
				}
				//sdata
				if(header.sz_sdata)
				{
					fprintf(stdout,"Section SDATA is %lu bytes long\n",
							(size_t)header.sz_sdata);
					fseek(fp,(size_t)header.sz_sdata,SEEK_CUR);
				}
				//sbss
				if(header.sz_sbss)
				{
					fprintf(stdout,"Section SBSS is %lu bytes long\n",
							(size_t)header.sz_sbss);
					fseek(fp,(size_t)header.sz_sbss,SEEK_CUR);
				}
				//bss
				if(header.sz_bss)
				{
					fprintf(stdout,"Section BSS is %lu bytes long\n",
							(size_t)header.sz_bss);
					fseek(fp,(size_t)header.sz_bss,SEEK_CUR);
				}
					numOffset = ftell(fp);
				
				//reloc (entries)
				if(header.n_reloc)
				{
					fprintf(stdout,"Section RELTAB is %lu entries long\n",
							(size_t)header.n_reloc);
					fseek(fp,(size_t)header.n_reloc*sizeof(relent_t),SEEK_CUR);
				}
				//refs (entries)
				if(header.n_refs)
				{
					fprintf(stdout,"Section REFTAB is %lu entries long\n",
							(size_t)header.n_refs);
					fseek(fp,(size_t)header.n_refs*sizeof(refent_t),SEEK_CUR);
				}
				//syms (entries)
				if(header.n_syms)
				{
					fprintf(stdout,"Section SYMTAB is %lu entries long\n",
							(size_t)header.n_syms);
					fseek(fp,(size_t)header.n_syms*sizeof(syment_t),SEEK_CUR);
				}
				//strings
				if(header.sz_strings)
				{
					strIndex = ftell(fp);
					fprintf(stdout,"Section STRINGS is %lu bytes long\n",
							(size_t)header.sz_strings);
				}
				
				fseek(fp,numOffset,SEEK_SET);
				

				// Relocation Table Printing
				if(header.n_reloc)
				{
					fprintf(stdout,"Relocation table:\n");
			
					for( size_t rel_i = 0; rel_i < header.n_reloc ; ++rel_i )
					{
						relent_t relTemp;
						int c = fread(&relTemp, sizeof(relent_t),1,fp);
						if(c>0)
						{
							relTemp.addr = be32toh(relTemp.addr);
							fprintf(stdout,"   %#010x (%s) type %#06x \n",
									relTemp.addr,sections(relTemp.section),
									relTemp.type);
						}
						else
						{
							fprintf(stderr,
								"error::%s cannot read the relocation table",
									argv[i]);
						}
					}
				}	

				// Reference Table Printing
				if(header.n_refs)
				{
					fprintf(stdout,"Reference table:\n");
					for( size_t ref_i = 0; ref_i < header.n_refs ; ++ref_i )
					{
						refent_t refTemp;
						int c = fread(&refTemp, sizeof(refent_t),1,fp);
						size_t currPos = ftell(fp);
						if(c>0)
						{
							refTemp.addr = be32toh(refTemp.addr);
							refTemp.sym  = be32toh(refTemp.sym);
							//---------------------------------------
							fseek(fp,strIndex+(size_t)refTemp.sym,SEEK_SET);
							char symbol[100];
							getSym(fp,symbol);
							fseek(fp,currPos,SEEK_SET);
							//---------------------------------------
							fprintf(stdout,"   %#010x type %#06x symbol %s\n",
									refTemp.addr,refTemp.type,symbol);
						}
						else
						{
							fprintf(stderr,
								"error::%s cannot read the reference table",
									argv[i]);
						}
					}
				}
				
				// Symbol Table Printing
				if(header.n_syms)
				{
					fprintf(stdout,"Symbol table:\n");
				
					for( size_t sym_i = 0; sym_i < header.n_syms ; ++sym_i )
					{
						syment_t symTemp;
						int c = fread(&symTemp, sizeof(syment_t),1,fp);
						size_t currPos = ftell(fp);
						if(c>0)
						{
							symTemp.flags = be32toh(symTemp.flags);
							symTemp.value = be32toh(symTemp.value);
							symTemp.sym   = be32toh(symTemp.sym);
							//------------------------------------	
							fseek(fp,strIndex+(size_t)symTemp.sym,SEEK_SET);
							char symbol[100];
							getSym(fp,symbol);
							fseek(fp,currPos,SEEK_SET);
							//------------------------------------
							fprintf(stdout,
								"   value %#010x flags %#010x symbol %s\n",
									symTemp.value,symTemp.flags,symbol);
						}
						else
						{
							fprintf(stderr,
								"error::%s cannot read the symbol table",
								argv[i]);
						}
					}
				}
				fclose(fp);
			}
			else
			{
				fprintf(stderr,"error: cannot read %s",argv[i]);
			}
		}
	}
	else
	{
	 fprintf(stderr,"usage: alm file1 [ file2 ... ]\n");
	 return 1; // abnormal exit
	}

	return 0;  //normal exit
}


/**
 * sections: takes in a data index and returns the string
 *			 that corresponds with this section of data
 *
 * sect: 	 index of the section in data
 *
 * return:	 string containing the section name
 */
char *sections(size_t sect)
{
	switch((int)sect)
	{
		case 1:
			return "TEXT";
			break;
		case 2:
			return "RDATA";
			break;
		case 3:
			return "DATA";
			break;
		case 4:
			return "SDATA";
			break;
		case 5:
			return "SBSS";
			break;
		case 6:
			return "BSS";
			break;
		case 7:
			return "REL";
			break;
		case 8:
			return "REF";
			break;
		case 9:
			return "SYM";
			break;
		case 10:
			return "STR";
			break;
		default:
			return NULL;
			break;
	}
	return NULL;
}

/**
 * getSym: reads and returns the NUL-terminated
 *		   string at the current location of the
 *		   FILE
 *
 * fp:	   FILE pointer for I/O
 *
 * str:	   The string that the NUL-terminated
 * 		   string is stored in
 *
 */
int getSym(FILE *fp, char *str)
{
	int ch, i = 0;
	while ((ch = fgetc(fp)) != '\0' && ch != EOF) 
	{
	     str[i++] = ch;
	}
	str[i] = '\0';
	return 1;
}

