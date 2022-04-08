// Library which allows to look for file types like JPEG, PNG, GIF and BMP.
// FF D8 FF DB, FF D8 FF E0, FF D8 FF EE, FF D8 FF E1 // JPG
// 89 50 4E 47 //PNG
// 47 49 46 38 //GIF
// 42 4D //BMP

#define _GNU_SOURCE

// C program to read particular bytes from the existing file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "plugin_api.h"

 
static char *g_lib_name = "libivsN3251.so";

static char *g_plugin_purpose = "Look for file types like JPEG, PNG, GIF and BMP";

static char *g_plugin_author = "Stupnitskiy Ivan Vitalievich, N3251";
 

#define OPT_FILE_PNG "png"
#define OPT_FILE_GIF "gif"
#define OPT_FILE_BMP "bmp"


static struct plugin_option g_po_arr[] = {
/*
    struct plugin_option {
        struct option {
           const char *name;
           int         has_arg;
           int        *flag;
           int         val;
        } opt,
        char *opt_descr
    }
*/

    {{"pic", 1, 0, 0},
     "Check for png,bmp,gif or jpg files."}
};

static int g_po_arr_len = sizeof(g_po_arr)/sizeof(g_po_arr[0]); // Some crazy arguments

//
//  Private functions
//
static int file_check(const char*, const char*); // my variant function

//
//  API functions
//
//Function structurizing the plugin information
int plugin_get_info(struct plugin_info* ppi) { 
    if (!ppi) {
        fprintf(stderr, "ERROR: invalid argument\n");
        return -1;
    }
    
    ppi->plugin_purpose = g_plugin_purpose;
    ppi->plugin_author = g_plugin_author;
    ppi->sup_opts_len = g_po_arr_len;
    ppi->sup_opts = g_po_arr;
    
    return 0;
}

//Function that handles plugin options and arguments

int plugin_process_file(const char *fname, struct option in_opts[], size_t in_opts_len) {
    
    int res = 1;

    if (!fname || !in_opts || in_opts_len <= 0){
        errno = EINVAL;
        return -1;
    }
    
    int* res_opts = (int*)malloc(sizeof(int) *in_opts_len);
    
    if (!res_opts){
        perror("malloc");
        return -1;
    }
    
    for (size_t i = 0; i < in_opts_len; i++)
        res_opts[i] = 0;
    
    for (size_t i = 0; i < in_opts_len; i++){
    
        if (strcmp(in_opts[i].name, "pic") == 0){
            char *tmp = (char *)in_opts[i].flag;
            int p_a_count = 1; //How many parameters (arguments) user send in option
            while (( tmp = strstr(tmp, ",")) != NULL){
                p_a_count++;
                tmp = tmp+1;
            }
            
            int arg_len = strlen((char *)in_opts[i].flag);
            tmp = (char *)malloc(sizeof(char)*(arg_len+1));
            char *tmp_saved = tmp;
            if (!tmp){
                perror("malloc");
                if (res_opts) free(res_opts);
                return -1;
            }
            
            for (int j = 0; j <= arg_len; j++)
                tmp[j] = ((char *) in_opts[i].flag)[j];
            
            //Let's part arguments by comma
            char *p_type = NULL;
            char *checked_param;
            char *rest = tmp;            
            checked_param = (char*)calloc(3, sizeof(char));
            
            //jpg, png, gif, bmp
            while ((p_type = strtok_r(rest, ",", &rest))){
                if (strlen(p_type) == 3) {
                    if (strcmp(p_type, "jpg") == 0) { checked_param = "jpg"; /* printf("\nAAAAA ETO ZHE ARGUMENT JPG)))) %s\n", checked_param); */}
                    else if (strcmp(p_type, "png") == 0) checked_param = "png";
                    else if (strcmp(p_type, "gif") == 0) checked_param = "gif";
                    else if (strcmp(p_type, "bmp") == 0) checked_param = "bmp";
                    else {
                        fprintf(stderr, "%s: argument by index %d is incorrect, (%s)\n", g_lib_name, p_a_count-1, p_type); 
                        if (res_opts) free(res_opts);
                        if (tmp_saved) free(tmp_saved);
                        if (checked_param) free(checked_param);
                        if (p_type) free(p_type);
                        if (tmp) free(tmp);
                        return -1;
                    }
                    FILE *fp = fopen(fname,"rb");
                if (fp == NULL) {
                    fprintf(stderr, "%s: Error occured while opening file '%s'\n", g_lib_name, fname);
                    if (res_opts) free(res_opts);
                    if (tmp_saved) free(tmp_saved);
                    if (p_type) free(p_type);
                    if (checked_param) free(checked_param);
                    if (tmp) free(tmp);
                    return -1;
                }
                
                //printf("\n\nChecked param is %s and fname is %s\n\n", checked_param, fname);
                //printf("Res is first time%d\n",res);
                res = res && file_check(checked_param, fname);
                
                //printf("Res is %d\n",res);
                }
                else {
                    fprintf(stderr, "%s: argument by index %d is too long, (%s)\n", g_lib_name, p_a_count-1, p_type);
                    if (res_opts) free(res_opts);
                    if (tmp_saved) free(tmp_saved);
                    if (p_type) free(p_type);
                    if (checked_param) free(checked_param);
                    if (tmp) free(tmp);
                    return -1;
                    }
                }
        }
    
    }
    
     if (res_opts) free(res_opts);
     return res;
}

