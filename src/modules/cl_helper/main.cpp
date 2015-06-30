#include <nuttx/config.h>


#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdlib.h>

#include <uORB/uORB.h>
#include <uORB/topics/bt_state.h>

#include <cstdio>
#include <cstring>
#include <ctime>

#include <activity/activity_change_manager.hpp>
#include <activity/activity_file_manager.hpp>
#include <activity/activity_lib_constants.h>

extern "C" __EXPORT int
main(int argc, const char * const * argv);

using namespace std;

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

static void
usage()
{
    fprintf(stderr, "Usage:\n"
        "   clh pairing\t[on|off|toggle]\n"
        "       activity\t[test|receive_init|receive_modify|\n"
        "   \t\t print_orb|file_to_orb|orb_to_file|\n"
        "   \t\t get_path|set_file_state|fill_files]"
        "\n"
    );
}

#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)
#define PAIRING_TOGGLE      _IOC(_BLUETOOTH21_BASE, 2)

static int _bt_sub = -1;
static struct bt_state_s _bt_state;

void bt_state() {
	_bt_sub = orb_subscribe(ORB_ID(bt_state));
    if (_bt_sub > 0) {
        orb_copy(ORB_ID(bt_state), _bt_sub, &_bt_state);
        switch (_bt_state.global_state) {
            case INITIALIZING:
                printf("Current bluetooth state: NOT STARTED\n");
                break;
            case PAIRING:
                printf("Current bluetooth state: PAIRING\n");
                break;
            case NO_PAIRED_DEVICES:
                printf("Current bluetooth state: NO_PAIRED_DEVICES\n");
                break;
            case CONNECTING:
                printf("Current bluetooth state: CONNECTING\n");
                break;
            case CONNECTED:
                printf("Current bluetooth state: CONNECTED\n");
                break;
            default:
                printf("Unknown bluetooth state\n");
                break;
        }
    }
}

void pairing_on() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_ON, 0);
    }

    close(fd);


}

void pairing_off() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_OFF, 0);
    }

    close(fd);
}


void pairing_toggle() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_TOGGLE, 0);
    }

    close(fd);

}

bool test_activity_manager(const int activity_number = 5);
bool init_receive_fake_activity_params();
bool modify_receive_fake_activity_params();
bool print_activity_params_orb_content();
bool fill_activity_files();

int
main(int argc, char const * const * argv)
{
	if (argc < 2) {
        usage();
		return 1;
	}

    if (streq(argv[1], "help")) {
        usage();
    }

    else if (streq(argv[1], "status")) {
        bt_state();
    }

    else if (streq(argv[1], "pairing")) {

        if (argc < 3) usage();
        else if (streq(argv[2], "on")) {
            pairing_on();
        } else if (streq(argv[2], "off")) {
            pairing_off();
        } else if (streq(argv[3], "toggle")) {
            pairing_toggle();
        } else {
            usage();
        }
    
    } else if (streq(argv[1], "activity")) {

        if (streq(argv[2], "test")) {
            if (argc == 4)
            {
                const int activ_number = atoi(argv[3]);
                test_activity_manager(activ_number);
            }
            test_activity_manager();
        } else if (streq(argv[2], "receive_init")) {
            init_receive_fake_activity_params();
        } else if (streq(argv[2], "receive_modify")) {
            modify_receive_fake_activity_params();
        } else if (streq(argv[2], "print_orb")) {
            print_activity_params_orb_content();
        } else if (streq(argv[2], "file_to_orb")) {
            Activity::Files::activity_file_to_orb(1); 
        } else if (streq(argv[2], "orb_to_file")) {
            Activity::Files::activity_orb_to_file(1);
        } else if (streq(argv[2], "get_path")) {
            char path[PATH_MAX];
            Activity::Files::get_path(1,0,path);
            printf("%s\n", path);
        } else if (streq(argv[2], "set_file_state")) {
            Activity::Files::set_file_state();
        } else if (streq(argv[2], "fill_files")){
            fill_activity_files(); 
        }
        else {
            usage();
        }
    
    } else {
        usage();
    }

	return 0;
}

