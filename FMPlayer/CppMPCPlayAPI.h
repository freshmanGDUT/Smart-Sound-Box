#ifndef CPPMPCPLAYAPI_H_INCLUDE_
#define CPPMPCPLAYAPI_H_INCLUDE_

#include <utility>
#include <cctype>
#include <sstream>
#include "Python.h"
#include "CppMPCPlay.h"

using namespace std;

/* Save program url got from python
 * lizhifm: lizhifm object
 * program_url_name: program's url
 */
void MPCPlayer::save_program_url(PyObject *lizhifm, string &program_url_name) {
//   char *program_url_name_char = "";
//   PyObject *p_program_url_name = PyObject_GetAttrString(lizhifm, "url");
//   PyArg_Parse(p_program_url_name, "s", &program_url_name_char);
//   program_url_name = string(program_url_name_char);
//   program_url = program_url_name;
}

/* Save classes got from python
 *  lizhifm: lizhifm object
 *  classes_name: classes name
 */
void MPCPlayer::save_classes(PyObject* lizhifm, string& classes_name) {
    char *classes_char = "";
    PyObject *p_classes = PyObject_GetAttrString(lizhifm, "classes");
    PyArg_Parse(p_classes, "s", &classes_char);
    classes_name = string(classes_char);
    classes = classes_name;
}

/* Save program got from python
 * lizhifm: lizhifm object
 * program_name: program name
 */
void MPCPlayer::save_program(PyObject *lizhifm, string &program_name) {
    char *program_char = "";
    PyObject *p_program = PyObject_GetAttrString(lizhifm, "program_name");
    PyArg_Parse(p_program, "s", &program_char);
    program_name = string(program_char);
    program = program_name;
}

/* Enter num to choose FM
  * classes_num: FM kinds 
  */
void MPCPlayer::ChooseFM(const int &classes_num,  string& classes_id_string) {
  cout << "Please enter the FM channel id(\"h\" to exit): ";
  while (cin >> classes_id_string) {
    if (classes_id_string == "h") {
      system("mpc stop");
      cout << "Player exits" << endl;
      exit(2);
    }
    // if classes_id is digit and out of index
    int classes_id = 0;
    if (DigitOrNot(classes_id_string)) {
      // conver string to int
      ConvertFromString(classes_id, classes_id_string);
      if (classes_id > classes_num) {
        cout << "id is out of FM channel id, please try again: " << endl;
      }else {
        break;        
        }
      }else {
          cout << "input is not digit, please try again: " << endl;
    } 
  }
  // clear cin
  cin.clear();
  cin.ignore();
}

/* Enter num to choose program 
  * program_num: program kinds 
  */
void MPCPlayer::ChooseProgram(const int &program_num,  string&name_id_string) { 
  cout << "Please enter the program name id, \"h\" to exit: ";
  while(cin >> name_id_string) {
    if (name_id_string == "h") {
      system("mpc stop");
      cout << "Player exits" << endl;
      exit(2);
    }
    // if classes_id is digit and out of index
    int name_id = 0;
    if (DigitOrNot(name_id_string)) {
      // conver string to int
      ConvertFromString(name_id, name_id_string);
      if (name_id > program_num) {
        cout << "id is out of program name id, please try again:" << endl;
      }else {
        break;        
        }
      }else {
          cout << "input is not digit, please try again: " << endl;
    } 
  }
  // clear cin
  cin.clear();
  cin.ignore();
}

/* Get the num of classes from python
 * lizhifm: lizhifm object
 * classes_num: save the num of classes
 */
void MPCPlayer::get_classes_set_num(PyObject *lizhifm, int&classes_num) {
  PyObject *p_classes_set = PyObject_GetAttrString(lizhifm, "classes_set_num");
  PyArg_Parse(p_classes_set, "i", &classes_num);
}

/* Get the num of program from python
 * lizhifm: lizhifm object
 * program_num: save the num of program
 */
void MPCPlayer::get_program_num(PyObject *lizhifm, int &program_num) {
  PyObject *p_program_num = PyObject_GetAttrString(lizhifm, "program_num");
  PyArg_Parse(p_program_num, "i", &program_num);
}

/* Choose the fm and program you want 
  * lizhifm: lizhifm object
  * program_url: save the program url
  */
