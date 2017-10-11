#ifndef CPPMPCPLAY_H_INCLUDE_
#define CPPMPCPLAY_H_INCLUDE_

#include <utility>
#include <cctype>
#include <vector>
#include <sstream>
#include <stdio.h>
#include "Python.h"

using namespace std;

/* Convert T value to String
  * value: data to convert
  * string: the return value
  */
template <typename T> void ConvertFromString(T &value, const string &s) {
    stringstream ss(s);
    ss >> value;
}

/* Judge whether input is a digit
  * input: value to judge
  */
bool DigitOrNot(string &input) {
    for (string::iterator i = input.begin(); i != input.end(); ++i) {
        if (!isdigit(*i)) {
            return false;
        }
    }
    return true;
}

class MPCPlayer {
public:
    MPCPlayer(){
      playlist_img_path.reserve(1000);
      playlist.reserve(1000);
      songs_num = 0;
    }
    ~MPCPlayer() {
        system("mpc clear");
    }
    pair<string, string> PlayInit(PyObject *lizhifm);
    /* Get the num of classes from python
     * lizhifm: lizhifm object
     * classes_num: save the num of classes
     */
    void get_classes_set_num(PyObject *lizhifm, int&classes_num);
    /* Get the num of program from python
     * lizhifm: lizhifm object
     * program_num: save the num of program
     */
    void get_program_num(PyObject *lizhifm, int&program_num);
    /* Save classes got from python
     *  lizhifm: lizhifm object
     *  classes_name: classes name
     */
    void save_classes(PyObject* lizhifm, string& classes_name);
    /* Save program got from python
     * lizhifm: lizhifm object
     * program_name: program name
     */
    void save_program(PyObject *lizhifm, string &program_name);
    /* Save program url got from python
     * lizhifm: lizhifm object
     * program_url_name: program's url
     */
    void save_program_url(PyObject *lizhifm, string &program_url_name);
    /* Choose the fm and program you want 
     * lizhifm: lizhifm object
     * program_url: save the program url
     */
    void ChooseProgramURL(PyObject *lizhifm, string &program_url);
    /* Enter num to choose FM
     * classes_num: FM kinds 
     */
    void ChooseFM(const int &classes_num,  string&classes_id_string);
    /* Enter num to choose program 
     * program_num: program kinds 
     */
    void ChooseProgram(const int &program_num, string&name_id_string);
    /* Request for fm url
     * lizhifm: lizhifm object 
     * program_url: program's url
     */
    void RequestURL(PyObject *lizhifm, string &program_url); 
    /* Get the total num of fm
     * lizhifm: lizhifm object
     * program_url: program's url
     */
    void GetNumOfSong(PyObject *lizhifm, string &program_url);
    /* Get the list of FM
     * lizhifm: lizhifm object
     * program_url: program's url
     * p_songs_url: save songs' url PyObject*
     * song_num: save songs' number
     */
    void GetListOfSong(PyObject *lizhifm, string &program_url, PyObject *&p_songs_url, int &song_num);
    /* Get FM lists
     * lizhifm: lizhifm object
     */
    void GetClassesList(PyObject *lizhifm);
    /* Choose songs id
     * lizhifm: lizhifm object
     * p_songs_url: all songs url
     * songs_number: songs' number in first page
     * return: songs_id_string
     */
    string ChooseSongsURL(PyObject *lizhifm, PyObject *&p_songs_url, int &songs_number);
    /* Get Songs Url
     * lizhifm: lizhifm object
     * p_songs_url: all songs url
     * songs_number: songs number in first page
     * url: <song url, program url>
     */
    void GetSongsURL(PyObject *lizhifm, PyObject *&p_songs_url, int &songs_number, pair<string, string> &url);
    /* Save the path of songs' img
     */
    void SaveSongImgPath() {
        string path = "./Cover/" + classes + "/" + program + "/" + song;
        playlist_img_path.push_back(path);
    }
    /* Add Song to playlist
     * song_url: song's url
     */
    void AddSongToPlayList(string &song_url) {
        string song_add = "mpc add " + song_url;
        system(song_add.c_str());
        // Save song's name
        playlist.push_back(song);
        // Save path of song's img
        SaveSongImgPath();
        /*add up the songs' number*/
        ++songs_num;
    }
    /* Play songs
     */
    void SongPlay(int song_index=0) {
      /*Judge song index*/
      if (song_index > songs_num) {
        cout << "ID out of index" << endl;   
        return ;
      }
      /*play song_index song*/
      int num = songs_num;
      if (song_index != 0) {
        num = song_index;
      } 
      stringstream ss;
      string song_num = "";
      ss << num;
      ss >> song_num;
      string song_play = "mpc play " + song_num;
      system(song_play.c_str());
      /*save the song's img playing */
      song_playing_img_path = playlist_img_path[num - 1];
      cout << "开始播放" << endl;
    }
    /* Stop play
     */
    void StopPlay() {
        system("mpc stop");
    }
    /* Show Play list
     */
    void ShowPlayList() {
        cout << "Play List: " << endl; 
        for (int i = 0; i != playlist.size(); ++i) {
            cout << i+1 << " " << playlist[i] << endl;
        }
    }
    /* Set Volume 
     */
    void SetVolume(string &volume) {
        string setvolume = "amixer set Headphone " + volume;
        system(setvolume.c_str());
    }
    /* Speed up
     */
    void PlaySpeed() {
        system("mpc seek +");
    }
    /* Slow down
     */
    void PlayBack() {
        system("mpc seek  -");
    }
    /* Exit Mpc
     */
    void MpcStop() {
        system("mpc stop");
    }
    /* Get playlist img path
     */
    vector<string> get_playlist_img_path() {
        return playlist_img_path;
    }
    /* Get song's img path playing
     */
    string get_song_img_path() {
      return song_playing_img_path;
    }
private:
    string classes;
    string program;
    string song;
    vector<string> playlist_img_path;
    vector<string> playlist;
    string song_playing_img_path;
    string song_url;
    int songs_num;
};

#endif
