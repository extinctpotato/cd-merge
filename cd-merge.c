#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <taglib/tag_c.h>

#define BUFFER 128

void gethelp(char *argument)
{
	printf("Usage: %s -1 [CD1] -2 [CD2] [OPTIONS]\n", argument);
	printf("Change the TRACK tag in CD2 so that it becomes contiguous with CD1\n");
	printf("THIS WILL NOT WORK ON UNTAGGED FILES!\n\n");
	printf("Optional arguments:\n");
	printf("   -m\tMove files from CD1 to CD2\n");
	printf("   -r\tRename the files in format 'TRACK TRACKNAME'\n");
}

int dircount(DIR *dir)
{
	int counter=0;
	struct dirent *dir_entry;
	while((dir_entry=readdir(dir)) != NULL)
	{
		if(dir_entry -> d_type == DT_REG)
		{
			counter++;
		}
	}
	
	return counter;
}

void trackmodify(TagLib_File *file, int offset)
{
	TagLib_Tag *tag;
	tag = taglib_file_tag(file);
	int newtrack = taglib_tag_track(tag) + offset;
	taglib_tag_set_track(tag, newtrack);
	taglib_file_save(file);
	printf("New track number: %d for ", newtrack);
}

void tracktagrename(char *filename)
{
	TagLib_File *file;
	TagLib_Tag *tag;
	char formatted[BUFFER];
	
	file = taglib_file_new(filename);
	if(file != NULL)
	{
		tag = taglib_file_tag(file);
	
		if(taglib_tag_track(tag) < 10)
		{
			sprintf(formatted, "0%d", taglib_tag_track(tag));
		}
		else
		{
			sprintf(formatted, "%d", taglib_tag_track(tag));
		}

		sprintf(formatted + strlen(formatted), " %s", taglib_tag_title(tag));
		sprintf(formatted + strlen(formatted), "%s", strrchr(filename, '.'));
	
		printf("Renaming %s to %s\n", filename, formatted);
		
		rename(filename, formatted);
	}
}

void dirtagrename(DIR *dir, char *dirname)
{
	rewinddir(dir);
	chdir(dirname);
	struct dirent *dir_entry;
	while((dir_entry=readdir(dir)) != NULL)
	{
		if(dir_entry -> d_type == DT_REG)
		{
			tracktagrename(dir_entry -> d_name);
		}
	}
}

void directorymodify(DIR *dir, int offset, char *dirname)
{
	rewinddir(dir);
	struct dirent *dir_entry;
	TagLib_File *file;
	chdir(dirname);
	while((dir_entry=readdir(dir)) != NULL)
	{
		if(dir_entry -> d_type == DT_REG)
		{
			file = taglib_file_new(dir_entry -> d_name);
			if(file != NULL)
			{
				trackmodify(file, offset);
				printf("%s\n", dir_entry -> d_name);
			}
			else
			{
				printf("File %s is invalid, skipping...\n", dir_entry -> d_name);
			}
		}
	}
}

void move(DIR *source, char *sourcename, char *destinationname)
{
	rewinddir(source);
	struct dirent *dir_entry;
	chdir(sourcename);
	char fullpath[BUFFER];
	while((dir_entry=readdir(source)) != NULL)
	{
		if(dir_entry -> d_type == DT_REG)
		{
			sprintf(fullpath, "%s/%s", destinationname, dir_entry -> d_name);
			printf("Moving %s to %s\n", dir_entry -> d_name, fullpath);
			rename(dir_entry -> d_name, fullpath);
		}
	}
}



int main(int argc, char *argv[])
{
	int option_index=0;
	int CD1TRACKS=0;
	int CD2TRACKS=0;
	DIR *CD1, *CD2;
	char CD1_PATH[BUFFER], CD2_PATH[BUFFER];
	
	while((option_index=getopt(argc, argv, "1:2:mrh")) != -1)
	{
		switch(option_index)
		{
			case '1':
				if((CD1 = opendir(optarg)) == NULL)
				{
					printf("%s: error opening %s\n", argv[0], optarg);
					return 1;
				}
				realpath(optarg, CD1_PATH);
				printf("CD1 directory is: %s\n", CD1_PATH);
				break;
			case '2':
				if((CD2 = opendir(optarg)) == NULL)
				{
					printf("%s: error opening %s\n", argv[0], optarg);
					return 1;
				}
				realpath(optarg, CD2_PATH);
				printf("CD2 directory is %s\n", CD2_PATH);
				if((CD1TRACKS = dircount(CD1)) == 0)
				{
					printf("Directory CD1 is empty!\n");
					return 1;
				}
				if((CD2TRACKS = dircount(CD2)) == 0)
				{
					printf("Directory CD2 is empty!\n");
					return 1;
				}
				printf("There are %d files in the CD1\n", CD1TRACKS);
				directorymodify(CD2, CD1TRACKS, optarg);
				break;
			case 'm':
				if(CD1TRACKS != 0 && CD2TRACKS !=0)
				{
					move(CD2, CD2_PATH, CD1_PATH);
				}
				else
				{
					printf("%s: nothing to move\n", argv[0]);
					return 1;
				}
				break;
			case 'h':
				gethelp(argv[0]);
				break;
			case 'r':
				if(CD1TRACKS)
				{
					dirtagrename(CD1, CD1_PATH);
				}
				else
				{
					printf("%s: nothing to rename or did not move the files\n", argv[0]);
					return 1;
				}
				break;
			case '?':
				if(isprint(optopt))
				{
					gethelp(argv[0]);
				}
		}
	}
	
	return 0;
}
