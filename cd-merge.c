#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <taglib/tag_c.h>

void gethelp(char *argument)
{
	printf("Usage %s -1 [CD1] -2 [CD2]\n", argument);
	printf("Pass -m if you want to move the files after merging\n");
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
	char fullpath[128];
	while((dir_entry=readdir(source)) != NULL)
	{
		if(dir_entry -> d_type == DT_REG)
		{
			strcpy(fullpath, destinationname);
			strcat(fullpath, "/");
			strcat(fullpath, dir_entry -> d_name);
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
	char CD1_PATH[128], CD2_PATH[128];
	
	while((option_index=getopt(argc, argv, "1:2:mh")) != -1)
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
		}
	}
	
	return 0;
}
