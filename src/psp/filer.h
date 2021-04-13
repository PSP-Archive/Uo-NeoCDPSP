#ifndef FILER_H
#define FILER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern char LastPath[], FilerMsg[];

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out);
int InitFiler(char*msg,char*path);
char *find_file(char *pattern,char *path);

// —LŒø‚ÈŠg’£Žq
enum {    
  EXT_ZIP,
  EXT_UNKNOWN  
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