bool
test_activity_manager(const int activity_number){


    auto A = Activity::getActivityChangeManager(activity_number, true);
    init_receive_fake_activity_params();
    if (A.params_updated()) { 

        printf("So far so good .\n");

        char val[32];
        char display_name[32];

        auto *param = A.get_current_param();

        for (int i=0;i<20;i++) {

            param->get_param_name(display_name, 32);
            param->get_display_value(val, 32);
            printf("%s : %s\n", display_name, val);

            for (int j=0;j<2;j++) {
                param->get_next_value(val, 32);
                printf("%s : %s\n", display_name, val);
            }

            param->save_value();

            int last = param->get_id();
            param = A.get_next_visible_param();

            if (last > param->get_id())
                break;
        }

        A.save_params();

        modify_receive_fake_activity_params();

        while (!A.params_updated()) {}

        if (A.params_updated()){
            printf("Params received ! \n ");
        }

        printf("Prev PART.\n"); 

        param = A.get_current_param();

        memset(val, 0, sizeof(val));

        for (int i=0;i<9;i++) {

            param->get_param_name(display_name, 32);
            param->get_display_value(val, 32);
            printf("%s : %s\n", display_name, val);

            for (int j=0;j<10;j++) {
                param->get_prev_value(val, 32);
                printf("%s : %s\n", display_name, val);
            }

            param->save_value();
            param = A.get_prev_visible_param();

        }

        A.save_params();

        modify_receive_fake_activity_params();

        while (!A.params_updated()) {}

        if (A.params_updated()){
            printf("Params received ! \n ");
        }

        printf("Cancel param PART.\n"); 

        param = A.get_current_param();
        memset(val, 0, sizeof(val));

        for (int i=0;i<9;i++) {

            param->get_param_name(display_name, 32);
            param->get_display_value(val, 32);
            printf("%s : %s\n", display_name, val);

            for (int j=0;j<10;j++) {
                param->get_prev_value(val, 32);
                printf("%s : %s\n", display_name, val);
            }

            param->cancel_value();
            param = A.get_prev_visible_param();

        }

        A.save_params();

        modify_receive_fake_activity_params();

        while (!A.params_updated()) {}

        if (A.params_updated()){
            printf("Params received ! \n ");
        }

    } 
}

bool
modify_receive_fake_activity_params() {

    activity_params_s activity_params;

    int activity_params_sub = orb_subscribe(ORB_ID(activity_params));
	orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

    activity_params.type = ACTIVITY_PARAMS_RECEIVED; 

    for (int i=0;i<Activity::ALLOWED_PARAM_COUNT;i++)
        activity_params.values[i]+=1.0f;

    int pub = orb_advertise(ORB_ID(activity_params), &activity_params);

    printf("Moddified %d\n", pub);
}

bool
init_receive_fake_activity_params() {

    activity_params_s activity_params;
    activity_params.type = ACTIVITY_PARAMS_RECEIVED; 

    for (int i=0;i<Activity::ALLOWED_PARAM_COUNT;i++)
        activity_params.values[i]=1.0f;

    int pub = orb_advertise(ORB_ID(activity_params), &activity_params);
    printf("Published %d\n", pub);

}

bool
print_activity_params_orb_content() {

	int activity_params_sub = orb_subscribe(ORB_ID(activity_params));

    activity_params_s activity_params;
	orb_copy(ORB_ID(activity_params), activity_params_sub, &activity_params);

    for (int i=0;i<Activity::ALLOWED_PARAM_COUNT;i++){
        printf("%f: %.5f\n", (double)i, (double)activity_params.values[i]);
    }

    return true;
}


bool 
fill_activity_files(){

    for (int i=0;i<Activity::ACTIVITIES_COUNT;i++) {
        char pathname[PATH_MAX];
        Activity::Files::get_path(i,0,pathname);

        printf("%s\n", pathname);
        FILE * f = fopen(pathname, "w");

        for (int j=0;j<Activity::ALLOWED_PARAM_COUNT;j++) {
            fprintf(f, "%i:%f\n", (int)j, (double)i);
            printf("%i:%f\n", (int)j, (double)i);
        }
        fclose(f);
    }
}
