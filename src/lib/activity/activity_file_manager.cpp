/**
 * @file activity_file_manager.cpp
 * Function set to help manage activity files.
 * @author Martins Frolovs <martins.f@airdog.vom>
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <systemlib/systemlib.h>
#include <systemlib/param/param.h>
#include <systemlib/err.h>
#include <sys/stat.h>

#include <uORB/uORB.h>
#include <uORB/topics/activity_params.h>

#include "activity_lib_constants.h"
#include "activity_file_manager.hpp"

namespace Activity
{
namespace Files
{

bool
create_directory_structure() {

    // TODO: Add error handling
    
    DIR * pDir = opendir(ACTIVITY_FILE_DIR);

    if (pDir == NULL) {

        printf("Directory \"%s\" does not exist.\n", ACTIVITY_FILE_DIR);

        int mkdir_ret = mkdir(ACTIVITY_FILE_DIR, 0770);

        if (mkdir_ret == -1) { 
            printf("Failed to create directory: %s\n", ACTIVITY_FILE_DIR);
            return false;
        } else {
            printf("Successfuly created directory: %s\n", ACTIVITY_FILE_DIR);
        }

    } else {
        closedir(pDir);
    }

    for (int i=0;i<ACTIVITIES_COUNT;i++){

        char dir_path[PATH_MAX];
        sprintf(dir_path, "%s/%i", ACTIVITY_FILE_DIR, i);

        printf("Activity folder %s\n", dir_path);

        DIR * pDir = opendir(dir_path);
        if (pDir == NULL) {
     
            printf("Directory \"%s\" does not exist.\n", dir_path);

            int mkdir_ret = mkdir(dir_path, 0770);

            if (mkdir_ret == -1) { 
                printf("Failed to create directory: %s\n", dir_path);
                return false;
            } else {
                printf("Successfuly created directory: %s\n", dir_path);
            }

        } else {
            closedir(pDir);
        }

        char file_path[PATH_MAX];
        int attribute = 0;
        sprintf(file_path,"%s/%d",dir_path,attribute);

        FILE * pFile = fopen(file_path, "r");

       if (pFile == NULL) {
           printf("File %s does not exist let's create it.\n", file_path);
           pFile = fopen(file_path, "w");
           fclose(pFile);
       } else {
            printf("File %s exists.\n", file_path);
            fclose(pFile);
       }
    }
}

__EXPORT bool
get_path(uint8_t activity, uint8_t attribute, char (&pathname)[PATH_MAX]) {
    
    switch(attribute) {
        case PARAMS:
            sprintf(pathname,"%s/%d/%d", ACTIVITY_FILE_DIR, activity, attribute);
            break;
        default:
            return false;
    }

   FILE * pFile = fopen(pathname, "r");
   if (pFile == NULL) {
       printf("Cannot open file %s. \nLet's check and create directory structure if needed.\n", pathname);
       create_directory_structure();
   }

    return true;
}


__EXPORT bool
has_valid_content(uint8_t activity, uint8_t attribute, const char pathname[]){

    if (attribute == PARAMS) 
        return validate_activity_params_file(activity, pathname);
    else
        return false;

}

bool
validate_activity_params_file(uint8_t activity, const char pathname[]) {

    char line[MAX_STR_LEN];
    char key_str[MAX_STR_LEN];
    char value_str[MAX_STR_LEN];

    int line_no = 0;

    float param_id;
    float param_val;

    bool param_appeared[ALLOWED_PARAM_COUNT];
    bool param_required[ALLOWED_PARAM_COUNT];

    memset(param_appeared, 0, sizeof(param_appeared));
    memset(param_required, 1, sizeof(param_required));

    FILE * activity_params_file = fopen(pathname, "r");

    if (activity_params_file == NULL) {

        printf("Failed to open file %s !\n", pathname);
        return false;
    }

    while (fgets(line, MAX_STR_LEN, activity_params_file) != NULL) {

        printf("line: %s\n", line);

        line_no++;

        // 1. Check every line is in the proper format int_number:float_number
        //
        // Validate if each line is on format:
        // " [0]number[1]:[2]number[3].[4]number[5]" 
        // number given AFTER every part
        // line can end after part 3 or part 5 (floating point is optional)
        
        int part = 0;

        for (int i=0;line[i]!='\n';i++) {

            switch (part) {
                case 0:
                    if (isdigit(line[i]))
                        part++;
                    else 
                    {
                        printf("%s Error on line %i: Line must start with digit\n", pathname, line_no);
                        return false;
                    }
                break;
                case 1:
                    if (line[i]==':')
                        part++;
                    else if (!isdigit(line[i]))
                    {
                        printf("%s Error on line %i: Activity number must be followed by colon\n", pathname, line_no);
                        return false;
                    }
                break;
                case 2:
                    if (isdigit(line[i]))
                        part++;
                    else
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        return false;
                    }
                break;
                case 3:
                    if (line[i] == '.') 
                        part++;
                    else if (!isdigit(line[i]))
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        return false;
                    }
                break;
                case 4:
                    if (isdigit(line[i])) {
                        part++;
                    }
                    else
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        return false;
                    }
                case 5:
                    if (!isdigit(line[i])){
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        return false;
                    }
            }
        }

        if (!(part == 3 || part == 5 )){
            printf("%s Error on line %i: Line must end with a number\n", pathname, line_no);
            return false;
        }

        // 2. Check: every required param is showing up

        parse_activity_file_line(line, key_str, value_str);

        sscanf(key_str,"%f", &param_id);
        sscanf(value_str,"%f", &param_val);

        printf("%s:%f | %s:%f\n", key_str, (double)param_id, value_str, (double)param_val);

        if (param_id >= ALLOWED_PARAM_COUNT) {
            printf("%s Error: Param number %f not allowed. \n", pathname, (double)param_id);
            return false;
        }
    
        if (param_appeared[(int)param_id]){
            printf("%s Error: Param number %f defined multiple times. \n", pathname, (double)param_id);
            return false;
        }

        if (line_no == 1) {
            if (param_id != 0) {
                printf("%s Error: First line of param file must define param 0 (activity_id) \n", pathname );
                return false;
            }

            if (param_val != activity) {
                printf("%s Error: File not valid for activity %f : parameter activity_id value in file is %f \n", pathname, (double)activity , (double)param_val);
                return false;
            }
        }

        param_appeared[(int)param_id] = true;

        // TODO: 3. Check: values are in proper range 
    }

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        if (param_appeared[i] && !param_required[i]){
            printf("%s Error: Param number %d must not appear in the file.\n", pathname, i);
            return false;
        }

        if (!param_appeared[i] && param_required[i]){
            printf("%s Error: Param number %d must appear in the file.\n", pathname, i);
            return false;
        }
    }

    return true;
}

bool isdigit(char c){
    return c>='0' && c<='9';
}


__EXPORT bool
update_activity(uint8_t activity, uint8_t attribute) {

    set_file_state();

}

__EXPORT bool
get_file_state() {

    char pathname[PATH_MAX];
    sprintf(pathname,"%s/ok", ACTIVITY_FILE_DIR);
    FILE * f = fopen(pathname, "r"); 

    bool file_state_ok = (f != NULL);

    fclose(f);

    return file_state_ok;
}


__EXPORT bool
set_file_state() {

    char ok_pathname[PATH_MAX];

    sprintf(ok_pathname, "%s/ok", ACTIVITY_FILE_DIR);

    unlink(ok_pathname);

    bool ok = true;

    for (int i=0;i<ACTIVITIES_COUNT && ok;i++) {
    
        char pathname[PATH_MAX];
        get_path(i,0,pathname);

        if (!has_valid_content(i,0,pathname))
            ok = false;
        else 
            printf("File %s ok!\n", pathname);
       
        break; //TODO: remove this break
        
    }

    if (ok) {
        FILE * f = fopen(ok_pathname, "w");
        fclose(f);
    }
     
    return ok;
}

__EXPORT bool
activity_orb_to_file(uint8_t activity){

    char tmp_file_path[PATH_MAX];
    get_path(activity, 0, tmp_file_path);

    printf("Returned path %s \n", tmp_file_path);

    FILE * tmp_file = fopen(tmp_file_path, "w");

	int activity_params_sub = orb_subscribe(ORB_ID(activity_params));

    activity_params_s activity_params;
	orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

    int value_activity = activity_params.values[0];

    if (value_activity != activity) {
        printf("Trying to store parameters of activity %d to activity file %d\n", value_activity, activity);
        return false; 
    }

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        fprintf(tmp_file, "%i:%.6f\n", i, (double)activity_params.values[i]); 
    }

    char new_file_path[PATH_MAX];
    get_path(activity,0, new_file_path);

    if (rename(tmp_file_path, new_file_path)) {
        printf("Successfully written %d params file.\n", activity);
    } else {
        printf("Failed to write file: %s\n", new_file_path);
    }

	orb_unsubscribe(activity_params_sub);
} 

__EXPORT bool
activity_file_to_orb(uint8_t activity) {

    if (activity>ACTIVITIES_COUNT){
        printf("ERROR: Activity file manager: activity number %d does not exist\n", activity);
        return false;
    }

    char activity_param_file_path[PATH_MAX];
    get_path(activity, 0, activity_param_file_path);

    FILE * activity_file = fopen(activity_param_file_path, "r");

    if ( activity_file == NULL ){ 
        printf("ERROR: Activity file manager: Failed to open file: %s\n", activity_param_file_path);
        return false;
    }

    char line[MAX_STR_LEN];
    char key_str[MAX_STR_LEN];
    char value_str[MAX_STR_LEN];

    int line_no = 0;

    activity_params_s activity_params;

    while (fgets(line, MAX_STR_LEN, activity_file) != NULL) {

        line_no++;

        parse_activity_file_line(line, key_str, value_str);

        float param_id;
        float param_val;

        sscanf(key_str,"%f", &param_id);
        sscanf(value_str,"%f", &param_val);

        activity_params.values[(int)param_id] = param_val;
    }

    int activity_params_pub = orb_advertise(ORB_ID(activity_params), &activity_params);

    if (activity_params_pub < 0) {
        printf("Failed to publish activity params\n");
    }

    return true;
}


bool
parse_activity_file_line(char line[],char (&id)[MAX_STR_LEN], char (&value)[MAX_STR_LEN]){

    // Trim line from whitespaces
    if (strlen(line) > 0 && line[strlen(line)-1] < 33) {
        line[strlen(line)] = '\0';
    }

    memset(id, 0, sizeof(char) * MAX_STR_LEN);
    memset(value, 0, sizeof(char) * MAX_STR_LEN);

    char * c = strstr(line, ":");

    if (c == NULL) {
        printf("ERROR: Activity file manager: line must contain colon.\n");
        return false;
    }

    int pos = c - line;

    strncpy(id, line, pos);
    strncpy(value, line + pos + 1, strlen(line));

    return true;
}

}
// end of namespace Files
}
// end of namespace Activity
