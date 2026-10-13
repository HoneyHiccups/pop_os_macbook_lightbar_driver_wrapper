#include <linux/input.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include <filesystem>
#include <iostream>
#include <string>

using namespace std;
namespace fs = std::filesystem;
void ajust_brightnes(int amount);
void fetch_event(string& refstring);
void write_brightness(int value);
void read_brightness();


int cur_brightness = 0; // I understand I could prob do this without a state manger here however I have this so I can clamp ez;
enum brightness_state{
    up,
    down,
    nothing
};
//brightness_state scan_events(string& event_path);
int wait_for_brightness_press(int device_fd,brightness_state& stateref);

int main (){
brightness_state State;
    //printf ("hello world\n");

   
        string event_device;
        fetch_event(event_device);
        if (event_device == "null") {
            printf("No iBridge device found.\n");
            return 1;
        }
        int event_stream = ::open(event_device.c_str(), O_RDONLY | O_CLOEXEC);

        while(true == true){

            //here we will scan for event and set state
            //read vals
            read_brightness();
            State = brightness_state::nothing;
            int key_code = wait_for_brightness_press(event_stream,State);


            if(State == brightness_state::up){
                ajust_brightnes(25);
                State = brightness_state::nothing;
            }
            if(State==brightness_state::down){
                ajust_brightnes(-25);
                State = brightness_state::nothing;
            }

            
        } 
        
    return 0;

}


void fetch_event(string& refstring){
    for (const auto& entry : fs::directory_iterator("/dev/input/by-id")) {
        string filename = entry.path().filename().string();
            if (filename.find("iBridge") != string::npos) {
                printf("Found iBridge device: %s\n", filename.c_str());
                refstring = entry.path().string();
                return;
            }
    }
    refstring = "null";
}
void ajust_brightnes(int amount){
    if(amount+cur_brightness>=255){
        cur_brightness = 255;
        write_brightness(255);
        return;
    }
    if(amount+cur_brightness<=0){
        cur_brightness =0;
        write_brightness(0);
        return;
    }
    cur_brightness = cur_brightness+amount;
    write_brightness(cur_brightness);
}
void write_brightness(int value) { // I think I will make this a void
    std::ofstream brightness_config("/sys/class/leds/spi::kbd_backlight/brightness");
    if (!brightness_config){
        return;
    }
    brightness_config << value << "\n";
    brightness_config.close();
    return;
}
void read_brightness(){
    std::ifstream brightness_config("/sys/class/leds/spi::kbd_backlight/brightness");
    if (!brightness_config){
        printf("could not open brightness configfile yikes");
        return;  
    } 
    int temp;
    brightness_config >>temp;
    if (temp >= 0 && temp<=255){
        cur_brightness = temp;
    }
    brightness_config.close();
}

/*
brightness_state scan_events(string& event_path){

    struct input_event ev;
    while (read(event_path, &ev, sizeof(ev)) == sizeof(ev)) {
    // handle ev
    }

    return brightness_state::nothing;
}
*/

int wait_for_brightness_press(int device_fd,brightness_state& stateref) {
    struct input_event ev;
    for (;;) {
        ssize_t n = ::read(device_fd, &ev, sizeof ev);
        if (n != (ssize_t)sizeof ev) return -1;

        if (ev.type != EV_KEY) continue;
        if (ev.value != 0) continue;            // grabbing realease
        if (ev.code != KEY_KBDILLUMUP && ev.code != KEY_KBDILLUMDOWN) continue;
        if (ev.code == KEY_KBDILLUMUP)  {
            stateref = brightness_state::up;
        }
        if (ev.code == KEY_KBDILLUMDOWN){
            stateref = brightness_state::down;
        }

        return ev.code;
    }
}