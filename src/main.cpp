/*
  RG350 Test
  Test the status of console RG-350:
  · Buttons
  · Sticks
  · Sound
  · Rumble

  Created by Rafa Vico
  November 2019
*/

///////////////////////////////////
/*  Libraries                    */
///////////////////////////////////
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <math.h>
#include <pthread.h>
#include <shake.h>  // rumble lib
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

///////////////////////////////////
/*  Joystick codes               */
///////////////////////////////////

#define GCW_BUTTON_UP           SDLK_UP
#define GCW_BUTTON_DOWN         SDLK_DOWN
#define GCW_BUTTON_LEFT         SDLK_LEFT
#define GCW_BUTTON_RIGHT        SDLK_RIGHT
#define GCW_BUTTON_A            SDLK_LCTRL
#define GCW_BUTTON_B            SDLK_LALT
#define GCW_BUTTON_X            SDLK_SPACE
#define GCW_BUTTON_Y            SDLK_LSHIFT
#define GCW_BUTTON_L1           SDLK_TAB
#define GCW_BUTTON_R1           SDLK_BACKSPACE
#define GCW_BUTTON_L2           SDLK_PAGEUP
#define GCW_BUTTON_R2           SDLK_PAGEDOWN
#define GCW_BUTTON_SELECT       SDLK_ESCAPE
#define GCW_BUTTON_START        SDLK_RETURN
#define GCW_BUTTON_L3           SDLK_KP_DIVIDE
#define GCW_BUTTON_R3           SDLK_KP_PERIOD
#define GCW_BUTTON_POWER        SDLK_HOME
#define GCW_BUTTON_VOLUP        0 //SDLK_PAUSE
//#define GCW_BUTTON_VOLDOWN      0
#define GCW_JOYSTICK_DEADZONE   1000

#define TRUE   1
#define FALSE  0
#define PI 3.1415926

///////////////////////////////////
/*  Structs                      */
///////////////////////////////////
struct mouse_state
{
  int x;
  int y;
  int button_left;
  int button_right;
};

struct joystick_state
{
  int j1_left;
  int j1_right;
  int j1_up;
  int j1_down;
  int button_l3;
  int j2_left;
  int j2_right;
  int j2_up;
  int j2_down;
  int button_r3;
  int pad_left;
  int pad_right;
  int pad_up;
  int pad_down;
  int button_a;
  int button_b;
  int button_x;
  int button_y;
  int button_l1;
  int button_l2;
  int button_r1;
  int button_r2;
  int button_select;
  int button_start;
  int button_power;
  int button_voldown;
  int button_volup;
  int escape;
  int any;
};

struct button_state
{
    int x,y;
    SDL_Surface* button;
    SDL_Surface* button_pressed;
    SDL_Surface* button_moved;
    Uint32 pressed_time;
    Uint32 moved_time;
};

struct sd_data
{
  int status;
  int full;   //0=empty(green),1=medium(white),2=full(red)
  char filesysname[25];
  char free_text[10];
  char max_text[10];
  char type[10];
};

///////////////////////////////////
/*  Globals                      */
///////////////////////////////////
SDL_Surface* screen;   		    // screen to work
int done=0;
TTF_Font* font;                 // used font
SDL_Joystick* joystick;         // used joystick
joystick_state mainjoystick;
mouse_state mainmouse;
int volume=120;
int scanlines=0;
int fullscreen=0;
Uint8* keys=SDL_GetKeyState(NULL);
int rg_x=90;
int rg_y=50;
int last_pressedkey=0;
int battery_level=0;
int battery_charging=0;
Uint32 battery_checktime=0;
int mouse_active=0;
//int sd1_readed=0;       // 0=no exist, 1=reading, 2=readed
//int sd2_readed=0;
//char sd1[20];
//char sd2[20];
sd_data sd_1;
sd_data sd_2;
pthread_t sd_th;

// strings
int view_author=FALSE;
const char* author="(c) Rafa Vico 2019";
const char* version="1.3d";
const char* msg[6]={
"Press L1 + START to exit.",
"Press L1 + X to play a sound.",
"Last detected key:",
"Press L2 + R2 to rumble.",
"Press POWER + R1 to de/activate mouse.",
"reading..."
};

const char* key_table[18]={
"SDLK_UP",
"SDLK_DOWN",
"SDLK_LEFT",
"SDLK_RIGHT",
"SDLK_LCTRL",
"SDLK_LALT",
"SDLK_SPACE",
"SDLK_LSHIFT",
"SDLK_TAB",
"SDLK_BACKSPACE",
"SDLK_PAGEUP",
"SDLK_PAGEDOWN",
"SDLK_ESCAPE",
"SDLK_RETURN",
"SDLK_KP_DIVIDE",
"SDLK_KP_PERIOD",
"SDLK_HOME",
"Not defined"
};

int key_val[18]={
SDLK_UP,
SDLK_DOWN,
SDLK_LEFT,
SDLK_RIGHT,
SDLK_LCTRL,
SDLK_LALT,
SDLK_SPACE,
SDLK_LSHIFT,
SDLK_TAB,
SDLK_BACKSPACE,
SDLK_PAGEUP,
SDLK_PAGEDOWN,
SDLK_ESCAPE,
SDLK_RETURN,
SDLK_KP_DIVIDE,
SDLK_KP_PERIOD,
SDLK_HOME,
0
};

// graphics
SDL_Surface *rg350_back;
SDL_Surface *rg350_padup;
SDL_Surface *rg350_padup_press;
SDL_Surface *rg350_paddown;
SDL_Surface *rg350_paddown_press;
SDL_Surface *rg350_padleft;
SDL_Surface *rg350_padleft_press;
SDL_Surface *rg350_padright;
SDL_Surface *rg350_padright_press;
SDL_Surface *rg350_power;
SDL_Surface *rg350_power_press;
SDL_Surface *rg350_select;
SDL_Surface *rg350_select_press;
SDL_Surface *rg350_start;
SDL_Surface *rg350_start_press;
SDL_Surface *rg350_a;
SDL_Surface *rg350_a_press;
SDL_Surface *rg350_b;
SDL_Surface *rg350_b_press;
SDL_Surface *rg350_x;
SDL_Surface *rg350_x_press;
SDL_Surface *rg350_y;
SDL_Surface *rg350_y_press;
SDL_Surface *rg350_l1;
SDL_Surface *rg350_l1_press;
SDL_Surface *rg350_l2;
SDL_Surface *rg350_l2_press;
SDL_Surface *rg350_r1;
SDL_Surface *rg350_r1_press;
SDL_Surface *rg350_r2;
SDL_Surface *rg350_r2_press;
SDL_Surface *rg350_voldown;
SDL_Surface *rg350_voldown_press;
SDL_Surface *rg350_volup;
SDL_Surface *rg350_volup_press;
SDL_Surface *rg350_stick;
SDL_Surface *rg350_stick_mov;
SDL_Surface *rg350_stick_press;
SDL_Surface *info_power;
SDL_Surface *info_select;
SDL_Surface *info_start;
SDL_Surface *info_volup;
SDL_Surface *info_voldown;
SDL_Surface *info_padup;
SDL_Surface *info_paddown;
SDL_Surface *info_padleft;
SDL_Surface *info_padright;
SDL_Surface *info_btna;
SDL_Surface *info_btnb;
SDL_Surface *info_btnx;
SDL_Surface *info_btny;
SDL_Surface *info_btnl1;
SDL_Surface *info_btnl2;
SDL_Surface *info_btnl3;
SDL_Surface *info_btnr1;
SDL_Surface *info_btnr2;
SDL_Surface *info_btnr3;
SDL_Surface *rg350_battery;
SDL_Surface *rg350_battery2;
SDL_Surface *sdcard_0;
SDL_Surface *sdcard_1;
SDL_Surface *sdcard_2;
//sonidos
Mix_Chunk *sound_tone;