void MPCPlayer::ChooseProgramURL(PyObject *lizhifm, string &program_url) {
  // get the classes_set number from PyObject
  int classes_num = 0;
  get_classes_set_num(lizhifm, classes_num);
  // input the fm id 
  string classes_id_string = "";
  ChooseFM(classes_num, classes_id_string);
  // request for FM list
  // set params
  PyObject *findfm_arg = PyTuple_New(1);
  PyObject *findfm_arg1 = Py_BuildValue("s", classes_id_string.c_str());
  PyTuple_SetItem(findfm_arg, 0, findfm_arg1);
  // get object func
  PyObject *findfm = PyObject_GetAttrString(lizhifm, "FindClasses");
  PyObject *findfm_return_val = PyObject_CallObject(findfm, findfm_arg);
  // get return val 
//   char *classes_char = "";
//   PyArg_Parse(findfm_return_val, "s", &classes_char);   // "s": string(Py) to char * 
//   string classes(classes_char);  
  // get classes name and save in MPCPlayer class
  string classes = "";
  save_classes(lizhifm, classes);
  // request for program url
  int program_num = 0;
  get_program_num(lizhifm, program_num);
  // input the program id 
  string program_num_string = "";
  ChooseProgram(program_num, program_num_string);
  // set params
  PyObject *findprogramurl_arg = PyTuple_New(2);
  PyObject *findprogramurl_arg1 = Py_BuildValue("s", program_num_string.c_str());
  PyObject *findprogramurl_arg2 = Py_BuildValue("s", classes.c_str());
  PyTuple_SetItem(findprogramurl_arg, 0, findprogramurl_arg1);
  PyTuple_SetItem(findprogramurl_arg, 1, findprogramurl_arg2);
  // get object func
  char *program_name_char = "";
  PyObject *findprogramurl = PyObject_GetAttrString(lizhifm, "FindProgramUrl");
  PyObject *findprogramurl_return_val = PyObject_CallObject(findprogramurl, findprogramurl_arg);
  // get program url and save in MPCPlayer class
  char *program_url_name_char = "";
  PyObject *p_program_url_name = PyObject_GetAttrString(lizhifm, "program_url");
  PyArg_Parse(p_program_url_name, "s", &program_url_name_char);
  program_url = string(program_url_name_char);
  // get program name and save in MPCPlayer class
  string program = "";
  save_program(lizhifm, program);
}
 /* Request for fm url
  * lizhifm: lizhifm object 
  * program_url: program's url
  */
void MPCPlayer::RequestURL(PyObject *lizhifm, string &program_url) {
  PyObject *requestlizhifm = PyObject_GetAttrString(lizhifm, "RequestLiZiFM");
  PyObject *requestlizhifm_arg = PyTuple_New(1);
  PyObject *requestlizhifm_arg1 = Py_BuildValue("s", program_url.c_str());
  PyTuple_SetItem(requestlizhifm_arg, 0, requestlizhifm_arg1);
  PyObject_CallObject(requestlizhifm, requestlizhifm_arg);
}

/* Get the total num of songs
  * lizhifm: lizhifm object
  * program_url: program's url
  */
void MPCPlayer::GetNumOfSong(PyObject *lizhifm, string &program_url) {
    PyObject *getnumoffm = PyObject_GetAttrString(lizhifm, "GetNumOfAudio");
    PyObject *getnumoffm_arg  = PyTuple_New(1);
    PyObject *getnumoffm_arg1 = Py_BuildValue("s", program_url.c_str());
    PyTuple_SetItem(getnumoffm_arg, 0, getnumoffm_arg1);
    PyObject_CallObject(getnumoffm, getnumoffm_arg);
}

/* Get songs' list
 * lizhifm: lizhifm object
 * program_url: program's url
 * p_songs_url: all songs' url
 * song_num: songs' number
 */
void MPCPlayer::GetListOfSong(PyObject *lizhifm, string &program_url, PyObject *&p_songs_url, int &song_num) {
    PyObject *getlistoffm = PyObject_GetAttrString(lizhifm, "GetListOfAudio");
    PyObject *getlistoffm_arg  = PyTuple_New(1);
    PyObject *getlistoffm_arg1 = Py_BuildValue("s", program_url.c_str());
    PyTuple_SetItem(getlistoffm_arg, 0, getlistoffm_arg1);
    PyObject *getlistoffm_return_val = PyObject_CallObject(getlistoffm, getlistoffm_arg);
    /*get return val, songs' url and songs' number*/
    p_songs_url = PyTuple_GetItem(getlistoffm_return_val, 0);
    PyObject *p_songs_number = PyTuple_GetItem(getlistoffm_return_val, 1);
    PyArg_Parse(p_songs_number, "i", &song_num);
}

/* Get FM lists
 * lizhifm: lizhifm object
 */
