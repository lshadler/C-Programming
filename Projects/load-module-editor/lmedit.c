#define _BSD_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "exec.h"
#include <endian.h>
#include <stdbool.h>
/**
 *	Lucas Shadler
 *	lmedit.c
 */


/**
 *		Forward Declaration of helper functions
 */
int      getSym(FILE *fp, char *str);
char*    sections(size_t);
int      sectInd(char*);
size_t   getAddrOff(int,size_t*,size_t);
int		 printTable(int,int, void**,int,size_t,size_t*,size_t*,FILE*);
int      examineTD(int,int,int,char,size_t,size_t*,void**);
int      modTD(int,int,int,char,size_t, size_t*,void**,int);
void	 addHist(char*,char**);

/// main reads in the file and allows or interactive editing
/// @param argc size of arguments
/// @param argv element string array
///
int main(int argc, char **argv)
{
	if( argc > 1 )
	{		
		exec_t header;
		size_t locs[10];
		void *loadMod[10];
		size_t sizes[10];
		FILE *fp = fopen(argv[1],"r+b");
		relent_t *relTab;
		refent_t *refTab;
		syment_t *symTab;
	    if( fp == NULL )
	    {
	    	perror(argv[1]);
	    	return 1;
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
				if(header.entry)
				{
					fprintf(stdout,
						"File %s is an R2K load module (entry point %#010x)\n",
						argv[1], header.entry);
				}
				else
				{
					fprintf(stdout,"File %s is an R2K object module\n",argv[1]);
				}
			}
			else
			{
				fprintf(stderr, "error: File %s is not an R2K object file.\n",
						argv[1]);
				return 1;
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
			
			// TEXT
			char text[header.sz_text];
			sizes[EH_IX_TEXT] = header.sz_text;
			locs[EH_IX_TEXT]  = ftell(fp);
			if(header.sz_text)
			{
				fprintf(stdout,"Section text is %lu bytes long\n",
						(size_t)header.sz_text);
				int c = fread(text, sizeof(char), header.sz_text,fp);
				if(c > 0)
				{
					loadMod[EH_IX_TEXT] = text;
				}
			}

			// RDATA
			char rdata[header.sz_rdata];
			sizes[EH_IX_RDATA] = header.sz_rdata;
			locs[EH_IX_RDATA]  = ftell(fp);
			if(header.sz_rdata)
			{
				fprintf(stdout,"Section rdata is %lu bytes long\n",
						(size_t)header.sz_rdata);
				int c = fread(rdata, sizeof(char), header.sz_rdata,fp);
				if(c > 0)
				{
					loadMod[EH_IX_RDATA] = rdata;
				}
			}
		
			// DATA
			char data[header.sz_data];
			sizes[EH_IX_DATA] = header.sz_data;
			locs[EH_IX_DATA]  = ftell(fp);
			if(header.sz_data)
			{
				fprintf(stdout,"Section data is %lu bytes long\n",
						(size_t)header.sz_data);
				int c = fread(data, sizeof(char), header.sz_data,fp);
				if(c > 0)
				{
					loadMod[EH_IX_DATA] = data;
				}
			}

			// SDATA
			char sdata[header.sz_sdata];
			sizes[EH_IX_SDATA] = header.sz_sdata;
			locs[EH_IX_SDATA]  = ftell(fp);
			if(header.sz_sdata)
			{
				fprintf(stdout,"Section sdata is %lu bytes long\n",
						(size_t)header.sz_sdata);
				int c = fread(sdata, sizeof(char), header.sz_sdata,fp);
				if(c > 0)
				{
					loadMod[EH_IX_SDATA] = sdata;
				}
			}


			// SBSS
			char sbss[header.sz_sbss];
			sizes[EH_IX_SBSS] = header.sz_sbss;
			locs[EH_IX_SBSS]  = ftell(fp);
			if(header.sz_sbss)
			{
				fprintf(stdout,"Section sbss is %lu bytes long\n",
						(size_t)header.sz_sbss);
				int c = fread(sbss, sizeof(char), header.sz_sbss,fp);
				if(c > 0)
				{
					loadMod[EH_IX_SBSS] = sbss;
				}
			}

			// BSS
			char bss[header.sz_bss];
			sizes[EH_IX_BSS] = header.sz_bss;
			locs[EH_IX_BSS]  = ftell(fp);
			if(header.sz_bss)
			{
				fprintf(stdout,"Section bss is %lu bytes long\n",
						(size_t)header.sz_bss);
				int c = fread(bss, sizeof(char), header.sz_bss,fp);
				if(c > 0)
				{
					loadMod[EH_IX_BSS] = bss;
				}
			}

			//reloc (entries)
			relTab = malloc(sizeof(relent_t) * header.n_reloc);
			sizes[EH_IX_REL] = header.n_reloc;
			locs[EH_IX_REL]  = ftell(fp);
			if(header.n_reloc)
			{
			    fprintf(stdout,"Section reltab is %lu entries long\n",
						(size_t)header.n_reloc);
				for( size_t rel = 0; rel < header.n_reloc; ++rel )
				{
					relent_t tempRel;
					int c = fread(&tempRel,sizeof(relent_t),1,fp);
					if(c > 0)
					{
						relTab[rel] = tempRel;
					}
				}
			}
			loadMod[EH_IX_REL] = relTab;
			
			
			//refs (entries)
			refTab = malloc(sizeof(refent_t) * header.n_refs);
			sizes[EH_IX_REF] = header.n_refs;
			locs[EH_IX_REF]  = ftell(fp);
			if(header.n_refs)
			{
			    fprintf(stdout,"Section reftab is %lu entries long\n",
						(size_t)header.n_refs);
				for( size_t ref = 0; ref < header.n_refs; ++ref )
				{
					refent_t tempRef;
					int c = fread(&tempRef,sizeof(refent_t),1,fp);
					if(c > 0)
					{
						refTab[ref] = tempRef;
					}
				}
			}
			loadMod[EH_IX_REF] = refTab;
	

			//syms (entries)
			symTab = malloc(header.n_syms * sizeof(syment_t));
			sizes[EH_IX_SYM] = header.n_syms;
			locs[EH_IX_SYM]  = ftell(fp);
			if(header.n_syms)
			{
			    fprintf(stdout,"Section symtab is %lu entries long\n",
						(size_t)header.n_syms);
				for( size_t sym = 0; sym < header.n_syms; ++sym )
				{
					syment_t tempSym;
					int c = fread(&tempSym,sizeof(syment_t),1,fp);
					if(c > 0)
					{
						symTab[sym] = tempSym;
					}
				}
			}
			loadMod[EH_IX_SYM] = symTab;

			//strings
			locs[EH_IX_STR] = ftell(fp);
			sizes[EH_IX_STR] = header.sz_strings;
			char strings[header.sz_strings];
			if(header.sz_strings)
			{
				
				int c = fread(strings, sizeof(char) , header.sz_strings, fp );
				if( c > 0 )
				{
				 	loadMod[EH_IX_STR] = strings;
				}
				
				fprintf(stdout,"Section strings is %lu bytes long\n",
						(size_t)header.sz_strings);
			}

			
            fprintf(stdout,"Available Commands: section, size, write, quit\n");
            fprintf(stdout,"                    address,count:type=value\n");
		
			/*
			 * Take in input from the user
			 */
			int currentSection = 0,cmdCount = 1;
		    char buf[128];	
			char **hist = calloc(10,sizeof(char*));
			char sect[32];
			bool isChange = false;
			while(1)
			{
			 	fprintf(stdout,"%s[%d] >",sections(currentSection),cmdCount);
				if(fgets(buf,128,stdin)==NULL)
			 	{
			 		//EOF
			 		break;
			 	}
				if( sscanf(buf,"section %s",sect) == 1 )
				{
				    addHist(buf,hist);	
					//this was a section command
					if(sectInd(sect) == -1)
						fprintf(stderr,"error:  '%s' is not a valid section name\n",sect);
					else if(sectInd(sect) == EH_IX_BSS || sectInd(sect) == EH_IX_SBSS)
						fprintf(stderr,"error:  cannot edit %s section\n",sect);
					else
					{
								currentSection = sectInd(sect);
								fprintf(stdout,"Now editing section %s\n",sections(currentSection));
					}
				}
				else if( sscanf(buf,"size %s",sect) != 0 )
				{
					addHist(buf,hist);
					//this was a size command
					if(currentSection < EH_IX_REL || currentSection == EH_IX_STR)
					{
						fprintf(stdout,"Section %s is %lu bytes long\n",
							sections(currentSection),sizes[currentSection]);
					}
					else
					{
						fprintf(stdout,"Section %s is %lu entries long\n",
							sections(currentSection),sizes[currentSection]);
					}
				}
				else if( sscanf(buf,"quit %s",sect) != 0 )
				{
					addHist(buf,hist);
					//this was a quit command
					if(isChange)
					{
						//process input
						bool done = false;
						while(1)
						{
							fprintf(stdout,"Discard modifications (yes or no)?");
							char buff[128];
							char extra[32];
							if(fgets(buff,128,stdin) == NULL)
							{
								//EOF
							}
							if(sscanf(buff,"yes %s",extra) != 0)
							{
								done = true;
						    	break;
							}
							else if( sscanf(buff,"no %s",extra) != 0 )
							{
								break;
							}
							else
							{
								fprintf(stderr,"This is invalid input. Try again...\n");
							}

							
							
						}
						if(done) break;
					}
					else
						break;
					
				}
				else if( sscanf(buf,"write %s",sect) != 0 )
				{
					addHist(buf,hist);
					//this was a write command
					for(int index = 0; index < EH_IX_SBSS; ++index)
					{
						fseek(fp, locs[index],SEEK_SET);
						fwrite(loadMod[index],sizeof(char),header.data[index],fp);
					}
					
					int str = EH_IX_STR;
					fseek(fp, locs[str],SEEK_SET);
					fwrite(loadMod[str],sizeof(char),header.data[str],fp);
					
					isChange = false;
				}
				else if( sscanf(buf,"history %s",sect) != 0 )
				{
					addHist(buf,hist);
					//this is a history command
					for(int i = 9;i >= 0; --i)
					{
						if(hist[i] != NULL)
						{
							fprintf(stdout,"%s",hist[i]);
						}
					}
				}
				else
				{
					//examination/modification of the file	
					if(currentSection < EH_IX_SBSS || currentSection == EH_IX_STR )
					{
						int addr,count,val;
						char type;
                    	size_t offset = getAddrOff(currentSection,sizes,header.entry);
						if( sscanf(buf,"%i,%i:%c=%i", &addr, &count, &type, &val) == 4 ) 
						{
							addHist(buf,hist);
                        	isChange = true;
							modTD(addr,currentSection,count,type,offset,sizes,loadMod,val);
    	            	} 
    	            	else if( sscanf(buf,"%i,%i:%c", &addr, &count, &type) == 3 ) 
    	            	{
							addHist(buf,hist);
				 			examineTD(addr,currentSection,count,type,offset,sizes,loadMod);
        	            } 
            	    	else if( sscanf(buf,"%i:%c=%i", &addr, &type, &val) == 3 ) 
            	    	{     	        
							addHist(buf,hist);
                        	isChange = true;
							modTD(addr,currentSection,1,type,offset,sizes,loadMod,val);
                    	}
            	    	else if( sscanf(buf,"%i,%i=%i", &addr, &count, &val) == 3 ) 
            	    	{     	        
							addHist(buf,hist);
                        	isChange = true;
							modTD(addr,currentSection,count,'w',offset,sizes,loadMod,val);
                    	} 
                    	else if( sscanf(buf,"%i,%i", &addr, &count) == 2 ) 
                    	{	
							addHist(buf,hist);
				 			examineTD(addr,currentSection,count,'w',offset,sizes,loadMod);
                     	}
                    	else if( sscanf(buf,"%i=%i", &addr, &val) == 2 ) 
                    	{
							addHist(buf,hist);
                        	isChange = true;
							modTD(addr,currentSection,1,'w',offset,sizes,loadMod,val);
                     	}
                       	else if( sscanf(buf,"%i:%c", &addr, &type) == 2 ) 
                    	{
							addHist(buf,hist);
				 			examineTD(addr,currentSection,1,type,offset,sizes,loadMod);	
                     	}
                    	else if( sscanf(buf,"%i", &addr) == 1 ) 
                    	{
							addHist(buf,hist);
				 			examineTD(addr,currentSection,1,'w',offset,sizes,loadMod);	
                     	}
					}	
					else
					{	
						int addr,count,val;
						char type;
                    	size_t offset = 0;
						if( sscanf(buf,"%i,%i:%c=%i", &addr, &count, &type, &val) == 4 ) 
						{
							addHist(buf,hist);
                    		fprintf(stderr,"error: '=0x%x' is not valid in table sections\n",val);
                    		fprintf(stderr,"error: ':%c' is not valid in table sections\n",type);
    	            	} 
    	            	else if( sscanf(buf,"%i,%i:%c", &addr, &count, &type) == 3 ) 
    	            	{
							addHist(buf,hist);
                    		fprintf(stderr,"error: ':%c' is not valid in table sections\n",type);
        	            } 
            	    	else if( sscanf(buf,"%i:%c=%i", &addr, &type, &val) == 3 ) 
            	    	{
							addHist(buf,hist);
                    		fprintf(stderr,"error: '=0x%x' is not valid in table sections\n",val);
                    		fprintf(stderr,"error: ':%c' is not valid in table sections\n",type);
                    	} 
                    	else if( sscanf(buf,"%i,%i", &addr, &count) == 2 ) 
                    	{	
							addHist(buf,hist);
                    		printTable(addr,currentSection,loadMod,count,offset,sizes,locs,fp);
                     	}
                    	else if( sscanf(buf,"%i=%i", &addr, &val) == 2 ) 
                    	{
							addHist(buf,hist);
                    		fprintf(stderr,"error: '=0x%x' is not valid in table sections\n",val);
                     	}
                       	else if( sscanf(buf,"%i:%c", &addr, &type) == 2 ) 
                    	{
							addHist(buf,hist);
                    		fprintf(stderr,"error: ':%c' is not valid in table sections\n",type);
                     	}
                    	else if( sscanf(buf,"%i", &addr) == 1 ) 
                    	{
							addHist(buf,hist);
							printTable(addr,currentSection,loadMod,1,offset,sizes,locs,fp);
                     	}
					}

				}
			
				cmdCount++;
			}
		
		fclose(fp);
	    free(relTab);
	    free(refTab);
	    free(symTab);
		free(hist);	
		}
		else
		{
			fprintf(stderr,"error: cannot read %s",argv[1]);
			return 1;
		}
	}
	else
	{
	 fprintf(stderr,"usage: lmedit file\n");
	 return 1; // abnormal exit
	}
	


	return 0;  //normal exit
}



