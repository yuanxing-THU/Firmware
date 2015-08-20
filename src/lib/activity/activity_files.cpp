/**
 * Function set to help manage activity files.
 * @author Martins Frolovs <martins.f@airdog.vom>
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

#include <systemlib/systemlib.h>
#include <systemlib/param/param.h>
#include <systemlib/err.h>
#include <sys/stat.h>

#include <drivers/drv_hrt.h>
#include <errno.h>

#include <uORB/uORB.h>
#include <uORB/topics/activity_params.h>

#include "activity_lib_constants.h"
#include "activity_files.hpp"


namespace Activity
{
namespace Files
{

bool
create_directory_structure() {

    if (chdir(ACTIVITY_FILE_DIR) != 0) {

        printf("Directory \"%s\" does not exist.\n", ACTIVITY_FILE_DIR);

        int mkdir_ret = mkdir(ACTIVITY_FILE_DIR, 0770);

        printf("Activity dir %s\n", ACTIVITY_FILE_DIR);

        if (mkdir_ret == -1) { 
            printf("Failed to create directory: %s\n", ACTIVITY_FILE_DIR);
            printf ("mkdir error : %s\n",strerror(errno));
            return false;
        } else {
            printf("Successfuly created directory: %s\n", ACTIVITY_FILE_DIR);
        }

    } else {
        printf("Directory %s exists.\n", ACTIVITY_FILE_DIR);
    }


    for (int i=0;i<ACTIVITIES_COUNT;i++){

        char dir_path[PATH_MAX];
        sprintf(dir_path, "%s/%i", ACTIVITY_FILE_DIR, i);

        if (chdir(dir_path) != 0) {
     
            printf("Directory \"%s\" does not exist.\n", dir_path);

            int mkdir_ret = mkdir(dir_path, 0770);

            if (mkdir_ret == -1) { 
                printf("Failed to create directory: %s\n", dir_path);
                return false;
            } else {
                printf("Successfuly created directory: %s\n", dir_path);
            }
        } else {
            printf("Directory %s exists.\n", dir_path);
        }
    }
}

__EXPORT bool
get_path(uint8_t activity, uint8_t attribute, char (&pathname)[PATH_MAX]) {
    
    char dir[PATH_MAX];
    switch(attribute) {
        case PARAMS:
            sprintf(dir,"%s/%d", ACTIVITY_FILE_DIR, activity);
            sprintf(pathname,"%s/%d/%d", ACTIVITY_FILE_DIR, activity, attribute);
            break;
        default:
            return false;
    }

   if (chdir(dir) != 0) {
       printf("Directory %s doesn't exist. \nLet's check and create directory structure if needed.\n", dir);
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

    bool file_ok = true;

    if (activity_params_file == NULL) {

        printf("Failed to open file %s !\n", pathname);
        return false;

    }

    while (file_ok && fgets(line, MAX_STR_LEN, activity_params_file) != NULL) {

        printf("line: %s\n", line);

        line_no++;

        // 1. Check every line is in the proper format int_number:float_number
        //
        // Validate if each line is on format:
        // " [0]number[1]:[2]number[3].[4]number[5]" 
        // number given AFTER every part
        // line can end after part 3 or part 5 (floating point is optional)
        
        int part = 0;

        for (int i=0;line[i]!='\n' && file_ok;i++) {

            switch (part) {
                case 0:
                    if (isdigit(line[i]))
                        part++;
                    else 
                    {
                        printf("%s Error on line %i: Line must start with digit\n", pathname, line_no);
                        file_ok = false;
                    }
                break;
                case 1:
                    if (line[i]==':')
                        part++;
                    else if (!isdigit(line[i]))
                    {
                        printf("%s Error on line %i: Activity number must be followed by colon\n", pathname, line_no);
                        file_ok = false;
                    }
                break;
                case 2:
                    if (isdigit(line[i]))
                        part++;
                    else
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        file_ok = false;
                    }
                break;
                case 3:
                    if (line[i] == '.') 
                        part++;
                    else if (!isdigit(line[i]))
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        file_ok = false;
                    }
                break;
                case 4:
                    if (isdigit(line[i])) {
                        part++;
                    }
                    else
                    {
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        file_ok = false;
                    }
                case 5:
                    if (!isdigit(line[i])){
                        printf("%s Error on line %i: Colon must be followed by [floating point] number\n", pathname, line_no);
                        file_ok = false;
                    }
            }
        }

        if (!file_ok)
            break;

        if (!(part == 3 || part == 5 )){
            printf("%s Error on line %i: Line must end with a number\n", pathname, line_no);
            file_ok = false;
            break;
        }


        // 2. Check: every required param is showing up

        parse_activity_file_line(line, key_str, value_str);

        sscanf(key_str,"%f", &param_id);
        sscanf(value_str,"%f", &param_val);

        printf("%s:%f | %s:%f\n", key_str, (double)param_id, value_str, (double)param_val);

        if (param_id >= ALLOWED_PARAM_COUNT) {
            printf("%s Error: Param number %f not allowed. \n", pathname, (double)param_id);
            file_ok = false;
            break;
        }
    
        if (param_appeared[(int)param_id]){
            printf("%s Error: Param number %f defined multiple times. \n", pathname, (double)param_id);
            file_ok = false;
            break;
        }

        if (line_no == 1) {
            if (param_id != 0) {
                printf("%s Error: First line of param file must define param 0 (activity_id) \n", pathname );
                file_ok = false;
                break;
            }

            if (param_val != activity) {
                printf("%s Error: File not valid for activity %f : parameter activity_id value in file is %f \n", pathname, (double)activity , (double)param_val);
                file_ok = false;
                break;
            }
        }

        param_appeared[(int)param_id] = true;

        // TODO: 3. Check: values are in proper range 
    }

    fclose(activity_params_file);

    if (!file_ok) return false;

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

    clear_file_state();

}

__EXPORT bool
get_file_state() {

    char ok_pathname[PATH_MAX];
    sprintf(ok_pathname,"%s/ok", ACTIVITY_FILE_DIR);
    bool file_state_ok = (chdir(ok_pathname) == 0);
    return file_state_ok;

}


__EXPORT bool
clear_file_state() {

    char ok_pathname[PATH_MAX];
    sprintf(ok_pathname,"%s/ok", ACTIVITY_FILE_DIR);
    if (chdir(ok_pathname) == 0){
        chdir(ACTIVITY_FILE_DIR);
        rmdir(ok_pathname);
    }


    return true;
}


__EXPORT bool
check_file_state() {

    char ok_pathname[PATH_MAX];
    sprintf(ok_pathname,"%s/ok", ACTIVITY_FILE_DIR);

    clear_file_state();

    bool ok = true;

    for (int i=0;i<ACTIVITIES_COUNT && ok;i++) {

        char pathname[PATH_MAX];
        get_path(i,0,pathname);

        if (!has_valid_content(i,0,pathname))
            ok = false;
        else 
            printf("File %s ok!\n", pathname);
        
    }

    if (ok) {

        int mkdir_ret = mkdir(ok_pathname, 0770);

        if (mkdir_ret == -1) { 
            printf("Failed to create directory: %s\n", ok_pathname);
            return false;
        } else {
            printf("Successfuly created directory: %s\n", ok_pathname);
        }

    }
     
    return ok;
}

__EXPORT bool
activity_orb_to_file(){

	int activity_params_sub = orb_subscribe(ORB_ID(activity_params));
    activity_params_s activity_params;
	orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

	orb_unsubscribe(activity_params_sub);

    char tmp_file_path[PATH_MAX];
    sprintf(tmp_file_path,"%s/tmp", ACTIVITY_FILE_DIR);
    FILE * tmp_file = fopen(tmp_file_path, "w");

    int activity = activity_params.values[0];

    for (int i=0;i<ALLOWED_PARAM_COUNT;i++) {
        fprintf(tmp_file, "%i:%.6f\n", i, (double)activity_params.values[i]); 
    }

    fclose(tmp_file);

    char new_file_path[PATH_MAX];
    get_path(activity, 0, new_file_path);

    unlink(new_file_path);

    if (rename(tmp_file_path, new_file_path) == 0) {
        printf("Successfully written %d params file.\n", activity);
    } else {
        printf("Failed to write file: %s\n", new_file_path);
    }

} 

__EXPORT bool
activity_file_to_orb(uint8_t activity) {

    printf("activity_file_to_orb\n");

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

    activity_params.ts = hrt_absolute_time();

    fclose(activity_file);

    int activity_params_pub = orb_advertise(ORB_ID(activity_params), &activity_params);

    printf("activity_params_pub: %i\n", activity_params_pub);

    if (activity_params_pub < 0) {
        printf("Failed to publish activity params\n");
    } else {
        printf("Pub successful\n");
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