button_state joy1;
button_state joy2;
button_state padup;
button_state paddown;
button_state padleft;
button_state padright;
button_state btna;
button_state btnb;
button_state btnx;
button_state btny;
button_state btnsel;
button_state btnst;
button_state btnl1;
button_state btnl2;
button_state btnr1;
button_state btnr2;
button_state btnpw;
button_state btnvu;
button_state btnvd;

Shake_Device *device;
Shake_Effect effect;
int shake_id;

///////////////////////////////////
/*  Function declarations        */
///////////////////////////////////
void process_events();
void process_joystick();

///////////////////////////////////
/*  Debug Functions              */
///////////////////////////////////
/*std::string getCurrentDateTime( std::string s )
{
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
}

void Logger( std::string logMsg )
{

    std::string filePath = "/usr/local/home/log_"+getCurrentDateTime("date")+".txt";
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}*/

///////////////////////////////////
/*  Draw a pixel in surface      */
///////////////////////////////////
void putpixel(SDL_Surface *dst, int x, int y, Uint32 pixel)
{
    int byteperpixel = dst->format->BytesPerPixel;
    Uint8 *p = (Uint8*)dst->pixels + y * dst->pitch + x * byteperpixel;
    // Adress to pixel
    *(Uint32 *)p = pixel;
}

///////////////////////////////////
/*  Draw a line in surface       */
///////////////////////////////////
void drawLine(SDL_Surface* dst, int x0, int y0, int x1, int y1, Uint32 pixel)
{
    int i;
    double x = x1 - x0;
    double y = y1 - y0;
    double length = sqrt( x*x + y*y );
    double addx = x / length;
    double addy = y / length;
    x = x0;
    y = y0;

    for ( i = 0; i < length; i += 1) {
        putpixel(dst, x, y, pixel);
        x += addx;
        y += addy;
    }
}

///////////////////////////////////
/*  Print text in surface        */
///////////////////////////////////
void draw_text(SDL_Surface* dst, char* string, int x, int y, int fR, int fG, int fB)
{
  if(dst && string && font)
  {
    SDL_Color foregroundColor={fR,fG,fB};
    SDL_Surface *textSurface=TTF_RenderText_Blended(font,string,foregroundColor);
    if(textSurface)
    {
      SDL_Rect textLocation={x,y,0,0};
      SDL_BlitSurface(textSurface,NULL,dst,&textLocation);
      SDL_FreeSurface(textSurface);
    }
  }
}

///////////////////////////////////
/*  Return text width             */
///////////////////////////////////
int text_width(char* string)
{
  int nx=0,ny=0;
  TTF_SizeText(font,string,&nx,&ny);

  return nx;
}

///////////////////////////////////
/*  Return disk's filesystem     */
///////////////////////////////////
/*void filesystemname(int id, sd_data& result)
{
  sprintf(result.filesysname,"%i",id);
  return;
  switch(id)
  {
    case 1:
    case 4:
    case 5:
    case 6:
    case 0x0e:
    case 0x0f:
      sprintf(result.filesysname,"FAT");
      break;
    case 7:
      sprintf(result.filesysname,"exFAT");
      break;
    case 0x0b:
    case 0x0c:
    case 0x1b:
    case 0x1c:
      sprintf(result.filesysname,"FAT32");
      break;
    case 0x83:
      sprintf(result.filesysname,"Ext3/Ext4");
      break;
    default:
      sprintf(result.filesysname,"Unknown");
      break;
  }
}*/

///////////////////////////////////
/*  Return size of a drive       */
///////////////////////////////////
void sdsize(const char* path, sd_data& result) {
	struct statvfs b;

	int ret=statvfs(path, &b);
	if (ret==0)
	{
		unsigned long freeMiB=((unsigned long long)b.f_bfree * b.f_bsize) / (1024 * 1024);
		unsigned long totalMiB=((unsigned long long)b.f_blocks * b.f_frsize) / (1024 * 1024);
		if (totalMiB >= 10000)
		{
      if((freeMiB / 1024)>=(totalMiB / 1024)/2)
        result.full=0;
      else if((freeMiB / 1024)>(totalMiB / 1024)/10)
        result.full=1;
      else
        result.full=2;
      sprintf(result.free_text,"%d.%d",(freeMiB / 1024),((freeMiB % 1024) * 10) / 1024);
      sprintf(result.max_text,"/%d.%d",(totalMiB / 1024),((totalMiB % 1024) * 10) / 1024);
      sprintf(result.type,"GiB");
		}
		else
		{
      if(freeMiB>=totalMiB/2)
        result.full=0;
      else if(freeMiB>=totalMiB/10)
        result.full=1;
      else
        result.full=2;
      sprintf(result.free_text,"%d",freeMiB);
      sprintf(result.max_text,"/%d",totalMiB);
      sprintf(result.type,"MiB");
		}
	}
}

///////////////////////////////////
/*  Thread that read the disks   */
///////////////////////////////////
static void* readdisks_thd(void* p)
{
  // card1
  sd_1.status=1;
  sdsize("/usr/local/home",sd_1);
  sd_1.status=2;

  // card2
  struct stat buffer;
  if(stat("/media/sdcard/", &buffer)==0)
  {
    sd_2.status=1;
    sdsize("/media/sdcard/",sd_2);
    sd_2.status=2;
  }
	return NULL;
}

///////////////////////////////////
/*  Return true if charging?     */
///////////////////////////////////
unsigned short is_batterycharging()
{
	FILE *usbdev = NULL;
	usbdev = fopen("/sys/class/power_supply/usb/online", "r");
	if(usbdev)
	{
		int usbval = 0;
		fscanf(usbdev,"%d",&usbval);
		fclose(usbdev);
		// if connected to usb, return 255
		if(usbval == 1)
			return 1;
    else
      return 0;
	}
}

///////////////////////////////////
/*  Read battery level           */
///////////////////////////////////
unsigned short get_batterylevel()
{
	FILE *batterydev=NULL;
	batterydev = fopen("/sys/class/power_supply/battery/capacity", "r");
	if(batterydev)
	{
		int battval = 0;
		fscanf(batterydev,"%d",&battval);
		fclose(batterydev);

		// return battery 0-100, could vary when charging
		if(battval>100)
      battval=100;
		return battval;
	}

	return 0;
}

///////////////////////////////////
/*  Return description key text  */
///////////////////////////////////
const char* get_keydata(int value)
{
  int f=0;
  for(f=0;f<18;f++)
  {
    if(value==key_val[f])
      return key_table[f];
  }
  return key_table[17]; // message "Not defined"
}

///////////////////////////////////
/*  Clear values from joystick   */
/*  structure                    */
///////////////////////////////////
void clear_joystick_state()
{
  mainjoystick.j1_left=0;
  mainjoystick.j1_right=0;
  mainjoystick.j1_up=0;
  mainjoystick.j1_down=0;
  mainjoystick.button_l3=0;
  mainjoystick.j2_left=0;
  mainjoystick.j2_right=0;
  mainjoystick.j2_up=0;
  mainjoystick.j2_down=0;
  mainjoystick.button_r3=0;
  mainjoystick.pad_left=0;
  mainjoystick.pad_right=0;
  mainjoystick.pad_up=0;
  mainjoystick.pad_down=0;
  mainjoystick.button_a=0;
  mainjoystick.button_b=0;
  mainjoystick.button_x=0;
  mainjoystick.button_y=0;
  mainjoystick.button_l1=0;
  mainjoystick.button_l2=0;
  mainjoystick.button_r1=0;
  mainjoystick.button_r2=0;
  mainjoystick.button_select=0;
  mainjoystick.button_start=0;
  mainjoystick.button_power=0;
  mainjoystick.button_voldown=0;
  mainjoystick.button_volup=0;
  mainjoystick.escape=0;
  mainjoystick.any=0;
}