/// sections takes in a data index and returns the string that corresponds with this section of data
/// @param sect index of the section in data
/// @param return string containing the section name
///
char *sections(size_t sect)
{
	char *sectName;
	switch((int)sect)
	{
		case 0:
			sectName = "text";
			break;
		case 1:
			sectName = "rdata";
			break;
		case 2:
			sectName = "data";
			break;
		case 3:
			sectName = "sdata";
			break;
		case 4:
			sectName = "sbss";
			break;
		case 5:
			sectName = "bss";
			break;
		case 6:
			sectName = "reltab";
			break;
		case 7:
			sectName = "reftab";
			break;
		case 8:
			sectName = "symtab";
			break;
		case 9:
			sectName = "strings";
			break;
		default:
			return NULL;
			break;
	}
	return sectName;
}


/// getSym reads and returns the NUL-terminated string at the current location of the FILE
/// @param fp FILE pointer for I/O
/// @param str The string that the NUL-terminated string is stored in
///
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



///	sectInd returns index of the section given its pre-defined string name
///	@param name	

int sectInd(char *name)
{
	if(strcmp(name,"text")==0)
		return EH_IX_TEXT;
	else if(strcmp(name,"rdata")==0)
		return EH_IX_RDATA;
	else if(strcmp(name,"data")==0)
		return EH_IX_DATA;
	else if(strcmp(name,"sdata")==0)
		return EH_IX_SDATA;
	else if(strcmp(name,"sbss")==0)
		return EH_IX_SBSS;
	else if(strcmp(name,"bss")==0)
		return EH_IX_BSS;
	else if(strcmp(name,"reltab")==0)
		return EH_IX_REL;
	else if(strcmp(name,"reftab")==0)
		return EH_IX_REF;
	else if(strcmp(name,"symtab")==0)
		return EH_IX_SYM;
	else if(strcmp(name,"strings")==0)
		return EH_IX_STR;
	else
		return -1;
}

