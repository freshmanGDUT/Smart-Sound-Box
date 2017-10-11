#-*- coding: UTF-8 -*-
import LiZhiFMCppAPI, sys, os, commands
reload(sys)
sys.setdefaultencoding("utf-8")
# print sys.getdefaultencoding()

class MpcPlayer(object):
    """
    添加歌曲到播放列表
    """
    def AddFMToPlayLIst(self, url):
        fm_add = "mpc add " + url
        os.system(fm_add)
        print "已将音频添加到播放列表"
    """
    获取播放列表
    """
    def PlayList(self):
        playlist_str = commands.getoutput("mpc playlist")
        self.playlist = playlist_str.split("\n")
    """
    播放音乐
    """
    def FMPlay(self, fm_num=0):
        self.PlayList()
        num = len(self.playlist)
        if fm_num != 0:
            num = fm_num
        fm_play = "mpc play " + str(num)
        os.system(fm_play)
        print "开始播放"
    """
    展示播放列表
    """
    def ShowPlayList(self):
        self.PlayList()
        for item in self.playlist:
            print str(item).decode("string_escape")
    """
    显示播放列表中歌曲数量
    """
    def NumOfPlayList(self):
        self.PlayList()
        return len(self.playlist)
    """
    暂停播放
    """
    def StopPlay(self):
        os.system("mpc pause")
    """
    调高音量
    """
    def SetVolume(self, volume):
        os.system("amixer set Headphone " + volume)
    """
    快进
    """
    def PlaySpeed(self):
        os.system("mpc seek +")
    """
    后退
    """
    def PlayBack(self):
        os.system("mpc seek -")
    """
    退出mpc
    """
    def MpcStop(self):
        os.system("mpc stop")
    """
    输入指令
    """
    def MpcCommand(self):
        command = raw_input("Input your command: ")
        os.system(command)

def PlayerInit(lizhifm):
    # 请求获取音频链接过程
    url = lizhifm.FindUrl()
    lizhifm.RequestLiZiFM(url)
    lizhifm.GetNumOfFM(url)
    fm_url, number = lizhifm.GetListOfFM(url)
    fm_url, url = lizhifm.GetFMUrl(fm_url, number)
    return url, fm_url

# def main():
#     # 初始化荔枝FM对象及播放器操作对象
#     lizhifm = LiZhiFMCppAPI.LiZhiFM()
#     mpc_player = MpcPlayer()
#     # 进入播放器主程序
#     # 初始化播放器
#     url, fm_url = PlayerInit(lizhifm)
#     # 使用播放器进行播放
#     mpc_player.AddFMToPlayLIst(fm_url)
#     mpc_player.FMPlay()
#     while True:
#         # 后续指令
#         print "Enter a(pause), b(play), c(uper direct), d(show playlist), e(set volume), f(speed)， g(back), h(exit)"
#         command = raw_input("Please enter your command: ")
#         if command == "a":
#             mpc_player.StopPlay()
#         if command == "b":
#             num = raw_input("Choose the num of the fm(enter means default): ")
#             if num != "":
#                 num = int(num)
#                 mpc_player.FMPlay(num)
#             else:
#                 mpc_player.FMPlay()
#         if command == "c":
#             fm_url, number = lizhifm.GetListOfFM(url)
#             fm_url, url = lizhifm.GetFMUrl(fm_url, number)
#             mpc_player.AddFMToPlayLIst(str(fm_url))
#         if command == "d":
#             mpc_player.ShowPlayList()
#         if command == "e":
#             volume = raw_input("Enter the volume you want(127 is 100%): ")
#             mpc_player.SetVolume(volume)
#         if command == "f":
#             mpc_player.PlaySpeed()
#         if command == "g":
#             mpc_player.PlayBack()
#         if command == "h":
#             mpc_player.MpcStop()
#             break
#
# if __name__ == "__main__":
#     main()