void MPCPlayer::GetClassesList(PyObject *lizhifm) {
    PyObject *getfmlist = PyObject_GetAttrString(lizhifm, "GetClassesList");
    PyObject_CallObject(getfmlist, NULL);
}

/* Choose songs id
 * lizhifm: lizhifm object
 * p_songs_url: all songs url
 * songs_number: songs' number in first page
 * return: songs_id_string
 */
string MPCPlayer::ChooseSongsURL(PyObject *lizhifm, PyObject *&p_songs_url, int &songs_number) {
  cout << "Please input the name id of FM(\"c\" to upper direct, \"h\" to exit): ";
  string songs_id_string = "";
  while (cin >> songs_id_string) {
    if (songs_id_string == "h") {
      system("mpc stop");
      cout << "Player exits" << endl;
      exit(2);
    }
    if (songs_id_string == "c") {
      /*get list of classes*/
      GetClassesList(lizhifm);
      /*choose the program url */
      string program_url = "";
      ChooseProgramURL(lizhifm, program_url);
      /*request for program url*/
      RequestURL(lizhifm, program_url);
      /*get the total num of  songs*/
      GetNumOfSong(lizhifm, program_url);
      /*get the list of songs*/
      GetListOfSong(lizhifm, program_url, p_songs_url, songs_number);
      /*Choose Songs again*/
      return ChooseSongsURL(lizhifm, p_songs_url, songs_number);
    }
    // if classes_id is digit and out of index
    int song_id = 0;
    if (DigitOrNot(songs_id_string)) {
      // conver string to int
      ConvertFromString(song_id, songs_id_string);
      if (song_id > songs_number) {
        cout << "id is out of songs id, please try again:" << endl;
      }else {
        break;        
        }
      }else {
          cout << "input is not digit, please try again: " << endl;
    } 
  }
  // clear cin
  cin.clear();
  cin.ignore();
  return songs_id_string;
}

/* Get Songs Url
 * lizhifm: lizhifm object
 * p_songs_url: all songs url
 * songs_number: songs number in first page
 * url: <song url, program url>
 */
void MPCPlayer::GetSongsURL(PyObject *lizhifm, PyObject *&p_songs_url, int &songs_number, pair<string, string> &url) {
  /*choose song id*/
  string songs_id_string = "";
  songs_id_string = ChooseSongsURL(lizhifm, p_songs_url, songs_number);
  /*run the func of python*/
  PyObject *getfmurl = PyObject_GetAttrString(lizhifm, "GetAudioUrl");
  PyObject *getfmurl_arg = PyTuple_New(2);
  PyObject *getfmurl_arg1 = Py_BuildValue("s", songs_id_string.c_str());
  PyTuple_SetItem(getfmurl_arg, 0, p_songs_url);
  PyTuple_SetItem(getfmurl_arg, 1, getfmurl_arg1);
  PyObject *getsongsurl_return_val = PyObject_CallObject(getfmurl, getfmurl_arg);
  /*get return val*/
  PyObject *getfmurl_return_val1 = PyTuple_GetItem(getsongsurl_return_val, 0);
  PyObject *getfmurl_return_val2 = PyTuple_GetItem(getsongsurl_return_val, 1);
  char *url_first_char = "";
  char *url_second_char = "";
  PyArg_Parse(getfmurl_return_val1, "s", &url_first_char);
  PyArg_Parse(getfmurl_return_val2, "s", &url_second_char);
  url.first = string(url_first_char);            // Song url
  song_url = string(url_first_char);  
  url.second = string(url_second_char); // Program url
  /*get song's name*/
  char *song_name_char = "";
  PyObject *song_name_dict = PyObject_GetAttrString(lizhifm, "id_audio");
  PyObject *p_song_name = PyDict_GetItem(song_name_dict, getfmurl_arg1);
  PyArg_Parse(p_song_name, "s", &song_name_char);
  song = string(song_name_char);
}

pair<string, string> MPCPlayer::PlayInit(PyObject *lizhifm) {
  // define pair<string, stirng> to save fm_url and program fm_url
  pair<string, string> url;
  /*choose the program url */
  string program_url = "";
  ChooseProgramURL(lizhifm, program_url);
  /*request for program url*/
  RequestURL(lizhifm, program_url);
  /*get the total num of  songs*/
  GetNumOfSong(lizhifm, program_url);
  /*get the list of songs*/
  PyObject *p_songs_url = NULL;
  int song_num = 0;
  GetListOfSong(lizhifm, program_url, p_songs_url, song_num);
  /*get the url of song and save the song's name*/
  GetSongsURL(lizhifm, p_songs_url, song_num, url);
  return url;
}

#endif