///////////////////////////////////
/*  Clear values from joystick   */
/*  structure                    */
///////////////////////////////////
void clear_mouse_state()
{
  mainmouse.x=0;
  mainmouse.y=0;
  mainmouse.button_left=0;
  mainmouse.button_right=0;
}

///////////////////////////////////
/*  Init rumble device           */
///////////////////////////////////
void init_rumble()
{
	Shake_Init();

	if (Shake_NumOfDevices() > 0)
	{
		device = Shake_Open(0);

		Shake_InitEffect(&effect, SHAKE_EFFECT_PERIODIC);
		effect.periodic.waveform		= SHAKE_PERIODIC_SINE;
		effect.periodic.period			= 0.1*0x100;
		effect.periodic.magnitude		= 0x6000;
		effect.periodic.envelope.attackLength	= 0x100;
		effect.periodic.envelope.attackLevel	= 0;
		effect.periodic.envelope.fadeLength	= 0x100;
		effect.periodic.envelope.fadeLevel	= 0;
		effect.direction			= 0x4000;
		effect.length				= 2000;
		effect.delay				= 0;

		shake_id=Shake_UploadEffect(device, &effect);
    }
}

///////////////////////////////////
/*  Close rumble device          */
///////////////////////////////////
void end_rumble()
{
    Shake_EraseEffect(device, shake_id);
    Shake_Close(device);
	Shake_Quit();
}

///////////////////////////////////
/*  Rumble                       */
///////////////////////////////////
void play_rumble()
{
    Shake_Play(device, shake_id);
}

///////////////////////////////////
/*  Load graphic with alpha      */
///////////////////////////////////
void load_imgalpha(const char* file, SDL_Surface *&dstsurface)
{
  SDL_Surface *tmpsurface;

  tmpsurface=IMG_Load(file);
  if(tmpsurface)
  {
    dstsurface=SDL_CreateRGBSurface(SDL_SRCCOLORKEY, tmpsurface->w, tmpsurface->h, 16, 0,0,0,0);
    SDL_BlitSurface(tmpsurface,NULL,dstsurface,NULL);
    SDL_SetColorKey(dstsurface,SDL_SRCCOLORKEY,SDL_MapRGB(screen->format,255,0,255));
    SDL_FreeSurface(tmpsurface);
  }
}

///////////////////////////////////
/*  Init the app                 */
///////////////////////////////////
void init_game()
{
  // Initalizations
  srand(time(NULL));
  joystick=SDL_JoystickOpen(0);
  SDL_ShowCursor(0);

  Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 1024);

  TTF_Init();
  font=TTF_OpenFont("media/pixelberry.ttf", 8);

  // Graphics
  load_imgalpha("media/rg350_back.png",rg350_back);
  load_imgalpha("media/rg350_stick.png",rg350_stick);
  load_imgalpha("media/rg350_stick_moved.png",rg350_stick_mov);
  load_imgalpha("media/rg350_stick_pressed.png",rg350_stick_press);
  load_imgalpha("media/rg350_button_power.png",rg350_power);
  load_imgalpha("media/rg350_button_power_pressed.png",rg350_power_press);
  load_imgalpha("media/rg350_button_s.png",rg350_select);
  load_imgalpha("media/rg350_button_s_pressed.png",rg350_select_press);
  load_imgalpha("media/rg350_button_s.png",rg350_start);
  load_imgalpha("media/rg350_button_s_pressed.png",rg350_start_press);
  load_imgalpha("media/rg350_button_vol1.png",rg350_voldown);
  load_imgalpha("media/rg350_button_vol1_pressed.png",rg350_voldown_press);
  load_imgalpha("media/rg350_button_vol2.png",rg350_volup);
  load_imgalpha("media/rg350_button_vol2_pressed.png",rg350_volup_press);
  load_imgalpha("media/rg350_button_up.png",rg350_padup);
  load_imgalpha("media/rg350_button_up_pressed.png",rg350_padup_press);
  load_imgalpha("media/rg350_button_down.png",rg350_paddown);
  load_imgalpha("media/rg350_button_down_pressed.png",rg350_paddown_press);
  load_imgalpha("media/rg350_button_left.png",rg350_padleft);
  load_imgalpha("media/rg350_button_left_pressed.png",rg350_padleft_press);
  load_imgalpha("media/rg350_button_right.png",rg350_padright);
  load_imgalpha("media/rg350_button_right_pressed.png",rg350_padright_press);
  load_imgalpha("media/rg350_button_l1.png",rg350_l1);
  load_imgalpha("media/rg350_button_l1_pressed.png",rg350_l1_press);
  load_imgalpha("media/rg350_button_l2.png",rg350_l2);
  load_imgalpha("media/rg350_button_l2_pressed.png",rg350_l2_press);
  load_imgalpha("media/rg350_button_r1.png",rg350_r1);
  load_imgalpha("media/rg350_button_r1_pressed.png",rg350_r1_press);
  load_imgalpha("media/rg350_button_r2.png",rg350_r2);
  load_imgalpha("media/rg350_button_r2_pressed.png",rg350_r2_press);
  load_imgalpha("media/rg350_button_a.png",rg350_a);
  load_imgalpha("media/rg350_button_a_pressed.png",rg350_a_press);
  load_imgalpha("media/rg350_button_b.png",rg350_b);
  load_imgalpha("media/rg350_button_b_pressed.png",rg350_b_press);
  load_imgalpha("media/rg350_button_x.png",rg350_x);
  load_imgalpha("media/rg350_button_x_pressed.png",rg350_x_press);
  load_imgalpha("media/rg350_button_y.png",rg350_y);
  load_imgalpha("media/rg350_button_y_pressed.png",rg350_y_press);

  load_imgalpha("media/info_power.png",info_power);
  load_imgalpha("media/info_select.png",info_select);
  load_imgalpha("media/info_start.png",info_start);
  load_imgalpha("media/info_volup.png",info_volup);
  load_imgalpha("media/info_voldw.png",info_voldown);
  load_imgalpha("media/info_padup.png",info_padup);
  load_imgalpha("media/info_paddown.png",info_paddown);
  load_imgalpha("media/info_padleft.png",info_padleft);
  load_imgalpha("media/info_padright.png",info_padright);
  load_imgalpha("media/info_btna.png",info_btna);
  load_imgalpha("media/info_btnb.png",info_btnb);
  load_imgalpha("media/info_btnx.png",info_btnx);
  load_imgalpha("media/info_btny.png",info_btny);
  load_imgalpha("media/info_btnl1.png",info_btnl1);
  load_imgalpha("media/info_btnl2.png",info_btnl2);
  load_imgalpha("media/info_btnl3.png",info_btnl3);
  load_imgalpha("media/info_btnr1.png",info_btnr1);
  load_imgalpha("media/info_btnr2.png",info_btnr2);
  load_imgalpha("media/info_btnr3.png",info_btnr3);

  load_imgalpha("media/battery.png",rg350_battery);
  load_imgalpha("media/battery2.png",rg350_battery2);
  load_imgalpha("media/sd0.png",sdcard_0);
  load_imgalpha("media/sd1.png",sdcard_1);
  load_imgalpha("media/sd2.png",sdcard_2);

  // Set graphics to position
  joy1.button=rg350_stick;
  joy1.button_pressed=rg350_stick_press;
  joy1.button_moved=rg350_stick_mov;
  joy1.x=rg_x+11;
  joy1.y=rg_y+26;
  joy1.moved_time=-3000;
  joy1.pressed_time=-3000;
  joy2.button=rg350_stick;
  joy2.button_pressed=rg350_stick_press;
  joy2.button_moved=rg350_stick_mov;
  joy2.x=rg_x+116;
  joy2.y=rg_y+52;
  joy2.moved_time=-3000;
  joy2.pressed_time=-3000;
  padup.button=rg350_padup;
  padup.button_pressed=rg350_padup_press;
  padup.x=rg_x+12;
  padup.y=rg_y+45;
  padup.moved_time=-3000;
  padup.pressed_time=-3000;
  paddown.button=rg350_paddown;
  paddown.button_pressed=rg350_paddown_press;
  paddown.x=rg_x+12;
  paddown.y=rg_y+59;
  paddown.moved_time=-3000;
  paddown.pressed_time=-3000;
  padleft.button=rg350_padleft;
  padleft.button_pressed=rg350_padleft_press;
  padleft.x=rg_x+5;
  padleft.y=rg_y+52;
  padleft.moved_time=-3000;
  padleft.pressed_time=-3000;
  padright.button=rg350_padright;
  padright.button_pressed=rg350_padright_press;
  padright.x=rg_x+19;
  padright.y=rg_y+52;
  padright.moved_time=-3000;
  padright.pressed_time=-3000;
  btna.button=rg350_a;
  btna.button_pressed=rg350_a_press;
  btna.x=rg_x+124;
  btna.y=rg_y+27;
  btna.moved_time=-3000;
  btna.pressed_time=-3000;
  btnb.button=rg350_b;
  btnb.button_pressed=rg350_b_press;
  btnb.x=rg_x+117;
  btnb.y=rg_y+36;
  btnb.moved_time=-3000;
  btnb.pressed_time=-3000;
  btnx.button=rg350_x;
  btnx.button_pressed=rg350_x_press;
  btnx.x=rg_x+117;
  btnx.y=rg_y+20;
  btnx.moved_time=-3000;
  btnx.pressed_time=-3000;
  btny.button=rg350_y;
  btny.button_pressed=rg350_y_press;
  btny.x=rg_x+109;
  btny.y=rg_y+28;
  btny.moved_time=-3000;
  btny.pressed_time=-3000;
  btnsel.button=rg350_select;
  btnsel.button_pressed=rg350_select_press;
  btnsel.x=rg_x+23;
  btnsel.y=rg_y+13;
  btnsel.moved_time=-3000;
  btnsel.pressed_time=-3000;
  btnst.button=rg350_start;
  btnst.button_pressed=rg350_start_press;
  btnst.x=rg_x+110;
  btnst.y=rg_y+13;
  btnst.moved_time=-3000;
  btnst.pressed_time=-3000;
  btnpw.button=rg350_power;
  btnpw.button_pressed=rg350_power_press;
  btnpw.x=rg_x+44;
  btnpw.y=rg_y+76;
  btnpw.moved_time=-3000;
  btnpw.pressed_time=-3000;
  btnvu.button=rg350_volup;
  btnvu.button_pressed=rg350_volup_press;
  btnvu.x=rg_x+77;
  btnvu.y=rg_y+76;
  btnvu.moved_time=-3000;
  btnvu.pressed_time=-3000;
  btnvd.button=rg350_voldown;
  btnvd.button_pressed=rg350_voldown_press;
  btnvd.x=rg_x+77;
  btnvd.y=rg_y+76;
  btnvd.moved_time=-3000;
  btnvd.pressed_time=-3000;
  btnl1.button=rg350_l1;
  btnl1.button_pressed=rg350_l1_press;
  btnl1.x=rg_x+3;
  btnl1.y=rg_y+5;
  btnl1.moved_time=-3000;
  btnl1.pressed_time=-3000;
  btnl2.button=rg350_l2;
  btnl2.button_pressed=rg350_l2_press;
  btnl2.x=rg_x+19;
  btnl2.y=rg_y+5;
  btnl2.moved_time=-3000;
  btnl2.pressed_time=-3000;
  btnr1.button=rg350_r1;
  btnr1.button_pressed=rg350_r1_press;
  btnr1.x=rg_x+120;
  btnr1.y=rg_y+5;
  btnr1.moved_time=-3000;
  btnr1.pressed_time=-3000;
  btnr2.button=rg350_r2;
  btnr2.button_pressed=rg350_r2_press;
  btnr2.x=rg_x+109;
  btnr2.y=rg_y+5;
  btnr2.moved_time=-3000;
  btnr2.pressed_time=-3000;

  // Load sounds
  sound_tone=Mix_LoadWAV("media/tone.wav");

  // battery and rumble init
  init_rumble();
  battery_level=get_batterylevel();
}