size_t getAddrOff(int currentSection, size_t *sizes,size_t type)
{
	size_t offset;
	if(type)
	{
		switch(currentSection)
		{
			case 0:
				//text
				offset = TEXT_BEGIN;
				break;
			case 1:
				//rdata
				offset = DATA_BEGIN;
				break;
			case 2:
				//data
				offset = DATA_BEGIN+sizes[1]+sizes[1]%8;
				break;
			case 3:
				offset = DATA_BEGIN+sizes[1] + sizes[1]%8 + sizes[2] + sizes[2]%8;
				//sdata
				break;

			case 9:
				//string
				offset = 0;
				break;
		}
	}
	else
	{
		offset = 0;
	}
	return offset;
}

/// printTable prints an amount of table entries
///	@param addr the address
/// @param currentSection the index of the current section
/// @param count the number of elements being modified
/// @param offset the section offset from start index
/// @param sizes the array containing section sizes
/// @param loadMod the file stored into an array
/// @param locs the array containing the file locations of each section
/// @param fp the file
/// @pre loadMod[currentSection] is defined
/// @post nothing has been changed
///
int printTable(int addr,int currentSection, void **loadMod, int count,size_t offset,size_t *sizes,size_t *locs,FILE *fp)
{
	
    int    min    = offset;
    size_t max    = offset + sizes[currentSection];
	
	if(addr < min || addr >= (int)max)
	{
   		fprintf(stderr,"error:  '%i' is not a valid address.\n",addr);
   		return -1;
	}
	if(count <= 0 || count > (int)(max-addr))
	{
   		fprintf(stderr,"error:  '%i' is not a valid count.\n",count);
   		return -1;
	}
	switch(currentSection)
	{
		case EH_IX_REL:
		{
    		relent_t *dataRel = (relent_t *)loadMod[currentSection];
			for(int j = 0; j<count; ++j)
			{
				relent_t tempRel = dataRel[addr+j];
				tempRel.addr = be32toh(tempRel.addr);
				fprintf(stdout,"   0x%08x (%s) type 0x%04x \n",tempRel.addr,
				sections(tempRel.section-1),tempRel.type);
			}
			break;
		}
		case EH_IX_REF:
		{
			refent_t *dataRef = (refent_t *)loadMod[currentSection];
			for(int j = 0; j < count; ++j)
			{
				refent_t tempRef = dataRef[addr+j];
				tempRef.addr = be32toh(tempRef.addr);
				tempRef.sym  = be32toh(tempRef.sym);

				//------------------------------------------
				fseek(fp,locs[EH_IX_STR]+(size_t)tempRef.sym,SEEK_SET);
				char symbol[100];
				getSym(fp,symbol);
				//------------------------------------------
				fprintf(stdout,"   0x%08x type 0x%04x symbol %s\n",
						tempRef.addr,tempRef.type,symbol);
			}
			break;
		}
		case EH_IX_SYM:
		{
			syment_t *dataSym = (syment_t *)loadMod[currentSection];
			for(int j = 0; j < count; ++j)
			{
				syment_t tempSym = dataSym[addr+j];
				tempSym.value = be32toh(tempSym.value);
				tempSym.sym  = be32toh(tempSym.sym);
				tempSym.flags = be32toh(tempSym.flags);
				//------------------------------------------
				fseek(fp,locs[EH_IX_STR]+(size_t)tempSym.sym,SEEK_SET);
				char symbol[100];
				getSym(fp,symbol);
				//------------------------------------------
				fprintf(stdout,"   value 0x%08x flags 0x%08x symbol %s\n",
						tempSym.value,tempSym.flags,symbol);
			}
			break;
		}
	}
	return 0;
}