// The function of this plugin, which checks file information (Picture or not)
int file_check(const char *param, const char *path)
{
    unsigned int jpg[3] = {0xFF, 0xD8, 0xFF};
    unsigned int png[4] = {0x89, 0x50, 0x4E, 0x47};
    unsigned int gif[4] = {0x47, 0x49, 0x46, 0x38};
    unsigned int bmp[2] = {0x42, 0x4D};
    // Pointer to the file to be read from
    FILE* fptr1;
    int c = 0;
    int counter = 0;
    // Stores the bytes to read
    unsigned int str[4];
    int i = 0, j, to = 3;
  
    // If the file exists and has read permission
    fptr1 = fopen(path, "r");
    // Just in case, anyway the main file has additional check
    if (fptr1 == NULL) {
        printf("No permission\n");
        return 1;
    }
    //printf("\n\n");
    // Loop to read required byte of file
    for (i = 0, j = 0; i <= to && c != EOF;
         i++) 
    {
        c = fgetc(fptr1); 
        //printf("%2x ", c); //+
        str[j] = c;
        j++;
    }
    //printf("\n\n The bytes has been readen %d\n\n", str[0]); //+
    //printf("\n");
    
// The first bytes of pictures are:
// FF D8 FF DB, FF D8 FF E0, FF D8 FF EE, FF D8 FF E1 // JPG //ATTENTION! In first bytes of jpg files, the 4th byte is for metadata
// 89 50 4E 47 //PNG
// 47 49 46 38 //GIF
// 42 4D //BMP
    
    //Check given parameters  and after it check file itself
    if (!strcmp(param, "jpg"))
    {
        //printf("\n\nThe jpg parameter is on!\n\n"); //+
        for (int i = 0; i <= 2; i++)
        {
            if (str[i] == jpg[i]){
                //printf("+"); //+
                counter++;
            }
        }
        //printf("\n");
        if (counter == 3)
            return 0;
        else
            return 1;
    }
    
    else if (!strcmp(param, "png"))
    {
        for (int i = 0; i <= 3; i++)
        {
            if (str[i] == png[i])
                counter++;
        }
        if (counter == 4)
            return 0;
        else
            return 1;
    }
    
    else if (!strcmp(param, "gif"))
    {
        for (int i = 0; i <= 3; i++)
        {
            if (str[i] == gif[i])
                counter++;
        }
        if (counter == 4)
            return 0;
        else
            return 1;
    }
    
    else if (!strcmp(param, "bmp"))
    {
        for (int i = 0; i <= 1; i++)
        {
            if (str[i] == bmp[i])
                counter++;
        }
        if (counter == 2)
            return 0;
        else
            return 1;
    }    
    else
        return 0;
    //Testing correct save
    /*for (i=0; i<4; i++)
        printf("%d ", str[i]); */
    //printf("\n");
    
    // Close the file
    //fclose(fptr1);
}