///////////////////////////////////
/*  Finish app, free memory      */
///////////////////////////////////
void end_game()
{
  SDL_FillRect(screen, NULL, 0x000000);

  if(SDL_JoystickOpened(0))
    SDL_JoystickClose(joystick);

  // Free graphics
  if(rg350_back)
    SDL_FreeSurface(rg350_back);
  if(rg350_power)
    SDL_FreeSurface(rg350_power);
  if(rg350_power_press)
    SDL_FreeSurface(rg350_power_press);
  if(rg350_select)
    SDL_FreeSurface(rg350_select);
  if(rg350_select_press)
    SDL_FreeSurface(rg350_select_press);
  if(rg350_start)
    SDL_FreeSurface(rg350_start);
  if(rg350_start_press)
    SDL_FreeSurface(rg350_start_press);
  if(rg350_stick)
    SDL_FreeSurface(rg350_stick);
  if(rg350_stick_mov)
    SDL_FreeSurface(rg350_stick_mov);
  if(rg350_stick_press)
    SDL_FreeSurface(rg350_stick_press);
  if(rg350_voldown)
    SDL_FreeSurface(rg350_voldown);
  if(rg350_voldown_press)
    SDL_FreeSurface(rg350_voldown_press);
  if(rg350_volup)
    SDL_FreeSurface(rg350_volup);
  if(rg350_volup_press)
    SDL_FreeSurface(rg350_volup_press);
  if(rg350_l1)
    SDL_FreeSurface(rg350_l1);
  if(rg350_l1_press)
    SDL_FreeSurface(rg350_l1_press);
  if(rg350_l2)
    SDL_FreeSurface(rg350_l2);
  if(rg350_l2_press)
    SDL_FreeSurface(rg350_l2_press);
  if(rg350_r1)
    SDL_FreeSurface(rg350_r1);
  if(rg350_r1_press)
    SDL_FreeSurface(rg350_r1_press);
  if(rg350_r2)
    SDL_FreeSurface(rg350_r2);
  if(rg350_r2_press)
    SDL_FreeSurface(rg350_r2_press);
  if(rg350_padup)
    SDL_FreeSurface(rg350_padup);
  if(rg350_padup_press)
    SDL_FreeSurface(rg350_padup_press);
  if(rg350_paddown)
    SDL_FreeSurface(rg350_paddown);
  if(rg350_paddown_press)
    SDL_FreeSurface(rg350_paddown_press);
  if(rg350_padleft)
    SDL_FreeSurface(rg350_padleft);
  if(rg350_padleft_press)
    SDL_FreeSurface(rg350_padleft_press);
  if(rg350_padright)
    SDL_FreeSurface(rg350_padright);
  if(rg350_padright_press)
    SDL_FreeSurface(rg350_padright_press);
  if(rg350_a)
    SDL_FreeSurface(rg350_a);
  if(rg350_a_press)
    SDL_FreeSurface(rg350_a_press);
  if(rg350_b)
    SDL_FreeSurface(rg350_b);
  if(rg350_b_press)
    SDL_FreeSurface(rg350_b_press);
  if(rg350_x)
    SDL_FreeSurface(rg350_x);
  if(rg350_x_press)
    SDL_FreeSurface(rg350_x_press);
  if(rg350_y)
    SDL_FreeSurface(rg350_y);
  if(rg350_y_press)
    SDL_FreeSurface(rg350_y_press);
  if(rg350_battery)
    SDL_FreeSurface(rg350_battery);
  if(rg350_battery2)
    SDL_FreeSurface(rg350_battery2);

  // Free sounds
  Mix_HaltChannel(-1);
  Mix_FreeChunk(sound_tone);
  Mix_CloseAudio();

  end_rumble();
}

