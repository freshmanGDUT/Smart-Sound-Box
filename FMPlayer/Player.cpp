#ifndef PLAY_H_INCLUDE
#define PLAY_H_INCLUDE

#include <iostream>
#include "CppMPCPlayAPI.h"
using namespace std;

static string control_bit = "";

/* Set the path 
 * path: path that LiZhiFMCppAPI.py saved
 */ 
void start(string path){
  // initialize python virtual env
  Py_Initialize();
  // set up the path that module in 
  string module_path = path;
  // add the module path
  string chdir_cmd = string("sys.path.append(\"") + module_path + "\")";
  const char *cstr_cmd = chdir_cmd.c_str();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString(cstr_cmd);
  
  control_bit = "start";
}

/* Player run initially
 * mpcplayer: MPCPlayer object to use the api of MPCPlayer
 * lizhifm: LiZhiFM object to use the api of LiZhiFMCppAPI.py
 * returen val: <song's url, program's url>
 */
pair<string, string> run(MPCPlayer &mpcplayer, PyObject *&lizhifm){
  // define a value save song url and program url
  pair<string, string> url;
  // judge start or not
  if (control_bit != "start") {
    cout << "[ERROR] start first" << endl; 
    exit(0);
  }
  // import some modules
  PyObject *lizhifmAPI = PyImport_ImportModule("LiZhiFMCppAPI"); // load py doc
  PyObject *lizhifmDict = PyModule_GetDict(lizhifmAPI);                       // get func dict in py doc
  lizhifm = PyDict_GetItemString(lizhifmDict, "LiZhiFM");                       // get LiZhiFM 
  lizhifm = PyObject_CallObject(lizhifm, NULL);                                       // create LiZhiFM object
  // Initialize mpc_player
  url = mpcplayer.PlayInit(lizhifm);
  // Add songs to play list and save the path of song's img
  mpcplayer.AddSongToPlayList(url.first);
  // Play song
  mpcplayer.SongPlay();
  // set contro_bit
  control_bit = "run";
  return url;
}

/* Run the main thread that support the player
 * mpcplayer: MPCPlayer object to use the api of MPCPlayer
 * lizhifm: LiZhiFM object to use the api of LiZhiFMCppAPI.py
 * url: <song's url, program's url>
 * return val: exit flag
 */
int run_main_thread(MPCPlayer &mpcplayer, PyObject *&lizhifm, pair<string, string>&url) {
  // judge run or not
  if (control_bit != "run") {
    cout << "[ERROR] Please run the run func first" << endl;
    exit(0);
}
  // main thread
  int num_int = 0;
  PyObject *p_songs_url;
  int song_num = 0;
  int flag = 0;
  string command = "";
  string num = "";
  string volume = "";
  cout << "Enter a(pause), b(play), c(uper direct), d(show playlist), e(set volume), f(speed)， g(back), h(exit)" << endl;  
  cout << "Please enter your command: ";
  cin >> command;
  cin.clear();
  cin.ignore();
  char c = *command.c_str();
  switch(c) {
    case 'a':
      mpcplayer.StopPlay();
      break;
    case 'b':
      cout << "Choose the num of the fm(enter '0' means default):" ;
      cin >> num;
      cin.clear();     // reset flag bit 
      cin.ignore();  // clear buffer
      if (num != "0") {
        ConvertFromString(num_int, num);
        mpcplayer.SongPlay(num_int);
      } else {
        mpcplayer.SongPlay();
      }
      break;
      case 'c':
        mpcplayer.GetListOfSong(lizhifm, url.second, p_songs_url, song_num);
        mpcplayer.GetSongsURL(lizhifm, p_songs_url, song_num, url);
        mpcplayer.AddSongToPlayList(url.first);
        break;
      case 'd':
        mpcplayer.ShowPlayList(); 
        break;
      case 'e':
        cout << "Enter the volume you want(127 is 100%): ";
        cin >> volume;
        cin.clear();
        cin.ignore();
        mpcplayer.SetVolume(volume);
        break;
      case 'f':
        mpcplayer.PlaySpeed();
        break;
      case 'g':
        mpcplayer.PlayBack();
        break;
      case 'h':
        mpcplayer.MpcStop();
        flag = 1;
        break;
      default:
        cout << "Please enter the right choice" << endl;
        break;
    }
    return flag;
}

/* End the Player
 */
void end(){
    Py_Finalize();
}
int main(int argc, char**argv){
  start("/root");
  MPCPlayer mpcplayer;
  PyObject *lizhifm;
  pair<string, string> url;
  url = run(mpcplayer, lizhifm);
  int flag = 0;
  string song_img_path;
  while (1) {
    flag = run_main_thread(mpcplayer, lizhifm, url);
    if (flag == 1) {
      break;
    }
    song_img_path = mpcplayer.get_song_img_path();
    cout << song_img_path << endl;
  } 
  end();
  return 0;
}

#endif
