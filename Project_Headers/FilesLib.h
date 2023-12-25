/*
 * FilesLib.h
 *
 *  Created on: Jun 29, 2016
 *      Author: omerfeld
 */

#ifndef FILESLIB_H_
#define FILESLIB_H_
struct File{
	int size;
	char *text;
	};
struct FileLib{
	struct File Filenum[20];
	int numoffiles;
};


#endif /* FILESLIB_H_ */