///////////////////////////////////
/*  Read events from Power Button*/
/*  and Volume Buttons           */
/*  This buttons are not         */
/*  available at key array       */
/*  And read mouse events        */
///////////////////////////////////
void process_extrabuttons_events()
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      // type SDL_KEYDOWN give errors with power button, always returns a 0 after power key.
      case SDL_KEYUP:
        last_pressedkey=event.key.keysym.sym;
        switch(event.key.keysym.sym)
        {
          // power button, and volume buttons can be read directly in the key array.
          // It only can be detected by events.
          case GCW_BUTTON_VOLUP:
            if(mainjoystick.button_power==0)
            {
                mainjoystick.button_volup=1;
                mainjoystick.button_voldown=1;
            }
            break;
          case GCW_BUTTON_POWER:
            mainjoystick.button_power=1;
            break;
        }
        mainjoystick.any=1;
        break;
      case SDL_MOUSEMOTION:
        mouse_active=1;
        mainmouse.x=event.motion.x;
        mainmouse.y=event.motion.y;
        break;
      case SDL_MOUSEBUTTONDOWN:
        mouse_active=1;
        if(event.button.button==1)
          mainmouse.button_left=1;
        else
          mainmouse.button_right=1;
        break;
      case SDL_MOUSEBUTTONUP:
        mouse_active=1;
        if(event.button.button==1)
          mainmouse.button_left=0;
        else
          mainmouse.button_right=0;
        break;
      case SDL_JOYAXISMOTION:
        mouse_active=0;
        break;
    }
  }
}

///////////////////////////////////
/*  Process buttons events       */
///////////////////////////////////
void process_events()
{
  SDL_Event event;
  static int joy_pressed=FALSE;

  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case GCW_BUTTON_LEFT:
            mainjoystick.pad_left=1;
            break;
          case GCW_BUTTON_RIGHT:
            mainjoystick.pad_right=1;
            break;
          case GCW_BUTTON_UP:
            mainjoystick.pad_up=1;
            break;
          case GCW_BUTTON_DOWN:
            mainjoystick.pad_down=1;
            break;
          case GCW_BUTTON_Y:
            mainjoystick.button_y=1;
            break;
          case GCW_BUTTON_X:
            mainjoystick.button_x=1;
            break;
          case GCW_BUTTON_B:
            mainjoystick.button_b=1;
            break;
          case GCW_BUTTON_A:
            mainjoystick.button_a=1;
            break;
          case GCW_BUTTON_L1:
            mainjoystick.button_l1=1;
            break;
          case GCW_BUTTON_L2:
            mainjoystick.button_l2=1;
            break;
          case GCW_BUTTON_R1:
            mainjoystick.button_r1=1;
            break;
          case GCW_BUTTON_R2:
            mainjoystick.button_r2=1;
            break;
          case GCW_BUTTON_L3:
            mainjoystick.button_l3=1;
            break;
          case GCW_BUTTON_R3:
            mainjoystick.button_r3=1;
            break;
          case GCW_BUTTON_SELECT:
            mainjoystick.button_select=1;
            break;
          case GCW_BUTTON_START:
            mainjoystick.button_start=1;
            break;
          // Volume Up and Volume Down can't be detected individual
          case GCW_BUTTON_VOLUP:
            mainjoystick.button_volup=1;
            mainjoystick.button_voldown=1;
            break;
//          case GCW_BUTTON_VOLDOWN:
//            mainjoystick.button_voldown=1;
//            break;
          case GCW_BUTTON_POWER:
            mainjoystick.button_power=1;
            break;
        }
        mainjoystick.any=1;
        break;
      case SDL_JOYAXISMOTION:
        mouse_active=0;
        if(joy_pressed && SDL_JoystickGetAxis(joystick,0)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,0)<GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)<GCW_JOYSTICK_DEADZONE)
        {
          joy_pressed=FALSE;
        }

        if(!joy_pressed)
        {
            switch(event.jaxis.axis)
            {
              case 0:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j1_left=event.jaxis.value;
                  mainjoystick.j1_right=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j1_right=event.jaxis.value;
                  mainjoystick.j1_left=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 1:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j1_up=event.jaxis.value;
                  mainjoystick.j1_down=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j1_down=event.jaxis.value;
                  mainjoystick.j1_up=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 2:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j2_left=event.jaxis.value;
                  mainjoystick.j2_right=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j2_right=event.jaxis.value;
                  mainjoystick.j2_left=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 3:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j2_up=event.jaxis.value;
                  mainjoystick.j2_down=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j2_down=event.jaxis.value;
                  mainjoystick.j2_up=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
            }
        }
        break;
      }
  }
}