/// examineTD prints the given element(s)
///	@param addr the address
/// @param currentSection the index of the current section
/// @param count the number of elements being modified
/// @param type describes the number of bytes per element
/// @param offset the section offset from start index
/// @param sizes the array containing section sizes
/// @param loadMod the file stored into an array
/// @pre loadMod[currentSection] is defined
/// @post nothing has been changed
///
int examineTD(int addr, int currentSection,int count,char type, size_t offset, size_t *sizes, void **loadMod)
{	
    int    min    = offset;
    size_t max    = offset + sizes[currentSection];
	switch(type)
	{
		case 'w':
		{
			int start = (addr-offset)/4;
        	uint32_t *data = (uint32_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint32_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
				    uint32_t prInt = data[start+i];
				    prInt = be32toh(prInt);
        			fprintf(stdout,"   0x%08x = 0x%08x\n",
        					(uint32_t)(addr+i*sizeof(uint32_t)),
        					prInt);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 

        	break;
		}
		case 'h':
		{
			int start = (addr-offset)/2;
        	uint16_t *data = (uint16_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint16_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
					uint16_t prInt = data[start+i];
					prInt = be16toh(prInt);
        			fprintf(stdout,"   0x%08x = 0x%04x\n",
        					(uint32_t)(addr+i*sizeof(uint16_t)),
        					prInt);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 
        	break;
		}
		case 'b':
		{
        	uint8_t *data = (uint8_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint8_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
        			fprintf(stdout,"   0x%08x = 0x%02x\n",
        					(uint32_t)(addr+i*sizeof(uint8_t)),
        					data[addr-offset+i]);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 
        	break;
		}
	}
	return 0;
}

/// modTD modifies a Text section or Data Section
///	@param addr the address
/// @param currentSection the index of the current section
/// @param count the number of elements being modified
/// @param type describes the number of bytes per element
/// @param offset the section offset from start index
/// @param sizes the array containing section sizes
/// @param loadMod the file stored into an array
/// @param val the value to set the element(s) to
/// @pre loadMod[currentSection] is defined
/// @post the chosen section was modified
///
int modTD(int addr, int currentSection,int count, char type, size_t offset, size_t *sizes, void **loadMod, int val)
{	
    int    min    = offset;
    size_t max    = offset + sizes[currentSection];
	switch(type)
	{
		case 'w':
		{
			int start = (addr-offset)/4;
        	uint32_t *data = (uint32_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint32_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
					val = be32toh(val);
					data[start+i] = val;
					val = be32toh(val);
        			fprintf(stdout,"   0x%08x is now 0x%08x\n",
        					(uint32_t)(addr+i*sizeof(uint32_t)),
        					val);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 
        	loadMod[currentSection] = data;
        	break;
		}
		case 'h':
		{
			int start = (addr-offset)/2;
        	uint16_t *data = (uint16_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint16_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
					val = be16toh(val);
					data[start+i] = val;
					val = be16toh(val);
        			fprintf(stdout,"   0x%08x is now 0x%04x\n",
        					(uint32_t)(addr+i*sizeof(uint16_t)),
        					data[addr-offset+i]);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 
        	loadMod[currentSection] = data;
        	break;
		}
		case 'b':
		{
        	uint8_t *data = (uint8_t *)loadMod[currentSection];
			if(addr >= min && addr+count*sizeof(uint8_t) <= max)
			{
				for(int i = 0;i<count;++i)
				{
					data[addr-offset+i] = val;
        			fprintf(stdout,"0x%08x is now 0x%02x\n",
        					(uint32_t)(addr+i*sizeof(uint8_t)),
        					data[addr-offset+i]);
        		}
        	}
        	else if(count == 1)
        	{
        		fprintf(stderr,"error:  '%i' is not a valid address\n",addr);
        	}
        	else
        	{
        		fprintf(stderr,"error:  '%i' is not a valid count\n",count);
        	} 
        	loadMod[currentSection] = data;
        	break;
		}
	}
	return 0;
}

/// addHist adds an element to the history table
/// @param str string to add
/// @param hist the history table
///
void addHist(char *str, char **hist)
{
	for(int j = 9; j > 0; --j)
	{
		if(hist[j-1])
		{
			hist[j]= hist[j-1]; 
		}
	}
	hist[0] = str;
}






