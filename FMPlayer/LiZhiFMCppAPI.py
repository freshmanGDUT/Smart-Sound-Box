#-*- coding: UTF-8 -*-
import requests, sys, os, time
from bs4 import BeautifulSoup
from PIL import Image
import cStringIO

reload(sys)
sys.setdefaultencoding("utf-8")
# print sys.getdefaultencoding()

class LiZhiFM(object):
    def __init__(self):
        self.lianai = {}
        self.xinli = {}
        self.zhiyu = {}
        self.yaogun = {}
        self.minyao = {}
        self.liuxing = {}
        self.caijing = {}
        self.jiaoyu = {}
        self.keji = {}
        self.id_classes = {}
        self.id_program = {}
        self.id_audio = {}
        self.name_duration = {}
        self.audio_img = {}
        self.audio_url = {}
        self.classes_set = set()
        self.header = {"User-Agent" : "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.116 Safari/537.36"}
        self.PlayList()
    """
    初始化节目列表
    """
    def PlayList(self):
        self.qinggan = [
            {"classes":"恋爱", "name":"修炼爱情", "url":"http://www.lizhi.fm/18459/"},
            {"classes":"恋爱", "name":"连招恋爱法", "url":"http://www.lizhi.fm/1831751/"},
            {"classes":"恋爱", "name":"遇见长安", "url":"http://www.lizhi.fm/1472063/"},
            {"classes":"心理", "name":"努力做最好的自己", "url":"http://www.lizhi.fm/1007613/"},
            {"classes":"心理", "name":"心理FM", "url":"http://www.lizhi.fm/11387/"},
            {"classes":"心理", "name":"TA人际沟通分析", "url":"http://www.lizhi.fm/1475490/"},
            {"classes":"治愈", "name":"晚间治愈系", "url":"http://www.lizhi.fm/29985/"},
            {"classes":"治愈", "name":"如你一般的全世界", "url":"http://www.lizhi.fm/1564437/"},
            {"classes":"治愈", "name":"城市夜无眠", "url":"http://www.lizhi.fm/928206/"}
        ]
        self.yinyue = [
            {"classes":"摇滚", "name":"摇滚天堂", "url":"http://www.lizhi.fm/11319/"},
            {"classes":"摇滚", "name":"六月黄家驹Beyond", "url":"http://www.lizhi.fm/957456/"},
            {"classes":"摇滚", "name":"听着摇滚去跑步", "url":"http://www.lizhi.fm/380826/"},
            {"classes":"民谣", "name":"赵雷的小屋", "url":"http://www.lizhi.fm/1906746/"},
            {"classes":"民谣", "name":"民谣小众，但不普通", "url":"http://www.lizhi.fm/452791/"},
            {"classes":"民谣", "name":"民谣乌托邦", "url":"http://www.lizhi.fm/543975/"},
            {"classes":"流行", "name":"深情的薛之谦", "url":"http://www.lizhi.fm/1708180/"},
            {"classes":"流行", "name":"好听的英伦歌曲", "url":"http://www.lizhi.fm/845961/"},
            {"classes":"流行", "name":"不听音乐会死星球", "url":"http://www.lizhi.fm/12350/"}
        ]
        self.zixun = [
            {"classes":"财经", "name":"财经郎眼官方播客", "url":"http://www.lizhi.fm/2646628/"},
            {"classes":"财经", "name":"知道点管理学", "url":"http://www.lizhi.fm/413926/"},
            {"classes":"财经", "name":"商业资本论", "url":"http://www.lizhi.fm/206634/"},
            {"classes":"教育", "name":"提高口才生活事例剖析", "url":"http://www.lizhi.fm/535127/"},
            {"classes":"教育", "name":"申论素材和热点", "url":"http://www.lizhi.fm/1093613/"},
            {"classes":"教育", "name":"明哥聊求职", "url":"http://www.lizhi.fm/875605/"},
            {"classes":"科技", "name":"极客电台", "url":"http://www.lizhi.fm/283091/"},
            {"classes":"科技", "name":"魅族科技", "url":"http://www.lizhi.fm/3520724/"},
            {"classes":"科技", "name":"UX Coffee 设计咖", "url":"http://www.lizhi.fm/1752909/"}
        ]
        for qinggan_id in self.qinggan:
            self.classes_set.add(qinggan_id["classes"])
            if qinggan_id["classes"] == "恋爱":
                dict_tmp = {qinggan_id["name"]:qinggan_id["url"]}
                self.lianai.update(dict_tmp)
            if qinggan_id["classes"] == "心理":
                dict_tmp = {qinggan_id["name"]:qinggan_id["url"]}
                self.xinli.update(dict_tmp)
            if qinggan_id["classes"] == "治愈":
                dict_tmp = {qinggan_id["name"]:qinggan_id["url"]}
                self.zhiyu.update(dict_tmp)
        for yinyue_id in self.yinyue:
            self.classes_set.add(yinyue_id["classes"])
            if yinyue_id["classes"] == "摇滚":
                dict_tmp = {yinyue_id["name"]:yinyue_id["url"]}
                self.yaogun.update(dict_tmp)
            if yinyue_id["classes"] == "民谣":
                dict_tmp = {yinyue_id["name"]:yinyue_id["url"]}
                self.minyao.update(dict_tmp)
            if yinyue_id["classes"] == "流行":
                dict_tmp = {yinyue_id["name"]:yinyue_id["url"]}
                self.liuxing.update(dict_tmp)
        for zixun_id in self.zixun:
            self.classes_set.add(zixun_id["classes"])
            if zixun_id["classes"] == "财经":
                dict_tmp = {zixun_id["name"]: zixun_id["url"]}
                self.caijing.update(dict_tmp)
            if zixun_id["classes"] == "教育":
                dict_tmp = {zixun_id["name"]: zixun_id["url"]}
                self.jiaoyu.update(dict_tmp)
            if zixun_id["classes"] == "科技":
                dict_tmp = {zixun_id["name"]: zixun_id["url"]}
                self.keji.update(dict_tmp)
        print "Welcom to FindLab FM"
        print "You can choose one of the FM channel below: "
        id = 1
        self.classes_set_num = len(self.classes_set)
        for classes_name in self.classes_set:
            self.id_classes[str(id)] = str(classes_name).decode("string_escape")
            if id < 10:
                print "%d  %s" % (id, str(classes_name).decode("string_escape"))
            else:
                print "%d %s" % (id, str(classes_name).decode("string_escape"))
            id += 1
    """
    获取电台列表
    """
    def GetClassesList(self):
        print "You can choose one of the FM channel below: "
        id = 1
        for classes_name in self.classes_set:
            self.id_classes[str(id)] = str(classes_name).decode("string_escape")
            if id < 10:
                print "%d  %s" % (id, str(classes_name).decode("string_escape"))
            else:
                print "%d %s" % (id, str(classes_name).decode("string_escape"))
            id += 1
    """
    查询url(2部分)
    """
    """
    获取
    """
    def FindClasses(self, classes_id):
        classes = self.id_classes[classes_id]
        # # 判断输入classes是否合法
        # if classes not in self.classes_set:
        #     print "classes not in FM, Please choose other FM"
        #     return
        number = 0
        if classes == "恋爱":
            id = 1
            for program in self.lianai.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "心理":
            id = 1
            for program in self.xinli.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "治愈":
            id = 1
            for program in self.zhiyu.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "摇滚":
            id = 1
            for program in self.yaogun.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "民谣":
            id = 1
            for program in self.minyao.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "流行":
            id = 1
            for program in self.liuxing.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "财经":
            id = 1
            for program in self.caijing.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "教育":
            id = 1
            for program in self.jiaoyu.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        if classes == "科技":
            id = 1
            for program in self.keji.keys():
                self.id_program[str(id)] = str(program).decode("string_escape")
                if id < 10:
                    print "%d  %s" % (id, str(program).decode("string_escape"))
                else:
                    print "%d %s" % (id, str(program).decode("string_escape"))
                id += 1
                number += 1
        self.program_num = number
        self.classes = classes
        return classes

    def FindProgramUrl(self, name_id, classes):
        name = self.id_program[name_id]
        print "正在进入%s电台，点播节目《%s》" % (classes, name)
        if classes == "恋爱":
            if name not in self.lianai.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.lianai[name]
        if classes == "心理":
            if name not in self.xinli.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.xinli[name]
        if classes == "治愈":
            if name not in self.zhiyu.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.zhiyu[name]
        if classes == "摇滚":
            if name not in self.yaogun.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.yaogun[name]
        if classes == "民谣":
            if name not in self.minyao.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.minyao[name]
        if classes == "流行":
            if name not in self.liuxing.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.liuxing[name]
        if classes == "财经":
            if name not in self.caijing.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.caijing[name]
        if classes == "教育":
            if name not in self.jiaoyu.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.jiaoyu[name]
        if classes == "科技":
            if name not in self.keji.keys():
                print "name not in channel, please choose other name"
                return
            self.program_url = self.keji[name]
        self.program_name = name
        return self.program_url
    """
    请求访问,并获取页面信息, 申请访问前需先通过FindUrl获取url
    """
    def RequestLiZiFM(self, url):
        # 判断url合法性
        if url == "":
            print "Please enter right url"
            return
        lizhifm_connect = requests.get(url, headers=self.header)
        self.lizhifm_content = BeautifulSoup(lizhifm_connect.content.decode("utf-8", "lxml"), "html.parser")
    """
    获取节目数量
    """
    def GetNumOfAudio(self, url):
        FMList = self.lizhifm_content.find("div", {"class": "box-title box-title-with-radio-controller"}).text
        FMList_num = int(FMList.split("（")[1].split("）")[0])
        FMPage_num = FMList_num // 20 + 1
        print "当前FM共有%d段音频" % FMList_num
    """
    获取节目列表,节目时长和封面(只列出荔枝FM第一页所有的音频)
    """
    def GetListOfAudio(self, url):
        id = 1
        number = 0
        fm_url = {}
        url_list = []
        audio_list = []
        audio_duration = []
        audio_img = []
        self.RequestLiZiFM(url + "p/" + "1" + ".html")
        FMList = self.lizhifm_content.find("ul", {"class": "audioList fontYaHei js-audio-list"})
        audio_list_item = FMList.find_all("a", {"class": "clearfix js-play-data audio-list-item"})
        for audio_url in audio_list_item:
            url_list.append(audio_url.attrs["data-url"])
            audio_duration.append(audio_url.attrs["data-duration"])
            audio_img.append(audio_url.attrs["data-cover"])
        print "节目列表"
        for FMaudio_list in audio_list_item:
            audio_name = FMaudio_list.find("p", {"class": "audioName"})
            self.id_audio[str(id)] = str(audio_name.text.strip()).decode("string_escape")
            if id < 10:
                print "%d  %s" % (id, str(audio_name.text.strip()).decode("string_escape"))
            else:
                print "%d %s" % (id, str(audio_name.text.strip()).decode("string_escape"))
            id += 1
            number += 1
            audio_list.append(audio_name.text.strip())
        # 存储对应电台的url
        for id in range(len(url_list)):
            self.audio_url[str(audio_list[id]).decode("string_escape")] = url_list[id]

        # 存储对应电台的时间
        for id in range(len(audio_duration)):
            self.name_duration[str(audio_list[id]).decode("string_escape")] = audio_duration[id]
        # 存储对应电台的封面
        for id in range(len(audio_img)):
            self.audio_img[str(audio_list[id]).decode("string_escape")] = audio_img[id]
        return self.audio_url, number
    """
    获取歌曲链接
    """
    def GetAudioUrl(self, audio_url, audio_id):
        audio = self.id_audio[audio_id]
        if audio not in audio_url.keys():
            print "Audio doesn't exist"
            return
        # print fm_url[play]
        return audio_url[audio], self.program_url
    # """
    # 保存电台和FM封面
    # """
    def SubSaveCover(self, url):
        self.RequestLiZiFM(url + "p/" + "1" + ".html")
        cover_content = self.lizhifm_content.find("div", {"class": "radioCover left"})
        cover_content = cover_content.find("img")
        dir = str(cover_content.attrs["alt"]).decode("string_escape")
        if not os.path.exists(dir):
            os.mkdir(str(cover_content.attrs["alt"]).decode("string_escape"))
        img_url = cover_content.attrs["src"]
        img_content = requests.get(img_url)
        img = Image.open(cStringIO.StringIO(img_content.content))
        if len(img.split()) == 4:
            r, g, b, a = img.split()
            img = Image.merge("RGB", (r, g, b))
        img = img.resize((200, 200), resample=Image.ANTIALIAS)
        img.save(str(cover_content.attrs["alt"]).decode("string_escape") + "/Subject Cover", "BMP")
        print "Save Subject Cover Successfully"
        # 保存音频封面
        FMList = self.lizhifm_content.find("ul", {"class": "audioList fontYaHei js-audio-list"})
        audio_list_item = FMList.find_all("a", {"class": "clearfix js-play-data audio-list-item"})
        # 读取列表url和名字
        for audio_url in audio_list_item:
            # 跳转到列表url中
            self.RequestLiZiFM(url + audio_url.attrs["data-id"])
            cover_content = self.lizhifm_content.find("div", {"class": "audioCover left"})
            cover_content = cover_content.find("img")
            img_url = cover_content.attrs["src"]
            print img_url
            print "save " + str(cover_content.attrs["alt"]).decode("string_escape")
            img_content = requests.get(img_url)
            img = Image.open(cStringIO.StringIO(img_content.content))
            if len(img.split()) == 4:
                r, g, b, a = img.split()
                img = Image.merge("RGB", (r, g, b))
            img = img.resize((200, 200), resample=Image.ANTIALIAS)
            audio_name = str(cover_content.attrs["alt"]).decode("string_escape").replace("/", "\\")
            img.save(dir + "/" + audio_name, "BMP")
            time.sleep(0.1)
    def SaveCover(self):
        # 保存专题封面
        for dict in self.lianai.keys():
            print "----------save " + dict + "----------"
            url = self.lianai[dict]
            self.SubSaveCover(url)
        for dict in self.xinli.keys():
            print "----------save " + dict + "----------"
            url = self.xinli[dict]
            self.SubSaveCover(url)
        for dict in self.zhiyu.keys():
            print "----------save " + dict + "----------"
            url = self.zhiyu[dict]
            self.SubSaveCover(url)
        for dict in self.yaogun.keys():
            print "----------save " + dict + "----------"
            url = self.yaogun[dict]
            self.SubSaveCover(url)
        for dict in self.minyao.keys():
            print "----------save " + dict + "----------"
            url = self.minyao[dict]
            self.SubSaveCover(url)
        for dict in self.liuxing.keys():
            print "----------save " + dict + "----------"
            url = self.liuxing[dict]
            self.SubSaveCover(url)
        for dict in self.caijing.keys():
            print "----------save " + dict + "----------"
            url = self.caijing[dict]
            self.SubSaveCover(url)
        for dict in self.jiaoyu.keys():
            print "----------save " + dict + "----------"
            url = self.jiaoyu[dict]
            self.SubSaveCover(url)
        for dict in self.keji.keys():
            print "----------save " + dict + "----------"
            url = self.keji[dict]
            self.SubSaveCover(url)
def TimeStandrize(time):
    min = time // 60
    sec = time % 60
    h = 0
    if min > 59:
        h = min // 60
        min = min % 60
    return {"h": h, "min": min, "sec": sec}
if __name__ == "__main__":
    lizhifm = LiZhiFM()
    lizhifm.SaveCover()
    # url = lizhifm.FindUrl()
    # lizhifm.RequestLiZiFM(url)
    # lizhifm.GetNumOfFM(url)
    # lizhifm.GetListOfFM(url)
    # lizhifm.GetFMUrl()