///////////////////////////////////
/*  Process keyboard and joystick*/
/*  (no events), and save in     */
/*  mainjoystick variable        */
///////////////////////////////////
void process_joystick()
{
  /*SDL_Event event;
  while(SDL_PollEvent(&event));*/

  if(keys[GCW_BUTTON_B])
    mainjoystick.button_b=1;
  if(keys[GCW_BUTTON_A])
    mainjoystick.button_a=1;
  if(keys[GCW_BUTTON_Y])
    mainjoystick.button_y=1;
  if(keys[GCW_BUTTON_X])
    mainjoystick.button_x=1;

  if(keys[GCW_BUTTON_LEFT])
    mainjoystick.pad_left=1;
  if(keys[GCW_BUTTON_RIGHT])
    mainjoystick.pad_right=1;
  if(keys[GCW_BUTTON_UP])
    mainjoystick.pad_up=1;
  if(keys[GCW_BUTTON_DOWN])
    mainjoystick.pad_down=1;

  if(SDL_JoystickGetAxis(joystick,0)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_left=SDL_JoystickGetAxis(joystick,0);
  if(SDL_JoystickGetAxis(joystick,0)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_right=SDL_JoystickGetAxis(joystick,0);
  if(SDL_JoystickGetAxis(joystick,1)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_up=SDL_JoystickGetAxis(joystick,1);
  if(SDL_JoystickGetAxis(joystick,1)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_down=SDL_JoystickGetAxis(joystick,1);
  if(SDL_JoystickGetAxis(joystick,2)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_left=SDL_JoystickGetAxis(joystick,2);
  if(SDL_JoystickGetAxis(joystick,2)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_right=SDL_JoystickGetAxis(joystick,2);
  if(SDL_JoystickGetAxis(joystick,3)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_up=SDL_JoystickGetAxis(joystick,3);
  if(SDL_JoystickGetAxis(joystick,3)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_down=SDL_JoystickGetAxis(joystick,3);

  /*if(keys[GCW_BUTTON_POWER])
    mainjoystick.button_power=1;
  if(keys[GCW_BUTTON_VOLUP])
  {
    mainjoystick.button_volup=1;
    mainjoystick.button_voldown=1;
  }
  if(keys[GCW_BUTTON_VOLDOWN])
  {
    mainjoystick.button_volup=1;
    mainjoystick.button_voldown=1;
  }*/

  if(keys[GCW_BUTTON_START])
    mainjoystick.button_start=1;
  if(keys[GCW_BUTTON_SELECT])
    mainjoystick.button_select=1;

  if(keys[GCW_BUTTON_L1])
    mainjoystick.button_l1=1;
  if(keys[GCW_BUTTON_R1])
    mainjoystick.button_r1=1;
  if(keys[GCW_BUTTON_L2])
    mainjoystick.button_l2=1;
  if(keys[GCW_BUTTON_R2])
    mainjoystick.button_r2=1;

  if(keys[GCW_BUTTON_L3])
    mainjoystick.button_l3=1;
  if(keys[GCW_BUTTON_R3])
    mainjoystick.button_r3=1;
}

///////////////////////////////////
/*  Draw screen, console and     */
/*  buttons                      */
///////////////////////////////////
void draw_game()
{
  Uint32 time=SDL_GetTicks();
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,16,16,16));

  // console
  SDL_Rect dest;
  dest.x=rg_x;
  dest.y=rg_y;
  if(rg350_back)
    SDL_BlitSurface(rg350_back,NULL,screen,&dest);

  // joystick1
  dest.x=rg_x+11+SDL_JoystickGetAxis(joystick,0)/5461;   // axis value (-32767,32768), I want draw it with 6 pixels displace
  dest.y=rg_y+26+SDL_JoystickGetAxis(joystick,1)/5461;
  // pressed buttons are drawed 3 seconds
  if((time-joy1.pressed_time)<3000)
  {
    if(joy1.button_pressed)
      SDL_BlitSurface(joy1.button_pressed,NULL,screen,&dest);
    dest.x=72;
    dest.y=78;
    if(info_btnl3)
      SDL_BlitSurface(info_btnl3,NULL,screen,&dest);
  }
  else
  {
    if((time-joy1.moved_time)<3000)
    {
        if(joy1.button_moved)
          SDL_BlitSurface(joy1.button_moved,NULL,screen,&dest);
    }
    else
    {
      if(joy1.button)
        SDL_BlitSurface(joy1.button,NULL,screen,&dest);
    }
  }

  // joystick2
  dest.x=rg_x+116+SDL_JoystickGetAxis(joystick,2)/5461;
  dest.y=rg_y+52+SDL_JoystickGetAxis(joystick,3)/5461;
  if((time-joy2.pressed_time)<3000)
  {
    if(joy2.button_pressed)
      SDL_BlitSurface(joy2.button_pressed,NULL,screen,&dest);
    dest.x=221;
    dest.y=104;
    if(info_btnr3)
      SDL_BlitSurface(info_btnr3,NULL,screen,&dest);
  }
  else
  {
    if((time-joy2.moved_time)<3000)
    {
      if(joy2.button_moved)
        SDL_BlitSurface(joy2.button_moved,NULL,screen,&dest);
    }
    else
    {
      if(joy2.button)
        SDL_BlitSurface(joy2.button,NULL,screen,&dest);
    }
  }

  // button a
  dest.x=btna.x;
  dest.y=btna.y;
  if((time-btna.pressed_time)<3000)
  {
    if(btna.button_pressed)
      SDL_BlitSurface(btna.button_pressed,NULL,screen,&dest);
    dest.x=224;
    dest.y=73;
    if(info_btna)
      SDL_BlitSurface(info_btna,NULL,screen,&dest);
  }
  else
  {
    if(btna.button)
      SDL_BlitSurface(btna.button,NULL,screen,&dest);
  }
  // button b
  dest.x=btnb.x;
  dest.y=btnb.y;
  if((time-btnb.pressed_time)<3000)
  {
    if(btnb.button_pressed)
      SDL_BlitSurface(btnb.button_pressed,NULL,screen,&dest);
    dest.x=216;
    dest.y=90;
    if(info_btnb)
      SDL_BlitSurface(info_btnb,NULL,screen,&dest);
  }
  else
  {
    if(btnb.button)
      SDL_BlitSurface(btnb.button,NULL,screen,&dest);
  }
  // button x
  dest.x=btnx.x;
  dest.y=btnx.y;
  if((time-btnx.pressed_time)<3000)
  {
    if(btnx.button_pressed)
      SDL_BlitSurface(btnx.button_pressed,NULL,screen,&dest);
    dest.x=216;
    dest.y=64;
    if(info_btnx)
      SDL_BlitSurface(info_btnx,NULL,screen,&dest);
  }
  else
  {
    if(btnx.button)
      SDL_BlitSurface(btnx.button,NULL,screen,&dest);
  }
  // button y
  dest.x=btny.x;
  dest.y=btny.y;
  if((time-btny.pressed_time)<3000)
  {
    if(btny.button_pressed)
      SDL_BlitSurface(btny.button_pressed,NULL,screen,&dest);
    dest.x=209;
    dest.y=82;
    if(info_btny)
      SDL_BlitSurface(info_btny,NULL,screen,&dest);
  }
  else
  {
    if(btny.button)
      SDL_BlitSurface(btny.button,NULL,screen,&dest);
  }
  // button up
  dest.x=padup.x;
  dest.y=padup.y;
  if((time-padup.pressed_time)<3000)
  {
    if(padup.button_pressed)
      SDL_BlitSurface(padup.button_pressed,NULL,screen,&dest);
    dest.x=71;
    dest.y=89;
    if(info_padup)
      SDL_BlitSurface(info_padup,NULL,screen,&dest);
  }
  else
  {
    if(padup.button)
      SDL_BlitSurface(padup.button,NULL,screen,&dest);
  }
  // button down
  dest.x=paddown.x;
  dest.y=paddown.y;
  if((time-paddown.pressed_time)<3000)
  {
    if(paddown.button_pressed)
      SDL_BlitSurface(paddown.button_pressed,NULL,screen,&dest);
    dest.x=59;
    dest.y=115;
    if(info_paddown)
      SDL_BlitSurface(info_paddown,NULL,screen,&dest);
  }
  else
  {
    if(paddown.button)
      SDL_BlitSurface(paddown.button,NULL,screen,&dest);
  }
  // button left
  dest.x=padleft.x;
  dest.y=padleft.y;
  if((time-padleft.pressed_time)<3000)
  {
    if(padleft.button_pressed)
      SDL_BlitSurface(padleft.button_pressed,NULL,screen,&dest);
    dest.x=63;
    dest.y=98;
    if(info_padleft)
      SDL_BlitSurface(info_padleft,NULL,screen,&dest);
  }
  else
  {
    if(padleft.button)
      SDL_BlitSurface(padleft.button,NULL,screen,&dest);
  }
  // button right
  dest.x=padright.x;
  dest.y=padright.y;
  if((time-padright.pressed_time)<3000)
  {
    if(padright.button_pressed)
      SDL_BlitSurface(padright.button_pressed,NULL,screen,&dest);
    dest.x=58;
    dest.y=105;
    if(info_padright)
      SDL_BlitSurface(info_padright,NULL,screen,&dest);
  }
  else
  {
    if(padright.button)
      SDL_BlitSurface(padright.button,NULL,screen,&dest);
  }
  // button power
  dest.x=btnpw.x;
  dest.y=btnpw.y;
  if((time-btnpw.pressed_time)<3000)
  {
    if(btnpw.button_pressed)
      SDL_BlitSurface(btnpw.button_pressed,NULL,screen,&dest);
    dest.x=106;
    dest.y=129;
    if(info_power)
      SDL_BlitSurface(info_power,NULL,screen,&dest);
  }
  else
  {
    if(btnpw.button)
      SDL_BlitSurface(btnpw.button,NULL,screen,&dest);
  }
  // button volup
  dest.x=btnvu.x;
  dest.y=btnvu.y;
  if((time-btnvu.pressed_time)<3000)
  {
    if(btnvu.button_pressed)
      SDL_BlitSurface(btnvu.button_pressed,NULL,screen,&dest);
    dest.x=184;
    dest.y=129;
    if(info_volup)
      SDL_BlitSurface(info_volup,NULL,screen,&dest);
  }
  else
  {
    if(btnvu.button)
      SDL_BlitSurface(btnvu.button,NULL,screen,&dest);
  }
  // button voldown
  dest.x=btnvd.x;
  dest.y=btnvd.y;
  if((time-btnvd.pressed_time)<3000)
  {
    if(btnvd.button_pressed)
      SDL_BlitSurface(btnvd.button_pressed,NULL,screen,&dest);
    dest.x=148;
    dest.y=129;
    if(info_voldown)
      SDL_BlitSurface(info_voldown,NULL,screen,&dest);
  }
  else
  {
    if(btnvd.button)
      SDL_BlitSurface(btnvd.button,NULL,screen,&dest);
  }
  // button l1
  dest.x=btnl1.x;
  dest.y=btnl1.y;
  if((time-btnl1.pressed_time)<3000)
  {
    if(btnl1.button_pressed)
      SDL_BlitSurface(btnl1.button_pressed,NULL,screen,&dest);
    dest.x=86;
    dest.y=40;
    if(info_btnl1)
      SDL_BlitSurface(info_btnl1,NULL,screen,&dest);
  }
  else
  {
    if(btnl1.button)
      SDL_BlitSurface(btnl1.button,NULL,screen,&dest);
  }
  // button l2
  dest.x=btnl2.x;
  dest.y=btnl2.y;
  if((time-btnl2.pressed_time)<3000)
  {
    if(btnl2.button_pressed)
      SDL_BlitSurface(btnl2.button_pressed,NULL,screen,&dest);
    dest.x=109;
    dest.y=40;
    if(info_btnl2)
      SDL_BlitSurface(info_btnl2,NULL,screen,&dest);
  }
  else
  {
    if(btnl2.button)
      SDL_BlitSurface(btnl2.button,NULL,screen,&dest);
  }
  // button r1
  dest.x=btnr1.x;
  dest.y=btnr1.y;
  if((time-btnr1.pressed_time)<3000)
  {
    if(btnr1.button_pressed)
      SDL_BlitSurface(btnr1.button_pressed,NULL,screen,&dest);
    dest.x=213;
    dest.y=40;
    if(info_btnr1)
      SDL_BlitSurface(info_btnr1,NULL,screen,&dest);
  }
  else
  {
    if(btnr1.button)
      SDL_BlitSurface(btnr1.button,NULL,screen,&dest);
  }
  // button r2
  dest.x=btnr2.x;
  dest.y=btnr2.y;
  if((time-btnr2.pressed_time)<3000)
  {
    if(btnr2.button_pressed)
      SDL_BlitSurface(btnr2.button_pressed,NULL,screen,&dest);
    dest.x=200;
    dest.y=40;
    if(info_btnr2)
      SDL_BlitSurface(info_btnr2,NULL,screen,&dest);
  }
  else
  {
    if(btnr2.button)
      SDL_BlitSurface(btnr2.button,NULL,screen,&dest);
  }
  // button select
  dest.x=btnsel.x;
  dest.y=btnsel.y;
  if((time-btnsel.pressed_time)<3000)
  {
    if(btnsel.button_pressed)
      SDL_BlitSurface(btnsel.button_pressed,NULL,screen,&dest);
    dest.x=53;
    dest.y=52;
    if(info_select)
      SDL_BlitSurface(info_select,NULL,screen,&dest);
  }
  else
  {
    if(btnsel.button)
      SDL_BlitSurface(btnsel.button,NULL,screen,&dest);
  }
  // button start
  dest.x=btnst.x;
  dest.y=btnst.y;
  if((time-btnst.pressed_time)<3000)
  {
    if(btnst.button_pressed)
      SDL_BlitSurface(btnst.button_pressed,NULL,screen,&dest);
    dest.x=206;
    dest.y=52;
    if(info_start)
      SDL_BlitSurface(info_start,NULL,screen,&dest);
  }
  else
  {
    if(btnst.button)
      SDL_BlitSurface(btnst.button,NULL,screen,&dest);
  }

  if(mouse_active)
  {
    // draw left button
    dest.x=rg_x+37;
    dest.y=rg_y+55;
    dest.w=32;
    dest.h=10;
    if(mainmouse.button_left)
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,192,192,0));
    else
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,64,64,0));

    // draw right button
    dest.x=rg_x+37+34;
    dest.y=rg_y+55;
    dest.w=32;
    dest.h=10;
    if(mainmouse.button_right)
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,192,192,0));
    else
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,64,64,0));

    // draw coordinates
    char mxval[5];
    char myval[5];
    sprintf(mxval,"%i",mainmouse.x);
    sprintf(myval,"%i",mainmouse.y);
    draw_text(screen,mxval,rg_x+65,rg_y+27,255,255,0);
    draw_text(screen,myval,rg_x+65,rg_y+37,255,255,0);

    // draw cross
    int posmx,posmy;
    posmx=mainmouse.x/5.5;
    posmy=mainmouse.y/5.8;
    drawLine(screen,131-3+posmx,70+posmy,131+4+posmx,70+posmy,SDL_MapRGB(screen->format,255,255,255));
    drawLine(screen,131+posmx,70-3+posmy,131+posmx,70+4+posmy,SDL_MapRGB(screen->format,255,255,255));

  }
  else
  {
    // joystick 1 range
    int j1_x,j1_y;
    j1_x=SDL_JoystickGetAxis(joystick,0);
    j1_y=SDL_JoystickGetAxis(joystick,1);
    char j1xval[5];
    sprintf(j1xval,"%.2f",double(j1_x)/32767.0);
    draw_text(screen,j1xval,131,69,255,0,255);
    char j1yval[5];
    sprintf(j1yval,"%.2f",double(j1_y)/32767.0);
    draw_text(screen,j1yval,131,79,255,0,255);

    // draw cross
    int pos1x,pos1y;
    pos1x=(j1_x+32767)/1130;
    pos1y=(j1_y+32767)/1598;
    drawLine(screen,131-3+pos1x,70+pos1y,131+4+pos1x,70+pos1y,SDL_MapRGB(screen->format,255,0,255));
    drawLine(screen,131+pos1x,70-3+pos1y,131+pos1x,70+4+pos1y,SDL_MapRGB(screen->format,255,0,255));

    // joystick 2 range
    int j2_x,j2_y;
    j2_x=SDL_JoystickGetAxis(joystick,2);
    j2_y=SDL_JoystickGetAxis(joystick,3);
    char j2xval[5];
    sprintf(j2xval,"%.2f",double(j2_x)/32767.0);
    draw_text(screen,j2xval,168,93,0,255,255);
    char j2yval[5];
    sprintf(j2yval,"%.2f",double(j2_y)/32767.0);
    draw_text(screen,j2yval,168,103,0,255,255);

    // draw cross
    int pos2x,pos2y;
    pos2x=(j2_x+32767)/1130;
    pos2y=(j2_y+32767)/1598;
    drawLine(screen,131-3+pos2x,70+pos2y,131+4+pos2x,70+pos2y,SDL_MapRGB(screen->format,0,255,255));
    drawLine(screen,131+pos2x,70-3+pos2y,131+pos2x,70+4+pos2y,SDL_MapRGB(screen->format,0,255,255));
  }

  // battery icon
  dest.x=rg_x+180;
  dest.y=rg_y+20;
  if(rg350_battery)
    SDL_BlitSurface(rg350_battery,NULL,screen,&dest);

  if(!is_batterycharging())
  {
    // battery percent, not drawing when charging because value can vary
    char batlev[7];
    sprintf(batlev,"%2i %%",battery_level);
    draw_text(screen,batlev,dest.x-1,dest.y+46,255,255,255);
  }

  // draw actual power
  dest.x=dest.x+1;
  dest.w=14;
  if(battery_level<=100)
    dest.h=battery_level*39/100;
  else
    dest.h=39;
  dest.y=dest.y+43-dest.h; // capacity rectangle is 38 pixels high

  if(battery_level>24)
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,64,192,64));  // green power
  else if(battery_level>0)
      SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,192,64,64));  // red power

  // if connected to usb, draw charging battery icon
  dest.x=rg_x+180;
  dest.y=rg_y+20;
  if(battery_charging)
    if(rg350_battery2)
      SDL_BlitSurface(rg350_battery2,NULL,screen,&dest);

  // info texts
  draw_text(screen, (char*)msg[0],10,180,255,255,0);
  draw_text(screen, (char*)msg[1],10,190,255,255,0);
  draw_text(screen, (char*)msg[3],10,200,255,255,0);
  draw_text(screen, (char*)msg[4],10,210,255,255,0);
  if(view_author)
    draw_text(screen, (char*)author,10,230,255,0,255);
  draw_text(screen, (char*)version,300,230,69,69,69);

  // print last key
  char lastkey[15];
  sprintf(lastkey,"%i [0x%04X]",last_pressedkey,last_pressedkey);
  draw_text(screen, (char*)msg[2],10,160,0,255,255);
  draw_text(screen, lastkey,110,160,255,255,255);
  char* key_desc=(char*)get_keydata(last_pressedkey);
  draw_text(screen,key_desc,180,160,0,255,255);

  // sdcards
  dest.x=133;
  dest.y=10;
  switch(sd_1.status)
  {
    case 1:
      if(sdcard_0)
        SDL_BlitSurface(sdcard_0,NULL,screen,&dest);
      draw_text(screen,(char*)msg[5],120-text_width((char*)msg[5]),20,255,255,255);
      break;
    case 2:
      if(sdcard_1)
        SDL_BlitSurface(sdcard_1,NULL,screen,&dest);
      if(sd_1.full==0)
        draw_text(screen,sd_1.free_text,120-text_width(sd_1.free_text)-text_width(sd_1.max_text)-text_width(sd_1.type),20,64,192,64);
      else if(sd_1.full==1)
        draw_text(screen,sd_1.free_text,120-text_width(sd_1.free_text)-text_width(sd_1.max_text)-text_width(sd_1.type),20,255,255,255);
      else
        draw_text(screen,sd_1.free_text,120-text_width(sd_1.free_text)-text_width(sd_1.max_text)-text_width(sd_1.type),20,192,64,64);
      draw_text(screen,sd_1.max_text,120-text_width(sd_1.max_text)-text_width(sd_1.type),20,255,255,255);
      draw_text(screen,sd_1.type,120-text_width(sd_1.type),20,192,192,192);
      // draw_text(screen,sd_1.filesysname,120-text_width(sd_1.filesysname),30,192,192,192);   // filesystem type
    break;
  }
  dest.x=163;
  switch(sd_2.status)
  {
    case 1:
      if(sdcard_0)
        SDL_BlitSurface(sdcard_0,NULL,screen,&dest);
      draw_text(screen,(char*)msg[5],197,20,255,255,255);
      break;
    case 2:
      if(sdcard_2)
        SDL_BlitSurface(sdcard_2,NULL,screen,&dest);
      if(sd_2.full==0)
        draw_text(screen,sd_2.free_text,197,20,64,192,64);
      else if(sd_2.full==1)
        draw_text(screen,sd_2.free_text,197,20,255,255,255);
      else
        draw_text(screen,sd_2.free_text,197,20,192,64,64);
      draw_text(screen,sd_2.max_text,197+text_width(sd_2.free_text),20,255,255,255);
      draw_text(screen,sd_2.type,197+text_width(sd_2.free_text)+text_width(sd_2.max_text),20,192,192,192);
      //draw_text(screen,sd_2.filesysname,197,30,192,192,192);  // filesystem type
      break;
  }

  /*
  // test all pressed keys
  int f,ln=16;
  for(f=0;f<2000;f++)
  {
    if(keys[f]==1)
    {
      draw_text(screen,(char*)"-",0,ln,255,128,128);
      char val[10];
      sprintf(val,"%i",f);
      draw_text(screen,&val[0],10,ln,255,255,255);
      ln+=9;
    }
  }*/
}

///////////////////////////////////
/*  Check buttons, update actions*/
///////////////////////////////////
void update_game()
{
    static int active_sound=0;
    static int active_rumble=0;

    clear_joystick_state();
    process_extrabuttons_events();
    process_joystick();
    //process_events();

    // exit
    if(mainjoystick.button_l1 && mainjoystick.button_start)
        done=1;
    // play sound
    if(mainjoystick.button_l1 && mainjoystick.button_x && !active_sound)
    {
        active_sound=1;
        Mix_PlayChannel(1,sound_tone,0);
    }
    if(!mainjoystick.button_l1 || !mainjoystick.button_x)
      active_sound=0;
    // show author
    if(mainjoystick.button_select && mainjoystick.button_start)
        view_author=TRUE;
    // rumble
    if(mainjoystick.button_l2 && mainjoystick.button_r2 && !active_rumble)
    {
        active_rumble=1;
        play_rumble();
    }
    if(!mainjoystick.button_l2 || !mainjoystick.button_r2)
      active_rumble=0;
    // take screenshot
    /*if(mainjoystick.button_l3)
        SDL_SaveBMP(screen,"/usr/local/home/rgtest.bmp");*/

    // set pressed time
    if(mainjoystick.button_a)
        btna.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_b)
        btnb.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_x)
        btnx.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_y)
        btny.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_l1)
        btnl1.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_l2)
        btnl2.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_r1)
        btnr1.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_r2)
        btnr2.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_select)
        btnsel.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_start)
        btnst.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_power)
        btnpw.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_volup)
        btnvu.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_voldown)
        btnvd.pressed_time=SDL_GetTicks();
    if(mainjoystick.pad_up)
        padup.pressed_time=SDL_GetTicks();
    if(mainjoystick.pad_down)
        paddown.pressed_time=SDL_GetTicks();
    if(mainjoystick.pad_left)
        padleft.pressed_time=SDL_GetTicks();
    if(mainjoystick.pad_right)
        padright.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_l3)
        joy1.pressed_time=SDL_GetTicks();
    if(mainjoystick.button_r3)
        joy2.pressed_time=SDL_GetTicks();
    if(mainjoystick.j1_left<-GCW_JOYSTICK_DEADZONE || mainjoystick.j1_right>GCW_JOYSTICK_DEADZONE || mainjoystick.j1_down>GCW_JOYSTICK_DEADZONE || mainjoystick.j1_up<-GCW_JOYSTICK_DEADZONE)
        joy1.moved_time=SDL_GetTicks();
    if(mainjoystick.j2_left<-GCW_JOYSTICK_DEADZONE || mainjoystick.j2_right>GCW_JOYSTICK_DEADZONE || mainjoystick.j2_down>GCW_JOYSTICK_DEADZONE || mainjoystick.j2_up<-GCW_JOYSTICK_DEADZONE)
        joy2.moved_time=SDL_GetTicks();

    // read battery every 2 seconds. Read it very fast can produce errors
    if(SDL_GetTicks()-battery_checktime>2000)
    {
        battery_charging=is_batterycharging();
        battery_level=get_batterylevel();
        battery_checktime=SDL_GetTicks();
    }
}

///////////////////////////////////
/*  Init                         */
///////////////////////////////////
int main(int argc, char *argv[])
{
  if(SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0)
		return 0;

  screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen==NULL)
      return 0;

  SDL_JoystickEventState(SDL_ENABLE);
  Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 1024);

  sd_1.status=0;
  sd_2.status=0;
  pthread_create(&sd_th, NULL, readdisks_thd, NULL);

  init_game();

  const int GAME_FPS=60;
  Uint32 start_time;

  while(!done)
	{
    start_time=SDL_GetTicks();
    update_game();
    draw_game();

    SDL_Flip(screen);

    // set FPS 60
    if(1000/GAME_FPS>SDL_GetTicks()-start_time)
      SDL_Delay(1000/GAME_FPS-(SDL_GetTicks()-start_time));
	}

	pthread_cancel(sd_th);
	pthread_join(sd_th, NULL);
  end_game();

  return 1;
}
